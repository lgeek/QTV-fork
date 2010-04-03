#ifndef DETAILSVIEW_H
#define DETAILSVIEW_H

#include "defines.h"

#include <QtGui/QDialog>
#include <QCloseEvent>

namespace Ui {
    class DetailsView;
}

class Downloader;

class DetailsView : public QDialog {
    Q_OBJECT
public:
    DetailsView(QWidget *parent = 0);
    ~DetailsView();

    void parseAndSetData(ICSData recordData, QUrl url);

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    Ui::DetailsView *m_ui;
    Downloader *downloader;

    QString getHttpClassBlock(QString *data, QString className);
};

#endif // DETAILSVIEW_H
