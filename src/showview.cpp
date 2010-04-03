#include "showview.h"
#include "icsparser.h"

#include <QString>
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QToolTip>

//function to compare ICSData types
bool ICSDataLessThan(const ICSData& d1, const ICSData& d2);

showView::showView(QWidget* parent) : viewAbstract(parent)
{
  /* set default colors */
  setHighlightBrush(QColor(Qt::red));
  setMouseOverBrush(QColor(0,0,0xFF,100));
  setOldBrush(QColor(Qt::blue));

  p_mouseOverIndex = -1;
  p_mouseOverDownloadIcon = -1;
  p_mouseOverDownloadAltIcon = -1;
  p_mouseOverInfoIcon = -1;
  showDate = true;
  markToday = true;

  /* set tooltip options */
  QFont font = QToolTip::font();
  font.setBold(true);
  QToolTip::setFont(font);

  connect(this, SIGNAL(clicked(int)), this, SLOT(episodeClicked(int)));
  connect(this, SIGNAL(mouseOver(int)), this, SLOT(mouseOver(int)));
}

QBrush showView::highlightBrush(void) const
{
  return p_highlightBrush;
}

QBrush showView::mouseOverBrush(void) const
{
  return p_mouseOverBrush;
}

QBrush showView::oldBrush(void) const
{
  return p_oldBrush;
}

QIcon showView::downloadIcon(void) const
{
  return p_downloadIcon;
}

QIcon showView::downloadAltIcon(void) const
{
  return p_downloadAltIcon;
}

QIcon showView::infoIcon(void) const
{
  return p_infoIcon;
}

bool showView::areIconsValid(void) const
{
  return (!p_downloadIcon.isNull() && !p_downloadAltIcon.isNull() && !p_infoIcon.isNull());
}

QSize showView::minimumSizeHint(void) const
{
  QSize size = viewAbstract::minimumSizeHint();

  if(dataCount() > 0 && areIconsValid())
    size += QSize(data2Rect(0).height()*3+4*margin+4,0);

  return size;
}

/*****************************************************************************
                                    SLOTS
******************************************************************************/
void showView::addEpisode(ICSData episode)
{
  episodes.append(episode);
  update();
  updateGeometry();
}

void showView::sortShows(void)
{
    qSort(episodes.begin(),episodes.end(),ICSParser::ICSDataLessThan);
}

void showView::clearEpisodes(void)
{
  episodes.clear();
  update();
  updateGeometry();
}

void showView::setHighlightBrush(QBrush brush)
{
  p_highlightBrush = brush;
  update();
}

void showView::setMouseOverBrush(QBrush brush)
{
  p_mouseOverBrush = brush;
  update();
}

void showView::setOldBrush(QBrush brush)
{
  p_oldBrush = brush;
  update();
}

void showView::setDownloadIcon(QIcon icon)
{
  p_downloadIcon = icon;
  update();
}

void showView::setDownloadAltIcon(QIcon icon)
{
  p_downloadAltIcon = icon;
  update();
}

void showView::setInfoIcon(QIcon icon)
{
  p_infoIcon = icon;
  update();
}

void showView::episodeClicked(int index)
{
  emit episodeClicked(episodes.at(index));
}

void showView::mouseOver(int index)
{
  if(index != p_mouseOverIndex)  //to decrease repaints
  {
    p_mouseOverIndex = index;
    update();
  }
}

/*****************************************************************************
                                    Events
******************************************************************************/
void showView::paintEvent(QPaintEvent* event)
{
  QDate date = QDate::currentDate();
  QString str;
  QRect r;
  int days;

  viewAbstract::paintEvent(event);  //let paint base class first
                                    //must be called before construct a new QPainter (to fix windows bug)

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing,true);

  //highlight all broadcasted shows
  //the next upcoming show will be extra-highlighted!
  for(int episode=0; episode<episodes.count(); episode++)
  {
    days = date.daysTo(episodes.at(episode).start);
    if(days <= 0)
    {
      str = data2str(episode);
      r = data2Rect(episode);

      if(days < 0)
      {
        painter.setBrush(oldBrush());
      }
      else
      {
          if (markToday)
              painter.setBrush(highlightBrush());
          else
              painter.setBrush(oldBrush());
      }

      painter.drawRoundedRect(r,10,10);
      painter.drawText(r,Qt::AlignCenter,str);
    }
    else
    {
        /* colorize also the future-shows if markToday is not set (used for showsToday with the date-selector) */
        if (!markToday)
        {
            str = data2str(episode);
            r = data2Rect(episode);
            painter.setBrush(oldBrush());
            painter.drawRoundedRect(r,10,10);
            painter.drawText(r,Qt::AlignCenter,str);
        }
        else
            break;
    }
  }

  //highlight episode where mouse is over
  if(p_mouseOverIndex >= 0 && p_mouseOverIndex < episodes.count())
  {
    r = data2Rect(p_mouseOverIndex);

    painter.setBrush(mouseOverBrush());
    painter.drawRoundedRect(r,10,10);
    QToolTip::hideText();
    QToolTip::showText(QCursor::pos(), episodes.at(p_mouseOverIndex).name + " " + tr("(click for details)"));

  }

  //draw icons
  if(areIconsValid())
  {
    painter.setBrush(Qt::NoBrush);
    for(int episode=0; episode<episodes.count(); episode++)
    {

      if(p_mouseOverInfoIcon == episode)
        painter.drawRect(infoIconRect(p_mouseOverInfoIcon).adjusted(-2,-2,2,2));

      if(p_mouseOverDownloadIcon == episode)
        painter.drawRect(downloadIconRect(p_mouseOverDownloadIcon).adjusted(-2,-2,2,2));

      if(p_mouseOverDownloadAltIcon == episode)
        painter.drawRect(downloadAltIconRect(p_mouseOverDownloadAltIcon).adjusted(-2,-2,2,2));

      p_infoIcon.paint(&painter,infoIconRect(episode));
      p_downloadIcon.paint(&painter,downloadIconRect(episode));
      p_downloadAltIcon.paint(&painter,downloadAltIconRect(episode));
    }
  }
}

