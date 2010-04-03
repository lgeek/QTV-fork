#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "downloader.h"
#include "icsparser.h"
#include "about.h"
#include "settings.h"
#include "queue.h"
#include "reminder.h"
#include "showstoday.h"
#include "msgbox.h"
#include "imageloader.h"
#include "infoloader.h"
#include "detailsview.h"

#include <QUrl>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTimer>
#include <QMenu>
#include <QDesktopServices>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QDesktopWidget>
#include <QToolTip>
#include <QCursor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    /* set up the graphical user interface */
    ui->setupUi(this);

    /* set applicatin settings */
    qApp->setApplicationName(APPLICATION_NAME);
    qApp->setApplicationVersion(APPLICATION_VERSION);
    qApp->setOrganizationDomain(ORGANIZATION_DOMAIN);
    qApp->setOrganizationName(ORGANIZATION_NAME);

    /* create instances of the needed classes */
    about = new About(this);
    about->setVisible(0);
    settings = new Settings(this);
    settings->setVisible(0);
    queue = new Queue(this);
    queue->setVisible(0);
    msg = new MsgBox;
    detailsView = new DetailsView(this);
    detailsView->setVisible(0);
    ui->labelCaption->setText("");
    updateLock = false;

    // CENTER the mainWindow!
    qint16 width = QApplication::desktop()->screenGeometry(0).width();
    qint16 height = QApplication::desktop()->screenGeometry(0).height();
    // set the window according to it (right in the middle of the screen)
    this->move((width/2) - (this->width()/2), (height/2) - (this->height()/2));
    // delete logFile
    QFile::remove(LOGFILE);

    /* load the settings */
    settings->loadSettings(settingsData);

    /* setup showViewer */
    QRadialGradient highlightGradient(0.5,0.5,0.9,0,0);
    highlightGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    highlightGradient.setColorAt(0,QColor(Qt::white));
    highlightGradient.setColorAt(1,QColor(0xFF,100,100));
    ui->showViewer->setHighlightBrush(highlightGradient);

    QRadialGradient oldGradient(0.5,0.5,0.9,0,0);
    oldGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    oldGradient.setColorAt(0,QColor(Qt::white));
    oldGradient.setColorAt(1,QColor(0,200,0xFF));
    ui->showViewer->setOldBrush(oldGradient);

    ui->showViewer->setDownloadIcon(QIcon(":/images/save"));
    ui->showViewer->setDownloadAltIcon(QIcon(":/images/saveAlt"));
    ui->showViewer->setInfoIcon(QIcon(":/images/web"));

    /* create model for listView */
    model = new QStandardItemModel(this);
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->listView->setModel(proxyModel);

    /* create the update-timer */
    timer = new QTimer(this);
    timer->start((settingsData.minutes*60)*1000);

    /* create a system-tray-icon */
    systemTray = new QSystemTrayIcon(this);
    systemTray->setIcon(QIcon(QPixmap(":/images/icon")));
    systemTray->setToolTip(tr("QTV - episode guide & reminder"));
    systemTray->show();
    closeForced = false;
    connect(systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    /* create context menu for system-tray-icon */
    QMenu* trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(ui->actionReload);
    trayIconMenu->addAction(ui->actionShows_today);
    trayIconMenu->addAction(ui->actionQueue);
    trayIconMenu->addAction(ui->actionSettings);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(ui->actionAbout_QTV);
    trayIconMenu->addAction(ui->actionExitForced);
    systemTray->setContextMenu(trayIconMenu);

    /* create a parser- & downloader-instance */
    downloader = new Downloader;
    downloader->setProxyUsed(settingsData.useProxy);
    downloader->setProxyData(settingsData.proxyHost, settingsData.proxyPort, settingsData.proxyUsername, settingsData.proxyPassword);
    parser = new ICSParser(QCoreApplication::applicationDirPath() + "/" + ICSFILE);
    queue->setParser(parser);
    reminder = new Reminder;
    showsToday = new ShowsToday(this);
    
    /* connect the parser- & downloader-signal and slots */
    connect(downloader, SIGNAL(finished()), parser, SLOT(parse()));
    connect(downloader, SIGNAL(error(QNetworkReply::NetworkError, QString)), this, SLOT(networkError(QNetworkReply::NetworkError, QString)));
    connect(parser, SIGNAL(finished()), this, SLOT(handleNewData()));

    /* create a imageLoader instance */
    imageLoader = new ImageLoader();
    imageLoader->setParser(parser);
    connect(imageLoader, SIGNAL(setStatusBar(QString)), ui->statusBar, SLOT(showMessage(QString)));
    /* search for new images if a new day begins */
    connect(reminder, SIGNAL(newDay()), imageLoader, SLOT(loadAllShows()));  // reload every image over night
    connect(reminder, SIGNAL(newDay()), imageLoader, SLOT(start()));

    /* create a infoLoader instance */
    infoLoader = new InfoLoader();
    infoLoader->setParser(parser);
    connect(infoLoader, SIGNAL(setStatusBar(QString)), ui->statusBar, SLOT(showMessage(QString)));
    //connect(infoLoader, SIGNAL(finished()), this, SLOT(update()));
    connect(reminder, SIGNAL(newDay()), infoLoader, SLOT(start()));
    connect(reminder, SIGNAL(updateStatusBar(QString)), ui->statusBar, SLOT(showMessage(QString)));

    /* connect all actions */
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionExitForced, SIGNAL(triggered()), this, SLOT(closeForce()));
    connect(ui->actionReload, SIGNAL(triggered()), this, SLOT(update()));
    connect(ui->actionAbout_QT, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionAbout_QTV, SIGNAL(triggered()), about, SLOT(show()));
    connect(ui->actionSettings, SIGNAL(triggered()), settings, SLOT(show()));
    connect(ui->actionQueue, SIGNAL(triggered()), this, SLOT(showQueue()));
    connect(ui->actionShows_today, SIGNAL(triggered()), showsToday, SLOT(show()));


    connect(settings, SIGNAL(settingsChanged(SettingsData)), this, SLOT(settingsChanged(SettingsData)));
    connect(ui->showViewer, SIGNAL(episodeClicked(ICSData)), this, SLOT(episodeClicked(ICSData)));
    connect(ui->showViewer, SIGNAL(downloadIconClicked(ICSData)), this, SLOT(openDownload(ICSData)));
    connect(ui->showViewer, SIGNAL(downloadAltIconClicked(ICSData)), this, SLOT(openDownloadAlt(ICSData)));
    connect(ui->showViewer, SIGNAL(infoIconClicked(ICSData)), this, SLOT(infoIconClicked(ICSData)));

    connect(ui->listView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(itemChosen(const QModelIndex&)));
    connect(ui->listView, SIGNAL(activated(const QModelIndex&)), this, SLOT(itemChosen(const QModelIndex&)));
    connect(ui->checkBox, SIGNAL(toggled(bool)), ui->spinBox, SLOT(setEnabled(bool)));
    connect(ui->checkBox, SIGNAL(toggled(bool)), ui->labelDays, SLOT(setEnabled(bool)));
    connect(ui->checkBox, SIGNAL(clicked()), this, SLOT(updateRecordSettings()));
    connect(ui->spinBox, SIGNAL(editingFinished()), this, SLOT(updateRecordSettings()));

    connect(ui->lineEdit, SIGNAL(textChanged(QString)), proxyModel, SLOT(setFilterRegExp(QString)));
    connect(ui->buttonClear, SIGNAL(clicked()), ui->lineEdit, SLOT(clear()));

    /* check for data directory and create it if necessary */
    QDir dir;
    if (!dir.exists(QCoreApplication::applicationDirPath() + "/data"))
    {
        dir.mkdir(QCoreApplication::applicationDirPath() + "/data");
    }

    /* timer-stuff */
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    QTimer::singleShot(10, this, SLOT(update()));

    /* first time parse; used to show some useful (old) data until the first update is done */
    // QTimer::singleShot(100, parser, SLOT(parse()));

    /* load all episode-settings */
    QFile file(QCoreApplication::applicationDirPath() + "/" + DATAFILE);
    if (file.open(QIODevice::ReadOnly))
    {
        qDebug() << "@MainWindow: reading datafile";
        QDataStream in(&file);    // read tdownloaderhe data serialized from the file

        for (;!file.atEnd();)
        {
            /* read data out of file and create a new element in the list */
            RecordData tmp;
            in >> tmp.show;
            in >> tmp.days;
            if (!tmp.show.isEmpty())         // just append it, if the name is NOT empty
                recordDataList.append(tmp);
        }
    }
    else
        qDebug() << "@MainWindow: couldn't open datafile";
    file.close();

    queue->setQueue(&recordDataList);

    /* check for a program-update */
    if (settingsData.informStable)
    {
        QString currentStable;
        downloader->getCurrentStableVersion(currentStable);
        currentStable.chop(1);

        if (currentStable.toFloat() > qApp->applicationVersion().toFloat())
        {
            if (settingsData.ballonMessageReminder)
                systemTray->showMessage(tr("New Version"),tr("The new stable version ")+currentStable+tr(" of QTV is available!"));
            if (settingsData.messageBoxReminder)
                msg->showMessage(tr("New Version"),tr("The new stable version ")+currentStable+tr(" of QTV is available!"));

            connect(systemTray, SIGNAL(messageClicked()), this, SLOT(openWebsite()));
            connect(msg, SIGNAL(messageClicked()), this, SLOT(openWebsite()));
        }
    }
    if (settingsData.informUnstable)
    {
        QString currentUnstable;
        downloader->getCurrentUnstableVersion(currentUnstable);
        currentUnstable.chop(1);

        if (currentUnstable.toFloat() > qApp->applicationVersion().toFloat())
        {
            if (settingsData.ballonMessageReminder)
                systemTray->showMessage(tr("New Version"),tr("The new unstable version ")+currentUnstable+tr(" of QTV is available!"));
            if (settingsData.messageBoxReminder)
                msg->showMessage(tr("New Version"),tr("The new unstable version ")+currentUnstable+tr(" of QTV is available!"));

            connect(systemTray, SIGNAL(messageClicked()), this, SLOT(openWebsite()));
            connect(msg, SIGNAL(messageClicked()), this, SLOT(openWebsite()));
        }
    }\
}

