#include "showstoday.h"
#include "ui_showstoday.h"

//function to compare ICSData types
bool ICSDataLessThan2(const ICSData& d1, const ICSData& d2);

ShowsToday::ShowsToday(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ShowsToday)
{
    ready = false;
    m_ui->setupUi(this);

    /* setup showViewer */
    m_ui->showViewer->enableDate(false);    // DO NOT show the date (because all shows are TODAY!)
    m_ui->showViewer->markTodaysShows(false);
    QRadialGradient highlightGradient(0.5,0.5,0.9,0,0);
    highlightGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    highlightGradient.setColorAt(0,QColor(Qt::white));
    highlightGradient.setColorAt(1,QColor(0xFF,100,100));
    m_ui->showViewer->setHighlightBrush(highlightGradient);

    QRadialGradient oldGradient(0.5,0.5,0.9,0,0);
    oldGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    oldGradient.setColorAt(0,QColor(Qt::white));
    oldGradient.setColorAt(1,QColor(0,200,0xFF));
    m_ui->showViewer->setOldBrush(oldGradient);

    m_ui->dateEdit->setDate(QDate::currentDate());

    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(close()));
    connect(m_ui->showViewer, SIGNAL(episodeClicked(ICSData)), parent, SLOT(episodeClicked(ICSData)));
    connect(m_ui->dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(showShows(QDate)));
    connect(m_ui->buttonToday, SIGNAL(clicked()), this, SLOT(showShowsToday()));
}

ShowsToday::~ShowsToday()
{
    delete m_ui;
}

void ShowsToday::setShows(QList<ICSData> *_records)
{
    records = _records;
    ready = true;
    showShows(m_ui->dateEdit->date());
}

void ShowsToday::showShows(QDate date)
{
    QString stringName, stringDetails;
    QList<ICSData> list;

    if (!ready)
        return;

    /* create a new list with only the episodes of today */
    for (int pos=0; pos<records->size(); pos++)
    {
        if (records->at(pos).start == date)
        {
            list.append(records->at(pos));
        }
    }

    /* sort the new list */
    qSort(list.begin(),list.end(),ICSDataLessThan2);

    /* clear old showViewer */
    m_ui->showViewer->clearEpisodes();

    /* set the showViewer*/
    for (int pos=0; pos<list.size(); pos++)
    {
        m_ui->showViewer->addEpisode(list.at(pos));
    }
}

void ShowsToday::showShowsToday(void)
{
    m_ui->dateEdit->setDate(QDate::currentDate());
}

void ShowsToday::changeEvent(QEvent *e)
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

void ShowsToday::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->hide();
}

void ShowsToday::showEvent(QShowEvent *event)
{
    m_ui->dateEdit->setDate(QDate::currentDate());
    showShows(QDate::currentDate());
    event->accept();
}

bool ICSDataLessThan2(const ICSData& d1, const ICSData& d2)
{
  if (d1.show < d2.show)
    return true;

  return false;
}
