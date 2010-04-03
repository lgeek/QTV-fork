#ifndef REMEMBERVIEW_H
#define REMEMBERVIEW_H

#include "viewabstract.h"
#include "defines.h"

class ICSParser;

class rememberView : public viewAbstract
{
  Q_OBJECT

  public:
    rememberView(QWidget* parent = 0);

    QString noShowMessage(void) const;
    ICSParser* parser(void) const;

    QBrush episodeBrush(void);
    QBrush showBrush(void);
    QBrush highlightBrush(void);

    QSize minimumSizeHint(void) const;
    QSize sizeHint(void) const;

  public slots:
    void setParser(ICSParser* parser);
    void setRecords(QList<RecordData> records);
    void clearAll(void);

    void setNoShowMessage(QString message);

    void setEpisodeBrush(QBrush brush);
    void setShowBrush(QBrush brush);
    void setHighlightBrush(QBrush brush);

  protected:
    void paintEvent(QPaintEvent* event);

    int     dataCount(void) const;
    QString data2str(int index) const;
    QRect   data2Rect(int index) const;

  protected slots:
    void mouseOver(int index);
    void mouseClicked(int index);

  private:
    QList<RecordData> records;
    ICSParser* p_parser;

    QString p_noShowMessage;
    int expanded;
    int highlightElement;

    QBrush p_episodeBrush;
    QBrush p_highlightBrush;

    QList<ICSData> searchNewEpisodes(QString show) const;
    QString ics2Str(ICSData ics) const;
    QRect   ics2Rect(ICSData ics) const;
};

#endif // REMEMBERVIEW_H