MainWindow::~MainWindow()
{
    delete about;
    delete settings;
    delete queue;
    delete systemTray;
    delete model;
    delete proxyModel;
    delete timer;
    delete parser;
    delete reminder;
    delete showsToday;

    delete ui;
}

/*****************************************************************************
                                    SLOTS
******************************************************************************/
void MainWindow::openWebsite(void)
{
    QDesktopServices::openUrl(QUrl(qApp->organizationDomain()));
    disconnect(this, SLOT(openWebsite()));
}
void MainWindow::openDownload(ICSData recordData)
{
    QString url = SEARCH_URL;
    QString search;

    search += recordData.show + "+S" +
              QString("%1").arg(recordData.season,2,10,QChar('0')) + "E" +
              QString("%1").arg(recordData.episode,2,10,QChar('0'));
    search = search.replace(QChar(' '), QChar('+'));
    url+=search;

    downloader->download(QUrl("http://serienjunkies.org"), search);   // first access to avoid welcome-message
    QDesktopServices::openUrl(QUrl(url));
}
void MainWindow::openDownloadAlt(ICSData recordData)
{
    QString url = SEARCH_ALT_URL;
    QString search;

    search += recordData.show + "+S" +
              QString("%1").arg(recordData.season,2,10,QChar('0')) + "E" +
              QString("%1").arg(recordData.episode,2,10,QChar('0'));
    search = search.replace(QChar(' '), QChar('+'));
    url+=search;

    QDesktopServices::openUrl(QUrl(url));
}
void MainWindow::infoIconClicked(ICSData recordData)
{
    QString url = TVCOM_SEARCH_URL;
    QString search;

    search += recordData.show + " " + recordData.name;
    search = search.replace(QChar(' '), QChar('+'));
    search = QString(search.toAscii().toPercentEncoding());
    url+=search;

    QString data;
    downloader->download(url, &data);
    if (data.contains("summary.html"))
    {
        int end = data.indexOf("summary.html") + 11;
        int start = data.lastIndexOf("\"", end) + 1;
        url = data.mid(start, end-start+1);

        qDebug() << "@MainWindow: detailsUrl: " + url;
        QDesktopServices::openUrl(QUrl(url));
    }    
}
void MainWindow::episodeClicked(ICSData recordData)
{
    QString url = TVCOM_SEARCH_URL;
    QString search;
    static bool summaryLocked = false;

    if (summaryLocked)
        return;
    summaryLocked = true;
    
    // search on tv.com for the episode
    ui->statusBar->showMessage(tr("Fetching details"));
    search = recordData.show + " " + recordData.name;
    search = search.replace(QChar(' '), QChar('+'));
    search = search.replace("&", "");
    search = QString(search.toAscii().toPercentEncoding());

    url+=search;

    // get the episode-site's url
    QString data;
    downloader->download(url, &data);
    if (data.contains("summary.html"))
    {
        int end = data.indexOf("summary.html") + 11;
        int start = data.lastIndexOf("\"", end) + 1;
        url = data.mid(start, end-start+1);
        qDebug() << "@MainWindow: detailsUrl: " + url;

        // pass the url on to detailsView
        detailsView->parseAndSetData(recordData, url);
        detailsView->show();
    }
    ui->statusBar->showMessage(tr("Ready"));
    summaryLocked = false;
}
void MainWindow::settingsChanged(SettingsData newSettings)
{
    settingsData = newSettings;

    timer->start((settingsData.minutes*60)*1000);
    downloader->setProxyUsed(settingsData.useProxy);
    downloader->setProxyData(settingsData.proxyHost, settingsData.proxyPort, settingsData.proxyUsername, settingsData.proxyPassword);
    //this->update(); // why should be update everytime?
}
void MainWindow::updateRecordSettings(void)
{
    qDebug() << "@MainWindow::updateRecordSettings: updating the settings";
    RecordData tmp;

    /* this slot is only called, if the user has already chosen a show AND if he uses the checkBox or spinBox in the settings-tab */
    tmp.show = proxyModel->data(ui->listView->currentIndex(),Qt::EditRole).toString();
    tmp.days = ui->spinBox->value();

    /* set no highlight brush */
    proxyModel->setData(ui->listView->currentIndex(),QBrush(),Qt::BackgroundColorRole);

    /* remove an old entry in the list */
    for (int pos=0; pos<recordDataList.size(); pos++)
    {
        if (recordDataList.at(pos).show  == tmp.show)
            recordDataList.removeAt(pos);
    }
    /* create a new entry in the list */
    if (ui->checkBox->checkState() == Qt::Checked)
    {
        recordDataList.append(tmp);
        proxyModel->setData(ui->listView->currentIndex(),highlightBrush(),Qt::BackgroundColorRole);
    }

    /* save the whole new list to the settings-file */
    QFile file(QCoreApplication::applicationDirPath() + "/" + DATAFILE);
    file.flush();
    if (file.open(QIODevice::WriteOnly))
    {
        qDebug() << "MainWindow::updateRecordSettings: writing datafile";
        QDataStream out(&file);    // write the data serialized to the file

        for (int pos=0; pos<recordDataList.size(); pos++)
        {
            out << recordDataList.at(pos).show;
            out << recordDataList.at(pos).days;
        }
    }
    else
        qDebug() << "MainWindow::updateRecordSettings: couldn't open datafile";
    file.close();

    /* update the Queue! */
    queue->setQueue(&recordDataList);
}

