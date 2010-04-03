#include "viewabstract.h"

#include <QString>
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QApplication>

viewAbstract::viewAbstract(QWidget* parent) : QWidget(parent)
{
  setMouseTracking(true);
  setDataBrush(QApplication::palette().brush(QPalette::Window));
}

QBrush viewAbstract::dataBrush(void)
{
  return p_dataBrush;
}

QSize viewAbstract::minimumSizeHint(void) const
{
  QSize size;
  int width;

  if(dataCount() > 0)
  {
    /* find max width */
    width = data2Rect(0).width();
    for(int i=0; i<dataCount(); i++)
      width = qMax(width,data2Rect(i).width());

    size.setWidth(width+2*margin);
    size.setHeight(margin+dataCount()*(data2Rect(0).height()+margin));
  }

  return size;
}

/*****************************************************************************
                                    SLOTS
******************************************************************************/
void viewAbstract::setDataBrush(QBrush brush)
{
  p_dataBrush = brush;
}

/*****************************************************************************
                                    Events
******************************************************************************/
void viewAbstract::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    for(int i=0; i<dataCount(); i++)
    {
      QPainterPath path;
      path.addRoundedRect(data2Rect(i),10,10);

      /* find out if bounding rect of element contains mouse press position */
      if(path.contains(event->posF()))
      {
        emit clicked(i);
        break;
      }
    }
  }

  QWidget::mousePressEvent(event);
}

void viewAbstract::mouseMoveEvent(QMouseEvent* event)
{
  int over = -1;  //send -1 if mouse is not over an element

  for(int i=0; i<dataCount(); i++)
  {
    QPainterPath path;
    path.addRoundedRect(data2Rect(i),10,10);

    if(path.contains(event->posF()))
    {
      over = i;  //send index of element where mouse is over
      break;
    }
  }

  emit mouseOver(over);

  QWidget::mouseMoveEvent(event);
}

void viewAbstract::leaveEvent(QEvent* event)
{
  emit mouseOver(-1);  //mouse leave widget -> thus mouse is not over an element
  QWidget::leaveEvent(event);
}

void viewAbstract::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event);  //to avoid warnings

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing,true);
  painter.setBrush(dataBrush());

  QString str;
  QRect r;

  for (int i=0; i<dataCount(); i++)  //draw all elements
  {
    str = data2str(i);
    r = data2Rect(i);

    paintShadow(&painter,r);

    painter.setBrush(dataBrush());
    painter.drawRoundedRect(r,10,10);
    painter.drawText(r,Qt::AlignCenter,str);
  }
}

void viewAbstract::paintShadow(QPainter* painter, QRect shadowArea)
{
  QBrush shadowBrush(QColor(0x58,0x58,0x58,100));

  shadowArea.translate(4,3);

  painter->save();

  painter->setPen(Qt::NoPen);
  painter->setBrush(shadowBrush);
  painter->drawRoundedRect(shadowArea,10,10);

  painter->restore();
}
