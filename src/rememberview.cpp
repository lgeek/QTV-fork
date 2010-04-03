#include "rememberview.h"
#include "icsparser.h"

#include <QPainter>


rememberView::rememberView(QWidget* parent) : viewAbstract(parent)
{
  setParser(0);
  expanded = -1;
  highlightElement = -1;

  setEpisodeBrush(Qt::blue);
  setShowBrush(Qt::red);
  setHighlightBrush(Qt::yellow);

  connect(this, SIGNAL(mouseOver(int)), this, SLOT(mouseOver(int)));
  connect(this, SIGNAL(clicked(int)), this, SLOT(mouseClicked(int)));
}

QString rememberView::noShowMessage(void) const
{
  return p_noShowMessage;
}

QBrush rememberView::episodeBrush(void)
{
  return p_episodeBrush;
}

QBrush rememberView::showBrush(void)
{
  return dataBrush();
}

QBrush rememberView::highlightBrush(void)
{
  return p_highlightBrush;
}

QSize rememberView::minimumSizeHint(void) const
{
  QSize size;
  int width, height;
  QFontMetrics fm(font());
  QRect rect;

  QList<ICSData> newEpisodes;


  //there are any show
  if(records.count() > 0)
  {
    //fist calculate size without expanded elements
    size = viewAbstract::minimumSizeHint();

    //elements are expanded
    if(expanded >= 0 && expanded < dataCount())
    {
      newEpisodes = searchNewEpisodes(records.at(expanded).show);

      //find out width and height
      width = size.width();
      height = size.height();
      for(int i=0; i<newEpisodes.count(); i++)
      {
        rect = ics2Rect(newEpisodes.at(i));

        width = qMax(width,rect.width() + 2 * margin);
        height += rect.height() + margin;
      }
      size.setWidth(width + 40);    // atm 40 is just a value which fixes the wrong size bug
      size.setHeight(height);
    }
  }
  else
  {
    rect = fm.boundingRect(noShowMessage()).adjusted(-margin,-margin,margin,margin);
    size = QSize(rect.width()+margin,rect.height()+margin);
  }

  return size;
}

QSize rememberView::sizeHint(void) const
{
  return minimumSizeHint();
}

ICSParser* rememberView::parser(void) const
{
  return p_parser;
}

/*****************************************************************************
                                    SLOTS
******************************************************************************/
void rememberView::setParser(ICSParser* parser)
{
  p_parser = parser;
}

void rememberView::setRecords(QList<RecordData> records)
{
  this->records = records;
  update();
  updateGeometry();
}

void rememberView::clearAll(void)
{
  records.clear();
  update();
  updateGeometry();
}

void rememberView::setNoShowMessage(QString message)
{
  p_noShowMessage = message;
}

void rememberView::setEpisodeBrush(QBrush brush)
{
  p_episodeBrush = brush;
}

void rememberView::setShowBrush(QBrush brush)
{
  setDataBrush(brush);
}

void rememberView::setHighlightBrush(QBrush brush)
{
  p_highlightBrush = brush;
}

void rememberView::mouseOver(int index)
{
  if(index != highlightElement)  //to increase repaints
  {
    highlightElement = index;
    update();
    updateGeometry();
  }
}

void rememberView::mouseClicked(int index)
{
  if(index == expanded)
    expanded = -1;
  else
    expanded = index;

  update();
  updateGeometry();
}

/*****************************************************************************
                             protected functions
******************************************************************************/
int rememberView::dataCount(void) const
{
  return records.count();
}

QString rememberView::data2str(int index) const
{
  RecordData rec = records.at(index);

  return QString(tr("%1: %n days","",rec.days)).arg(rec.show);
}

QRect rememberView::data2Rect(int index) const
{
  QFontMetrics fm(font());
  QString str = data2str(index);
  QRect rect;

  QList<ICSData> newEpisodes;
  int height = 0;

  if(index < records.count())
  {
    str = data2str(index);
    rect = fm.boundingRect(str).adjusted(-margin,-margin,margin,margin);

    if(index > expanded && (expanded >= 0 && expanded < dataCount()))
    {
      newEpisodes = searchNewEpisodes(records.at(expanded).show);

      /* calculate size of expanded elements and add them to result */
      for(int i=0; i<newEpisodes.count(); i++)
        height += ics2Rect(newEpisodes.at(i)).height() + margin;

      rect.moveTo(QPoint(margin,margin+index*(rect.height()+margin)+height));
    }
    else
    {
      rect.moveTo(QPoint(margin,margin+index*(rect.height()+margin)));
    }
  }

  return rect;
}

