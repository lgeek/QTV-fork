#ifndef GOOGLEAPI_H
#define GOOGLEAPI_H

#include <QObject>
#include <QSslError>
#include <QDateTime>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>

class QHttp;

class GoogleApi : public QObject
{
    Q_OBJECT

public:
    GoogleApi();

    void eventExists(QString title);

public slots:
    void setProxy(QString host, qint16 port, QString username, QString password);
    void clientLogin(QString email, QString password, QString source, QString service);
    void createCalendarEvent(QString title, QString content, QString where, QDateTime start, QDateTime end, int reminderType=-1, int reminderMinutes=-1);

signals:
    void error(QString errorString);
    void loggedIn(void);
    void eventExistent(bool);
    void ready(void);
    void loginError(void);

private:
    QHttp *http;
    QHttpRequestHeader oldHeader;
    QByteArray data;
    QString auth;
    QString httpData;
    bool jobLogin;
    bool jobCreateEvent;
    bool jobEventExists;

private slots:
    void sslErrors(QList<QSslError>);
    void readyRead(QHttpResponseHeader);
};

#endif // GOOGLEAPI_H
