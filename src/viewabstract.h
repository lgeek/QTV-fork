#ifndef VIEWABSTRACT_H
#define VIEWABSTRACT_H

#include <QWidget>

class viewAbstract : public QWidget
{
  Q_OBJECT

  public:
    viewAbstract(QWidget* parent = 0);

    QBrush dataBrush(void);

    QSize minimumSizeHint(void) const;

  public slots:
    void setDataBrush(QBrush brush);

  protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void leaveEvent(QEvent* event);

    virtual int     dataCount(void) const = 0;       //number of elements
    virtual QString data2str(int index) const = 0;   //returns output string
    virtual QRect   data2Rect(int index) const = 0;  //return bounding rect of output string

    void paintShadow(QPainter* painter, QRect shadowArea);

    enum {margin=5};

  private:
    QBrush p_dataBrush;  //background color

  signals:
    void clicked(int index);    //emit when an element was clicked
    void mouseOver(int index);  //emit when mouse is over an element
};

#endif // VIEWABSTRACT_H
