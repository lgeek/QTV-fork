#ifndef REMINDER_H
#define REMINDER_H

#include "defines.h"

#include <QObject>

class ICSParser;
class SystemTray;
class MsgBox;
class QAbstractButton;
class QSystemTrayIcon;
class Downloader;
class GoogleApi;

struct RemState
{
    QString show;
    bool reminded;
};

class Reminder : public QObject
{
    Q_OBJECT;
public:
    Reminder();
    ~Reminder();

public slots:
    void checkEpisodes(QList<ICSData> *records, QList<RecordData> *recordDataList, QSystemTrayIcon *tray, SettingsData *settings);
    void messageAccepted(void);

    /* gapi */
    void eventExistent(bool);
    void handleError(QString);
    void loggedIn(void);
    void gapiReady(void);

signals:
    void newDay(void);
    void updateStatusBar(QString);

private:
    QList<RemState> remState;
    QList<RemState> remStateTmp;
    MsgBox *msg;
    Downloader *downloader;
    // gapi
    GoogleApi *gapi;
    QList<ICSData> gReminderList;
    int gReminderListIndex;
    int reminderType, reminderMinutes;
};

#endif // REMINDER_H
