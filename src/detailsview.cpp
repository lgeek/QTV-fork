#include "detailsview.h"
#include "ui_detailsview.h"
#include "downloader.h"
#include "imageloader.h"

DetailsView::DetailsView(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::DetailsView)
{
    m_ui->setupUi(this);

    downloader = new Downloader;

    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(hide()));
}

DetailsView::~DetailsView()
{
    delete m_ui;
}

void DetailsView::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DetailsView::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->hide();
}

void DetailsView::parseAndSetData(ICSData recordData, QUrl url)
{
    QString data;
    downloader->download(url, &data);

    data = ICSParser::htmlToUnicode(data).toLocal8Bit();
    if (data.contains("&#"))
        qDebug() << "@DetailsView: found some unicode-chars";

    // window-title
    this->setWindowTitle(recordData.show + ": " + recordData.name + " (tv.com)");

    if (data.contains("404 - Page Not Found"))
    {
        m_ui->labelSummary->setText(tr("No data found"));
        m_ui->labelHeader->setPixmap(QPixmap(":/images/face-sad").scaled(200, 200));
        m_ui->tabWidget->setEnabled(false);
        return;
    }
    else
    {
        m_ui->tabWidget->setEnabled(true);
    }

    // Summary
    m_ui->labelSummary->setText(getHttpClassBlock(&data, "episode_recap"));
    // Cast and Crew<
    m_ui->labelCastCrew->setText(getHttpClassBlock(&data, "episode_cast_summary"));
    // Trivia
    m_ui->labelTrivia->setText(getHttpClassBlock(&data, "name=\"trivia\""));
    // Notes
    m_ui->labelNotes->setText(getHttpClassBlock(&data, "name=\"notes\""));
    // Quotes
    m_ui->labelQuotes->setText(getHttpClassBlock(&data, "name=\"quotes\""));
    // Allusions
    m_ui->labelAllusions->setText(getHttpClassBlock(&data, "name=\"allusions\""));

    // search for header-image
    if (data.contains("content_headers"))
    {
        int start=0, end=0;
        QPixmap img;

        start = data.indexOf("content_headers");
        start = data.lastIndexOf("\"", start) + 1;
        end = data.indexOf("\"", start) - 1;
        // download and set image
        if (img.loadFromData(downloader->download(data.mid(start, end-start+1))))
        {
            m_ui->labelHeader->setPixmap(img);
        }
        else
        {
            qDebug() << "img NOT loaded!";
        }
    }
    // set default-show-image
    else
    {
        qDebug() << "no image found; using default (" + recordData.show + ": " + recordData.name + ")";
        m_ui->labelHeader->setPixmap(ImageLoader::getImage(recordData.show));
    }

    // select "summary" tab!
    m_ui->tabWidget->setCurrentIndex(0);
}

QString DetailsView::getHttpClassBlock(QString *data, QString className)
{
    int start=0, end=0;
    QString retData = tr("No data found");

    if (className.startsWith("name="))
    {
        start = data->indexOf(className);
        start = data->lastIndexOf("<div", start) - 1;
        start = data->lastIndexOf("<div", start) - 1;
        end = data->indexOf("</div>", start) + 7;
        end = data->indexOf("</div>", end);
        retData = data->mid(start, end-start+1);
    }
    else
    {
        start = data->indexOf(QString("id=\"%1\"").arg(className));
        start = data->lastIndexOf("<div", start);
        end = data->indexOf("</div>", start) + 7;
        end = data->indexOf("</div>", end);
        retData = data->mid(start, end-start+1);
    }

    return retData;
}