void MainWindow::networkError(QNetworkReply::NetworkError err, QString errorString)
{
    static qint16 tries = 0;
    if (settingsData.ballonMessageReminder)
        systemTray->showMessage(tr("Network-Error"), tr("Code") +" "+ QString::number(err) + ": " + errorString);
    if (settingsData.messageBoxReminder)
        msg->showMessage(tr("Network-Error"), tr("Code") +" "+ QString::number(err) + ": " + errorString);

    if (tries < FAILURE_RETRIES)
    {
        QTimer::singleShot(RETRY_DELAY, this, SLOT(update()));
        tries++;
    }
}

void MainWindow::handleNewData(void)
{
    ICSData record;
    QStringList shows;
    static bool firstRun = true;

    QModelIndex index;

    // put all different shows in a list
    for (int pos=0; pos<parser->size(); pos++)
    {
        record = parser->at(pos);

        if (shows.indexOf(record.show) == -1)
            shows.append(record.show);
    }
    shows.sort();

    /* load the images */
    if (firstRun && !imageLoader->isRunning())
    {
        imageLoader->setShows(shows);
        imageLoader->start();
        infoLoader->setShows(shows);
        infoLoader->checkInfos();
        // reload info of shows which are in the reminder-list
        for (int pos=0; pos<recordDataList.size(); pos++)
            infoLoader->appendShowToLoad(recordDataList.at(pos).show);
        infoLoader->start();
    }

    /* set up model */
    model->clear();
    model->setColumnCount(1);
    model->setRowCount(shows.count());
    for(int show=0; show<shows.count(); show++)
    {
      index = model->index(show,0);
      model->setData(index,shows.at(show), Qt::EditRole);

      /* highlight remembered shows */
      for(int pos=0; pos<recordDataList.count(); pos++)
      {
        if(recordDataList.at(pos).show == shows.at(show))
        {
          model->setData(index,highlightBrush(),Qt::BackgroundColorRole);
        }
      }
    }

    /* update showsToday */
    showsToday->setShows(parser->getRecords());

    /* EXPERIMENTAL! set next/previous show in the parser to list them in the queue */
    ShowInfo info;
    for (int pos=0; pos<recordDataList.size(); pos++)
    {
        info = infoLoader->getInfo(recordDataList.at(pos).show);

        if (!parser->contains(info.previous) && !info.previous.show.isEmpty())
            parser->append(info.previous);
        if (!parser->contains(info.next) && !info.next.show.isEmpty())
            parser->append(info.next);
    }

    /* set infoLoader showsToLoad (for nightly info-updates) */
    if (!firstRun)
        infoLoader->setShowsToLoad(shows);

    /* enable some actions after the first update which means that data is valid now */
    if (firstRun)
    {
        ui->actionQueue->setEnabled(true);
        ui->actionShows_today->setEnabled(true);
    }

    /* check for new alerts */
    reminder->checkEpisodes(parser->getRecords(), &recordDataList, systemTray, &settingsData);
    firstRun = false;

    ui->statusBar->showMessage(tr("Ready"));
}

