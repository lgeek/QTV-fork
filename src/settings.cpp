#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>

#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::Settings)
{
    m_ui->setupUi(this);

    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect(m_ui->checkBoxProxy, SIGNAL(toggled(bool)), m_ui->groupBoxProxy, SLOT(setEnabled(bool)));

    connect(m_ui->checkBoxAutostart, SIGNAL(toggled(bool)), this, SLOT(enableAutostart(bool)));
    connect(m_ui->checkBoxAutoDownload, SIGNAL(toggled(bool)), m_ui->groupBoxAutoLoader, SLOT(setEnabled(bool)));
    connect(m_ui->checkBoxGReminder, SIGNAL(toggled(bool)), m_ui->groupBoxGReminder, SLOT(setEnabled(bool)));
}

Settings::~Settings()
{
    delete m_ui;
}

void Settings::enableAutostart(bool enabled)
{
    if (enabled)
    {
        qDebug() << "@Settings: enable autostart";

        #ifdef Q_OS_LINUX
            QDir dir;

            if (!dir.exists(dir.homePath() + "/.config/autostart"))
                dir.mkdir(dir.homePath() + "/.config/autostart");
            if (!QFile::exists(dir.homePath() + "/.config/autostart/qtv.desktop"))
            {
                QFile file(dir.homePath() + "/.config/autostart/qtv.desktop");
                if (file.open(QIODevice::ReadWrite))
                {
                    file.write("[Desktop Entry]\n");
                    file.write(qPrintable("Version=" + QString(APPLICATION_VERSION) + "\n"));
                    file.write("Type=Application\n");
                    file.write(qPrintable("Name=" + QString(APPLICATION_NAME) + "\n"));
                    file.write("Comment=QT TV-Show Reminder\n");
                    file.write(qPrintable("Exec=" + QCoreApplication::applicationFilePath() + "\n"));
                    file.write(qPrintable("Icon=" + QCoreApplication::applicationDirPath() + "/icon.png"));
                    file.close();
                    system(qPrintable("chmod +x " + dir.homePath() + "/.config/autostart/qtv.desktop"));
                }
            }
        #elif defined Q_OS_WIN32
            QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
            settings.setValue("QTV", QCoreApplication::applicationFilePath());
        #else
            #error "QTV can't handle the autostart-settings for your system. Please refer to http://code.google.com/p/qtv/"
        #endif
    }
    else
    {
        qDebug() << "@Settings: disable autostart";

        #ifdef Q_OS_LINUX
            QDir dir;
            QFile::remove(dir.homePath() + "/.config/autostart/qtv.desktop");
        #elif defined Q_OS_WIN32
            QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
            settings.remove("QTV");
        #else
            #error "QTV can't handle the autostart-settings for your system. Please refer to http://code.google.com/p/qtv/"
        #endif
    }
}

void Settings::loadSettings(SettingsData &settings)
{
    bool newSettings = false;

    qDebug() << "@Settings::loadSettings: loading settings";
    /* load settings out of file */
    QFile file(QCoreApplication::applicationDirPath() + "/" + SETTINGSFILE);
    if (file.open(QIODevice::ReadOnly))
    {
        QDataStream in(&file);

        in >> settings.minutes;
        in >> settings.useProxy;
        in >> settings.proxyHost;
        in >> settings.proxyPort;
        in >> settings.proxyUsername;
        in >> settings.proxyPassword;
        in >> settings.informStable;
        in >> settings.informUnstable;
        in >> settings.ballonMessageReminder;
        in >> settings.messageBoxReminder;
        in >> settings.autoDownload;
        in >> settings.enableAutoStart;
        in >> settings.loadAfterDays;
        in >> settings.btjunkie;
        in >> settings.isohunt;
        in >> settings.piratebay;
        in >> settings.googleUsername;
        in >> settings.googlePassword;
        in >> settings.enableGReminder;
        in >> settings.googleReminderType;
        in >> settings.googleReminderTime;
        in >> settings.googleReminderUnit;

        file.close();
    }
    else
    {
        qDebug() << "@Settings::loadSettings: couldn't open settingsfile";

        newSettings = true;
        /* set default values */
        settings.minutes = DEFAULT_MINUTES;
        settings.proxyHost = DEFAULT_PROXY;
        settings.proxyPassword = DEFAULT_PASSWORD;
        settings.proxyPort = DEFAULT_PORT;
        settings.proxyUsername = DEFAULT_USERNAME;
        settings.useProxy = DEFAULT_USEPROXY;
        settings.informStable = DEFAULT_INFORMSTABLE;
        settings.informUnstable = DEFAULT_INFORMUNSTABLE;
        settings.ballonMessageReminder = DEFAULT_BALLOONMESSAGEREMINDER;
        settings.messageBoxReminder = DEFAULT_MESSAGEBOXREMINDER;
        settings.messageBoxReminder = DEFAULT_AUTODOWNLOAD;
        settings.enableAutoStart = DEFAULT_AUTOSTART;
        settings.loadAfterDays = DEFAULT_LOADAFTERDAYS;
        settings.btjunkie = 1;
        settings.isohunt = 1;
        settings.piratebay = 1;
        settings.enableGReminder = 0;
        settings.googleReminderType = 0;
        settings.googleReminderTime = 1;
        settings.googleReminderUnit = 2;    // days
    }

    /* set all the ui-elements according to them */
    m_ui->spinBoxMinutes->setValue(settings.minutes);
    setCheckBoxInt(settings.useProxy, m_ui->checkBoxProxy);
    setCheckBoxInt(settings.informStable, m_ui->checkBoxStable);
    setCheckBoxInt(settings.informUnstable, m_ui->checkBoxUnstable);
    m_ui->lineEditProxy->setText(settings.proxyHost);
    m_ui->spinBoxPort->setValue(settings.proxyPort);
    m_ui->lineEditUsername->setText(settings.proxyUsername);
    m_ui->lineEditPassword->setText(settings.proxyPassword);
    setCheckBoxInt(settings.ballonMessageReminder, m_ui->checkBoxSysTray);
    setCheckBoxInt(settings.messageBoxReminder, m_ui->checkBoxMsgBox);
    setCheckBoxInt(settings.autoDownload, m_ui->checkBoxAutoDownload);
    setCheckBoxInt(settings.enableAutoStart, m_ui->checkBoxAutostart);
    m_ui->spinBoxDaysAfter->setValue(settings.loadAfterDays);
    setCheckBoxInt(settings.btjunkie, m_ui->checkBoxBtjunkie);
    setCheckBoxInt(settings.isohunt, m_ui->checkBoxIsohunt);
    setCheckBoxInt(settings.piratebay, m_ui->checkBoxPiratebay);
    m_ui->lineEditGUsername->setText(settings.googleUsername);
    m_ui->lineEditGPassword->setText(settings.googlePassword);
    setCheckBoxInt(settings.enableGReminder, m_ui->checkBoxGReminder);
    m_ui->comboBoxGReminderType->setCurrentIndex(settings.googleReminderType);
    m_ui->spinBoxGReminderTime->setValue(settings.googleReminderTime);
    m_ui->comboBoxGReminderUnit->setCurrentIndex(settings.googleReminderUnit);


    if(newSettings)
        this->saveSettings();
}

