#ifndef SHOWVIEW_H
#define SHOWVIEW_H

#include <QList>
#include <QIcon>

#include "defines.h"
#include "viewabstract.h"


class showView : public viewAbstract
{
  Q_OBJECT

  public:
    showView(QWidget* parent = 0);

    QBrush highlightBrush(void) const;
    QBrush mouseOverBrush(void) const;
    QBrush oldBrush(void) const;

    QIcon downloadIcon(void) const;
    QIcon downloadAltIcon(void) const;
    QIcon infoIcon(void) const;

    bool areIconsValid(void) const;

    QSize minimumSizeHint(void) const;

 public slots:
    void addEpisode(ICSData episode);
    void clearEpisodes(void);
    void sortShows();

    void setHighlightBrush(QBrush brush);
    void setMouseOverBrush(QBrush brush);
    void setOldBrush(QBrush brush);

    void setDownloadIcon(QIcon icon);
    void setDownloadAltIcon(QIcon icon);
    void setInfoIcon(QIcon icon);

    void enableDate(bool _showDate = true) { showDate = _showDate; }
    void markTodaysShows(bool _markToday = true) { markToday = _markToday; }

  protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void leaveEvent(QEvent* event);

    int     dataCount(void) const;
    QString data2str(int index) const;
    QRect   data2Rect(int index) const;

    QRect downloadIconRect(int index) const;
    QRect downloadAltIconRect(int index) const;
    QRect infoIconRect(int index) const;

  protected slots:
    void episodeClicked(int index);
    void mouseOver(int index);

  private:
    QList<ICSData> episodes;

    QBrush p_highlightBrush;
    QBrush p_oldBrush;
    QBrush p_mouseOverBrush;
    int    p_mouseOverIndex;         //stores index where mouse is over episode
    int    p_mouseOverDownloadIcon;
    int    p_mouseOverDownloadAltIcon;
    int    p_mouseOverInfoIcon;
    bool   showDate;
    bool   markToday;

    QIcon p_downloadIcon;
    QIcon p_downloadAltIcon;
    QIcon p_infoIcon;

    QRect oldIconRect;  //to accelerate "mouse over" algorithm

  signals:
    void episodeClicked(ICSData episode);
    void downloadIconClicked(ICSData episode);
    void downloadAltIconClicked(ICSData episode);
    void infoIconClicked(ICSData episode);
};

#endif // SHOWVIEW_H
