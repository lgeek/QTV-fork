#include "queue.h"
#include "ui_queue.h"
#include "rememberview.h"

#include <QRadialGradient>

//function to compare RecordData types
bool RecordDataLessThan(const RecordData& d1, const RecordData& d2);

Queue::Queue(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::Queue)
{
    m_ui->setupUi(this);

    QRadialGradient showGradient(0.5,0.5,0.9,0,0);
    showGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    showGradient.setColorAt(0,QColor(Qt::white));
    showGradient.setColorAt(1,QColor(0,200,0xFF));

    QRadialGradient episodeGradient(0.5,0.5,1,0,0);
    episodeGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    episodeGradient.setColorAt(0,QColor(Qt::white));
    episodeGradient.setColorAt(1,QColor(0xFF,100,100));

    QRadialGradient highlightGradient(0.5,0.5,1,0,0);
    highlightGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    highlightGradient.setColorAt(0,QColor(Qt::white));
    highlightGradient.setColorAt(1,QColor(0xFA,0xFF,0x6C));

    m_ui->rememberViewer->setNoShowMessage(tr("There are currently no shows in the queue!"));
    m_ui->rememberViewer->setShowBrush(showGradient);
    m_ui->rememberViewer->setEpisodeBrush(episodeGradient);
    m_ui->rememberViewer->setHighlightBrush(highlightGradient);

    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(hide()));
}

Queue::~Queue()
{
    delete m_ui;
}

void Queue::setQueue(QList<RecordData> *recordDataList)
{
    QList<RecordData> list;

    /* create a new list with only the episodes of today */
    for (int pos=0; pos<recordDataList->size(); pos++)
    {
        list.append(recordDataList->at(pos));
    }

    /* sort the new list */
    qSort(list.begin(),list.end(),RecordDataLessThan);

    m_ui->rememberViewer->clearAll();
    m_ui->rememberViewer->setRecords(list);
}

void Queue::setParser(ICSParser* parser)
{
  m_ui->rememberViewer->setParser(parser);
}

void Queue::changeEvent(QEvent *e)
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

void Queue::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->hide();
}

bool RecordDataLessThan(const RecordData& d1, const RecordData& d2)
{
  if (d1.show < d2.show)
    return true;

  return false;
}
