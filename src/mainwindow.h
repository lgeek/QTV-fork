#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QCloseEvent>
#include <QModelIndex>
#include <QNetworkReply>
#include <QSystemTrayIcon>

#include "defines.h"

class Downloader;
class ICSParser;
class About;
class Settings;
class Queue;
class QListWidgetItem;
class QTimer;
class Reminder;
class QStandardItemModel;
class QSortFilterProxyModel;
class ShowsToday;
class MsgBox;
class InfoLoader;
class ImageLoader;
class DetailsView;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void update(void);
    void handleNewData(void);
    void itemChosen(const QModelIndex& index);
    void updateRecordSettings(void);
    void settingsChanged(SettingsData newSettings);
    void openDownload(ICSData recordData);
    void openDownloadAlt(ICSData recordData);
    void infoIconClicked(ICSData recordData);
    void episodeClicked(ICSData recordData);
    void networkError(QNetworkReply::NetworkError err, QString errorString);
    void openWebsite(void);
    void closeForce(void);
    void showQueue(void);

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QSystemTrayIcon  *systemTray;
    Downloader *downloader;
    ICSParser *parser;

    About *about;
    Settings *settings;
    Queue *queue;
    Reminder *reminder;
    ShowsToday *showsToday;
    MsgBox *msg;
    ImageLoader *imageLoader;
    InfoLoader *infoLoader;
    DetailsView *detailsView;

    QStandardItemModel* model;
    QSortFilterProxyModel* proxyModel;

    QList<RecordData> recordDataList;       // the list with all settings for every episodes
    SettingsData settingsData;
    bool closeForced;
    bool updateLock;

    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
    QBrush highlightBrush(void);            // highlight brush of remembered episodes (listView)
};

#endif // MAINWINDOW_H