/*****************************************************************************
                                    EVENTS
******************************************************************************/
void rememberView::paintEvent(QPaintEvent* event)
{
  QFontMetrics fm(font());
  QRect rect;
  QString str;

  QList<ICSData> newEpisodes;
  int startY;
  int lineY;

  viewAbstract::paintEvent(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing,true);

  /* there are any shows */
  if(records.count() > 0)
  {
    /* there are any expanded elements */
    if(expanded >= 0 && expanded < dataCount())
    {
      newEpisodes = searchNewEpisodes(records.at(expanded).show);
      startY = margin+(expanded+1)*(data2Rect(0).height()+margin);  //calculate start position y of expanded elements

      /* draw vertical line */
      if(newEpisodes.count() > 0)
      {
        rect = ics2Rect(newEpisodes.at(0));
        painter.drawLine(margin+20,startY-margin,margin+20,startY+newEpisodes.count()*(rect.height()+margin)-margin-rect.height()/2);
      }

      /* draw all expanded elements */
      for(int i=0; i<newEpisodes.count(); i++)
      {
        rect = ics2Rect(newEpisodes.at(i));
        rect.moveTo(margin+30,startY+i*(ics2Rect(newEpisodes.at(i)).height()+margin));

        paintShadow(&painter,rect);

        /* draw horizontal line */
        lineY = startY+(i+1)*(rect.height()+margin)-margin-rect.height()/2;
        painter.drawLine(margin+20,lineY,margin+30,lineY);

        /* draw expanded element */
        painter.setBrush(episodeBrush());
        painter.drawRoundedRect(rect,10,10);
        painter.drawText(rect,Qt::AlignCenter,ics2Str(newEpisodes.at(i)));
      }
    }

    /* mouse is over an element -> draw number of new episodes and highlight element */
    if(highlightElement >= 0 && highlightElement < records.count())
    {
      newEpisodes = searchNewEpisodes(records.at(highlightElement).show);
      str = QString(tr("%n episode(s)","",newEpisodes.count()));

      rect = data2Rect(highlightElement);
      if(rect.width() < (fm.width(str)+2*margin))
        rect.setWidth(fm.width(str)+2*margin);

      painter.setBrush(highlightBrush());
      painter.drawRoundedRect(rect,10,10);
      painter.drawText(rect,Qt::AlignCenter,str);
    }
  }
  else
  { 
    /* draw no show message */
    rect = fm.boundingRect(noShowMessage()).adjusted(-margin,-margin,margin,margin);
    rect.moveTo(QPoint(margin,margin));

    painter.drawText(rect,Qt::AlignCenter,noShowMessage());
  }
}

/*****************************************************************************
                             private functions
******************************************************************************/
QList<ICSData> rememberView::searchNewEpisodes(QString name) const
{
  QDate date = QDate::currentDate();
  QList<ICSData> newEpisodes;
  QList<ICSData>* shows = parser()->getRecords();

  for(int i=0; i<shows->count(); i++)
  {
    if(shows->at(i).show == name && shows->at(i).start >= date)
    {
      newEpisodes.append(shows->at(i));
    }
  }

  return newEpisodes;
}

QString rememberView::ics2Str(ICSData ics) const
{
  QString str;
  int days = QDate::currentDate().daysTo(ics.start);

  if(days != 0)
  {
    str = QString(tr("S%1E%2 coming up in %n day(s)","",QDate::currentDate().daysTo(ics.start)))
                                                    .arg(ics.season,2,10,QChar('0'))
                                                    .arg(ics.episode,2,10,QChar('0'));
  }
  else
  {
    str = QString(tr("S%1E%2 coming up today")).arg(ics.season,2,10,QChar('0'))
                                               .arg(ics.episode,2,10,QChar('0'));
  }

  return str;
}

QRect rememberView::ics2Rect(ICSData ics) const
{
  QString str = ics2Str(ics);
  QFontMetrics fm(font());

  return fm.boundingRect(str).adjusted(-margin,-margin,margin,margin);
}
