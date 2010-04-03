#ifndef SHOWSTODAY_H
#define SHOWSTODAY_H

#include "defines.h"

#include <QtGui/QDialog>
#include <QCloseEvent>

namespace Ui {
    class ShowsToday;
}

class ShowsToday : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(ShowsToday)
public:
    explicit ShowsToday(QWidget *parent = 0);
    virtual ~ShowsToday();

protected:
    virtual void changeEvent(QEvent *e);

public slots:
    void setShows(QList<ICSData> *_records);
    void showShows(QDate date);
    void showShowsToday(void);

private:
    Ui::ShowsToday *m_ui;
    bool ready;
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

    QList<ICSData> *records;
};

#endif // SHOWSTODAY_H