void showView::mousePressEvent(QMouseEvent* event)
{
  viewAbstract::mousePressEvent(event);

  if(areIconsValid())
  {
    for(int i=0; i<dataCount(); i++)
    {
      QPainterPath downloadIconPath;
      QPainterPath downloadAltIconPath;
      QPainterPath infoIconPath;

      downloadIconPath.addRect(downloadIconRect(i));
      downloadAltIconPath.addRect(downloadAltIconRect(i));
      infoIconPath.addRect(infoIconRect(i));

      if(downloadIconPath.contains(event->posF()))
      {
        emit downloadIconClicked(episodes.at(i));
        break;
      }

      if(downloadAltIconPath.contains(event->posF()))
      {
        emit downloadAltIconClicked(episodes.at(i));
        break;
      }

      if(infoIconPath.contains(event->posF()))
      {
        emit infoIconClicked(episodes.at(i));
        break;
      }
    }
  }
}

void showView::mouseMoveEvent(QMouseEvent* event)
{
  viewAbstract::mouseMoveEvent(event);

  if(areIconsValid())
  {
    if(oldIconRect.contains(event->pos()) && oldIconRect.isValid())
      return;  //nothing to do -> mouse is still in same area

    p_mouseOverDownloadIcon = -1;
    p_mouseOverDownloadAltIcon = -1;
    p_mouseOverInfoIcon = -1;

    for(int i=0; i<dataCount(); i++)
    {
      QPainterPath downloadIconPath;
      QPainterPath downloadAltIconPath;
      QPainterPath infoIconPath;

      downloadIconPath.addRect(downloadIconRect(i));
      downloadAltIconPath.addRect(downloadAltIconRect(i));
      infoIconPath.addRect(infoIconRect(i));

      if(downloadIconPath.contains(event->posF()))
      {
        p_mouseOverDownloadIcon = i;
        oldIconRect = downloadIconRect(i);
        QToolTip::hideText();
        QToolTip::showText(QCursor::pos(), tr("Search for torrent"));
        update();

        break;
      }

      if(downloadAltIconPath.contains(event->posF()))
      {
        p_mouseOverDownloadAltIcon = i;
        oldIconRect = downloadAltIconRect(i);
        QToolTip::hideText();
        QToolTip::showText(QCursor::pos(), tr("Search on serienjunkies.org"));
        update();

        break;
      }

      if(infoIconPath.contains(event->posF()))
      {
        p_mouseOverInfoIcon = i;
        oldIconRect = infoIconRect(i);
        QToolTip::hideText();
        QToolTip::showText(QCursor::pos(), tr("Episode details on tv.com"));
        update();

        break;
      }
    }
  }
}

void showView::leaveEvent(QEvent* event)
{
  viewAbstract::leaveEvent(event);

  if(areIconsValid())
  {
    p_mouseOverDownloadIcon = -1;
    p_mouseOverDownloadAltIcon = -1;
    p_mouseOverInfoIcon = -1;
    update();
  }
}

/*****************************************************************************
                             protected functions
******************************************************************************/
int showView::dataCount(void) const
{
  return episodes.count();
}

QString showView::data2str(int index) const
{
  ICSData data;
  QString str;

  if(index < episodes.count())
  {
    data = episodes.at(index);

    if (showDate)
    {
        str =  QString(tr("%1 S%2E%3: %4")).arg(data.show)
                                       .arg(data.season,2,10,QChar('0'))
                                       .arg(data.episode,2,10,QChar('0'))
                                       .arg(data.start.toString("dd.MM.yyyy"),10,'0');
    }
    else
    {
        str =  QString(tr("%1 S%2E%3")).arg(data.show)
                                       .arg(data.season,2,10,QChar('0'))
                                       .arg(data.episode,2,10,QChar('0'));
    }
  }

  return str;
}

QRect showView::data2Rect(int index) const
{
  QFontMetrics fm(font());
  QString str;
  QRect rect;

  if(index < episodes.count())
  {
    str = data2str(index);

    rect = fm.boundingRect(str).adjusted(-margin,-margin,margin,margin);
    rect.moveTo(QPoint(margin,margin+index*(rect.height()+margin)));
  }

  return rect;
}

QRect showView::infoIconRect(int index) const
{
  int width = viewAbstract::minimumSizeHint().width();

  QRect r = data2Rect(index);
  QPoint start = r.topLeft() + QPoint(width+margin,0);

  return QRect(start,QSize(r.height(),r.height()));
}

QRect showView::downloadIconRect(int index) const
{
  int width = viewAbstract::minimumSizeHint().width();

  QRect r = data2Rect(index);
  QPoint start = r.topLeft() + QPoint(width+margin,0) + QPoint(r.height()+margin,0);

  return QRect(start,QSize(r.height(),r.height()));
}

QRect showView::downloadAltIconRect(int index) const
{
  int width = viewAbstract::minimumSizeHint().width();

  QRect r = data2Rect(index);
  QPoint start = r.topLeft() + QPoint(width+margin,0) + 2*QPoint(r.height()+margin,0);

  return QRect(start,QSize(r.height(),r.height()));
}
