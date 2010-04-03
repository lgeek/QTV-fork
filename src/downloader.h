#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QUrl>
#include <QNetworkProxy>
#include <QNetworkReply>

#include "defines.h"

class QNetworkReply;
class QFile;

class Downloader : public QObject
{
    Q_OBJECT

public:
    Downloader(void);

public slots:
    void download(QUrl url, QString filename);
    void download(QUrl url, QString *data);
    QByteArray download(QUrl url);
    void setProxyData(QString host, qint16 port, QString username, QString password);
    void setProxyUsed(qint16 _proxyUsed);
    void getCurrentStableVersion(QString &version);
    void getCurrentUnstableVersion(QString &version);

private:
    QNetworkProxy proxy;
    qint16 proxyUsed;
    bool errorOccured;
    QNetworkReply *reply, *reply2, *reply3;

private slots:
    void handleError(QNetworkReply::NetworkError err);

signals:
    void finished(void);
    void error(QNetworkReply::NetworkError err, QString errorString);
};

#endif // DOWNLOADER_H