void Settings::saveSettings(void)
{
    SettingsData tmp;
    qDebug() << "@Settings::saveSettings: save settings";

    /* hide the settings-window first */
    this->hide();

    /* set the tmp-element according to the ui-elements */
    tmp.minutes = m_ui->spinBoxMinutes->value();
    tmp.useProxy = getCheckBoxInt(m_ui->checkBoxProxy);
    tmp.informStable = getCheckBoxInt(m_ui->checkBoxStable);
    tmp.informUnstable = getCheckBoxInt(m_ui->checkBoxUnstable);
    tmp.proxyHost = m_ui->lineEditProxy->text();
    tmp.proxyPort = m_ui->spinBoxPort->value();
    tmp.proxyUsername = m_ui->lineEditUsername->text();
    tmp.proxyPassword = m_ui->lineEditPassword->text();
    tmp.ballonMessageReminder = getCheckBoxInt(m_ui->checkBoxSysTray);
    tmp.messageBoxReminder= getCheckBoxInt(m_ui->checkBoxMsgBox);
    tmp.autoDownload= getCheckBoxInt(m_ui->checkBoxAutoDownload);
    tmp.enableAutoStart = getCheckBoxInt(m_ui->checkBoxAutostart);
    tmp.loadAfterDays = m_ui->spinBoxDaysAfter->value();
    tmp.btjunkie = getCheckBoxInt(m_ui->checkBoxBtjunkie);
    tmp.isohunt = getCheckBoxInt(m_ui->checkBoxIsohunt);
    tmp.piratebay = getCheckBoxInt(m_ui->checkBoxPiratebay);
    tmp.googleUsername = m_ui->lineEditGUsername->text();
    tmp.googlePassword = m_ui->lineEditGPassword->text();
    tmp.enableGReminder = getCheckBoxInt(m_ui->checkBoxGReminder);
    tmp.googleReminderType = m_ui->comboBoxGReminderType->currentIndex();
    tmp.googleReminderTime = m_ui->spinBoxGReminderTime->value();
    tmp.googleReminderUnit = m_ui->comboBoxGReminderUnit->currentIndex();

    /* save it to a file */
    QFile file(QCoreApplication::applicationDirPath() + "/" + SETTINGSFILE);
    file.flush();
    if (file.open(QIODevice::WriteOnly))
    {
        qDebug() << "@Settings::saveSettings: writing settingsfile";
        QDataStream out(&file);    // write the data serialized to the file

        out << tmp.minutes;
        out << tmp.useProxy;
        out << tmp.proxyHost;
        out << tmp.proxyPort;
        out << tmp.proxyUsername;
        out << tmp.proxyPassword;
        out << tmp.informStable;
        out << tmp.informUnstable;
        out << tmp.ballonMessageReminder;
        out << tmp.messageBoxReminder;
        out << tmp.autoDownload;
        out << tmp.enableAutoStart;
        out << tmp.loadAfterDays;
        out << tmp.btjunkie;
        out << tmp.isohunt;
        out << tmp.piratebay;
        out << tmp.googleUsername;
        out << tmp.googlePassword;
        out << tmp.enableGReminder;
        out << tmp.googleReminderType;
        out << tmp.googleReminderTime;
        out << tmp.googleReminderUnit;
    }
    else
        qDebug() << "@Settings::saveSettings: couldn't open settingsfile";
    file.close();

    /* emit a signal to the mainwindow and close the settings-dialog */
    emit(settingsChanged(tmp));
}

void Settings::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Settings::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->hide();
}

/* sets a checkbox according to a quint16 variable */
void Settings::setCheckBoxInt(qint16 value, QCheckBox *box)
{
    if (value == 1)
        box->setCheckState(Qt::Checked);
    else
        box->setCheckState(Qt::Unchecked);
}

/* gets the value of a checkbox in a qint16 variable */
qint16 Settings::getCheckBoxInt(QCheckBox *box)
{
    if (box->checkState() == Qt::Checked)
        return 1;
    else
        return 0;
}