void MainWindow::itemChosen(const QModelIndex& index)
{
    ICSData record;
    bool entryFound = false;

    QString item = proxyModel->data(index,Qt::EditRole).toString();

    /* set image */
    ui->labelCaption->setText("<h3>" + item + "</h3>");
    QPixmap pixmap = imageLoader->getImage(item);
    // image
    if (!pixmap.isNull())
    {
        ui->labelImage->show();
        ui->labelImage->setPixmap(pixmap);
    }
    else
        ui->labelImage->hide();

    ShowInfo info = infoLoader->getInfo(item);
    // summary
    if (!info.summary.isEmpty())
    {
        ui->labelInfo->show();
        ui->labelInfo->setText(info.summary);
    }
    else
        ui->labelInfo->hide();
    // genre
    if (!info.genre.isEmpty())
    {
        ui->labelGenre->show();
        ui->labelGenre->setText("<b>Genre: </b>" + info.genre);
    }
    else
        ui->labelGenre->hide();
    // tvstations
    if (!info.tvstation.isEmpty())
    {
        ui->labelTVStation->show();
        ui->labelTVStation->setText("<b>TV-Station: </b>" + info.tvstation);
    }
    else
        ui->labelTVStation->hide();
    // similar shows
    if (!info.similar.isEmpty())
    {
        ui->labelSimilarShows->show();
        ui->labelSimilarShows->setText("<b>Similar Shows: </b>" + info.similar + "<br>");
    }
    else
    {
        ui->labelSimilarShows->hide();
        ui->labelTVStation->setText(ui->labelTVStation->text() + "<br>");
    }

    /* set checkBox to enabled and update the days with an entry in the list (if there is one) */
    if (!ui->checkBox->isEnabled())
        ui->checkBox->setEnabled(true);
    for (int pos=0; pos<recordDataList.size(); pos++)
    {
        if (recordDataList.at(pos).show == item)
        {
            entryFound = true;
            ui->spinBox->setValue(recordDataList.at(pos).days);
        }
    }
    if (entryFound)
    {
        ui->checkBox->setChecked(true);
        ui->spinBox->setEnabled(true);
        ui->labelDays->setEnabled(true);
    }
    else
    {
        ui->checkBox->setChecked(false);
        ui->spinBox->setEnabled(false);
        ui->labelDays->setEnabled(false);
        ui->spinBox->setValue(DEFAULT_DAYS);
    }

    ui->showViewer->clearEpisodes();

    bool setNext=true, setPrevious=true;
    /* work through the parsed shows, search for the given show and put it in a label */
    for (int pos=0; pos<parser->size(); pos++)
    {
        record = parser->at(pos);

        if (record.show == item && record.season != 0)
        {
            ui->showViewer->addEpisode(record);
            if (record.start == info.next.start)
                setNext = false;
            if (record.start == info.previous.start)
                setPrevious = false;
        }
    }
    // next/previous episode
    if (!info.previous.name.isEmpty() && setPrevious)
    {
        ui->showViewer->addEpisode(info.previous);
    }
    if (!info.next.name.isEmpty() && setNext)
    {
        ui->showViewer->addEpisode(info.next);
    }
    ui->showViewer->sortShows();
}

