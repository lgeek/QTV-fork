#ifndef QUEUE_H
#define QUEUE_H

#include <QtGui/QDialog>
#include <QCloseEvent>

#include "defines.h"

namespace Ui {
    class Queue;
}


class ICSParser;


class Queue : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(Queue)
public:
    explicit Queue(QWidget *parent = 0);
    virtual ~Queue();

public slots:
    void setQueue(QList<RecordData> *recordDataList);
    void setParser(ICSParser* parser);

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::Queue *m_ui;
    void closeEvent(QCloseEvent *event);
};

#endif // QUEUE_H
