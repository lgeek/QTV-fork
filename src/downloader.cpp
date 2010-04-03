#include "downloader.h"

#include <QUrl>
#include <QFileInfo>
#include <QHttp>
#include <QNetworkAccessManager>
#include <QEventLoop>

Downloader::Downloader(void)
{
    reply = 0;
    reply2 = 0;
    reply3 = 0;
}

void Downloader::download(QUrl url, QString filename)
{
    errorOccured = false;
    QNetworkAccessManager manager;
    QEventLoop loop;
    
    if(proxyUsed == 1)
    {
        manager.setProxy(proxy);
    }
    reply = manager.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
    loop.exec();

    if (reply != 0)
    {
        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());

        delete reply;
        reply = 0;
        file.close();

        if(!errorOccured)
            emit(finished());
    }
}

void Downloader::download(QUrl url, QString *data)
{
    QNetworkAccessManager manager;
    QEventLoop loop;

    if(proxyUsed == 1)
    {
        manager.setProxy(proxy);
    }
    reply = manager.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
    loop.exec();

    if (reply!=0)
    {
        *data = reply->readAll();
        delete reply;
        reply = 0;
    }
}

QByteArray Downloader::download(QUrl url)
{
    QNetworkAccessManager manager;
    QEventLoop loop;
    QByteArray data;

    if(proxyUsed == 1)
    {
        manager.setProxy(proxy);
    }
    reply = manager.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
    loop.exec();

    if (reply!=0)
    {
        data = reply->readAll();
        delete reply;
        reply = 0;
    }
    return data;
}

void Downloader::getCurrentStableVersion(QString &version)
{
    QNetworkAccessManager manager;
    QEventLoop loop;

    if(proxyUsed == 1)
    {
        manager.setProxy(proxy);
    }
    reply2 = manager.get(QNetworkRequest(QUrl(URLSTABLE)));
    connect(reply2, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply2, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
    loop.exec();

    if (reply2 != 0)
    {
        version = reply2->readAll();
        delete reply2;
        reply2 = 0;
    }
}
void Downloader::getCurrentUnstableVersion(QString &version)
{
    QNetworkAccessManager manager;
    QEventLoop loop;

    if(proxyUsed == 1)
    {
        manager.setProxy(proxy);
    }
    reply3 = manager.get(QNetworkRequest(QUrl(URLUNSTABLE)));
    connect(reply3, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply3, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
    loop.exec();

    if (reply3 != 0)
    {
        version = reply3->readAll();
        delete reply3;
        reply3 = 0;
    }
}

void Downloader::handleError(QNetworkReply::NetworkError err)
{
    errorOccured = true;
    emit(error(err,reply->errorString()));
}

void Downloader::setProxyUsed(qint16 _proxyUsed)
{
    proxyUsed = _proxyUsed;
}

void Downloader::setProxyData(QString host, qint16 port, QString username, QString password)
{
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName(host),
    proxy.setPort(port);
    proxy.setUser(username);
    proxy.setPassword(password);
}