void MainWindow::showQueue(void)
{
    ShowInfo info;

    // get the next/previous shows which aren't in the recordDataList
    for (int pos=0; pos<recordDataList.size(); pos++)
    {
        info = infoLoader->getInfo(recordDataList.at(pos).show);

        if (!parser->contains(info.previous))
            parser->append(info.previous);
        if (!parser->contains(info.next))
            parser->append(info.next);
    }
    queue->show();
}

void MainWindow::update(void)
{
    if (updateLock)
        return;

    updateLock = true;
    ui->statusBar->showMessage(tr("Updating..."));
    downloader->download(QUrl(ICAL_URL),QCoreApplication::applicationDirPath() + "/" + ICSFILE);

    //TODO: perhaps get and add a new feed from the guy who owns "next-episode.net" =)

    updateLock = false;
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)  // a normal left-click hides/shows the main-window
    {
        if (this->isVisible())
            this->hide();
        else
            this->show();
    }
}

void MainWindow::closeForce(void)
{
    closeForced = true;
    this->close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    /* this one's got priority #1 */
    if (closeForced)
        event->accept();
    else if (this->isVisible())
    {
        event->ignore();
        this->hide();
    }
    else
        event->accept();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    /* adjust the size of the infoLabel which is wordwrapped
       this fixes a bug (of QLabel?) which causes the text to be cut off
       http://lists.trolltech.com/qt-interest/2004-02/thread00045-0.html
    */
    ui->labelInfo->adjustSize();
    event->accept();
}

QBrush MainWindow::highlightBrush(void)
{
  QLinearGradient gradient(0,0.5,1,0.5);

  gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
  gradient.setColorAt(0,QColor(0xFF,0xFF,0x80));
  gradient.setColorAt(1,QColor(0xFF,0xFF,0xFF,0xFF));

  return QBrush(gradient);
}
