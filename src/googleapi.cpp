#include "googleapi.h"
#include "defines.h"

#include <QHttp>
#include <QFile>

GoogleApi::GoogleApi()
{
    http = new QHttp;

    /* connect signals and slots for request */
    connect(http, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
    connect(http, SIGNAL(readyRead(QHttpResponseHeader)), this, SLOT(readyRead(QHttpResponseHeader)));

    jobCreateEvent = jobEventExists = jobLogin = false;
}

void GoogleApi::setProxy(QString host, qint16 port, QString username, QString password)
{
    http->setProxy(host, port, username, password);
}

void GoogleApi::clientLogin(QString email, QString password, QString source, QString service)
{
    QHttpRequestHeader header;
    header.setRequest("POST", "/accounts/ClientLogin");

    /* prepare header and raw-data */
    header.setValue("content-type", "application/x-www-form-urlencoded");
    data.append("Email="   + email.toAscii().toPercentEncoding()    + "&");
    data.append("Passwd="  + password.toAscii().toPercentEncoding() + "&");
    data.append("source="  + source.toAscii().toPercentEncoding()   + "&");
    data.append("service=" + service.toAscii().toPercentEncoding()  + "&");
    header.setValue("Content-length", QString::number(data.length()));

    /* make SSL POST request */
    http->setHost("www.google.com", QHttp::ConnectionModeHttps);
    http->request(header, data);
    jobLogin = true;
    oldHeader = header;
    data.clear();       // clear data immediately to be shure not to use login-data in any unecrypted connection
}

void GoogleApi::createCalendarEvent(QString title, QString content, QString where, QDateTime start, QDateTime end, int reminderType, int reminderMinutes)
{
    QHttpRequestHeader header;

    /* prepare data-string in xml */
    data = "<entry xmlns='http://www.w3.org/2005/Atom' xmlns:gd='http://schemas.google.com/g/2005'>\n";
    data.append("<category scheme='http://schemas.google.com/g/2005#kind' term='http://schemas.google.com/g/2005#event'/>\n");
    data.append("<title type='text'>" + title + "</title>\n");
    data.append("<content type='text'>" + content + "</content>\n");
    data.append("<gd:transparency value='http://schemas.google.com/g/2005#event.opaque'/>\n");
    data.append("<gd:eventStatus value='http://schemas.google.com/g/2005#event.confirmed'/>\n");
    data.append("<gd:where valueString='" + where + "'/>\n");
    data.append("<gd:when startTime='" + QString("%1").arg(start.date().year()) + "-" + QString("%1").arg(start.date().month(),2,10,QChar('0')) + "-"+ QString("%1").arg(start.date().day(),2,10,QChar('0')) + "T" + QString("%1").arg(start.time().hour(),2,10,QChar('0')) + ":" + QString("%1").arg(start.time().minute(),2,10,QChar('0')) + ":" + QString("%1").arg(start.time().second(),2,10,QChar('0')) + "." + QString("%1").arg(start.time().msec(),3,10,QChar('0')) + "Z' endTime='"
                 + QString("%1").arg(end.date().year()) + "-" + QString("%1").arg(end.date().month(),2,10,QChar('0')) + "-" + QString("%1").arg(end.date().day(),2,10,QChar('0')) + "T" + QString("%1").arg(end.time().hour(),2,10,QChar('0')) + ":" + QString("%1").arg(end.time().minute(),2,10,QChar('0')) + ":" + QString("%1").arg(end.time().second(),2,10,QChar('0')) + "." + QString("%1").arg(end.time().msec(),3,10,QChar('0')) + "Z'>\n");
    if (reminderMinutes!=-1 && reminderType!=-1)
    {
        switch(reminderType)
        {
            case 0:
                data.append(QString("<gd:reminder minutes='%1' method='email'/>").arg(reminderMinutes));
            break;
            case 1:
                data.append(QString("<gd:reminder minutes='%1' method='sms'/>").arg(reminderMinutes));
            break;
            case 2:
                data.append(QString("<gd:reminder minutes='%1' method='alert'/>").arg(reminderMinutes));
            break;
         }
    }
    data.append("</gd:when>");
    data.append("</entry>\n ");

    /* prepare header */
    header.setRequest("POST", "/calendar/feeds/default/private/full");
    header.setValue("content-type", "application/atom+xml");
    header.setValue("Content-length", QString::number(data.length()));
    header.setValue("Authorization", "GoogleLogin auth=" + auth);

    /* make POST request */
    http->setHost("www.google.com", QHttp::ConnectionModeHttp);
    http->request(header, data);
    jobCreateEvent = true;
    oldHeader = header;
}

void GoogleApi::eventExists(QString title)
{
    QHttpRequestHeader header;

    /* prepare header */
    //title.replace(" ", "+");
    data.clear();
    title = QString(title.toAscii().toPercentEncoding());
    header.setRequest("GET", "/calendar/feeds/default/private/full?q=" + title);
    header.setValue("Authorization", "GoogleLogin auth=" + auth);

    /* make GET request */
    http->setHost("www.google.com", QHttp::ConnectionModeHttp);
    http->request(header);
    jobEventExists = true;
    oldHeader = header;
}

void GoogleApi::readyRead(QHttpResponseHeader header)
{
    /* 302 is a redirect which we have to accept and handle */
    if (header.statusCode() == 302)
    {
        QString newLocation = header.value("Location");
        newLocation = newLocation.mid(newLocation.indexOf("/", 10));
        if (oldHeader.method() == "POST")
            oldHeader.setRequest("POST", newLocation);
        else if(oldHeader.method() == "GET")
            oldHeader.setRequest("GET", newLocation);
        QString newHost = header.value("Location");
        newHost = newHost.section("/", 2, 2);
        http->setHost(newHost, QHttp::ConnectionModeHttp);
        http->request(oldHeader, data);
        
    }
    else if (jobEventExists && header.statusCode() == 200)
    {
        httpData = http->readAll();

        if (httpData.contains("totalResults"))
        {
            jobEventExists = false;
            int start = httpData.indexOf("totalResults>")+13;
            int end = httpData.indexOf("<", start)-1;
            int results = httpData.mid(start, end-start+1).toInt();

            if (results > 0)
                emit eventExistent(true);
            else
                emit eventExistent(false);
        }
    }
    else if(jobCreateEvent && (header.statusCode() == 200 || header.statusCode() == 201))
    {
        jobCreateEvent = false;
        emit ready();
    }
    else if (jobLogin && header.statusCode() == 200)
    {
        jobLogin = false;
        auth = QString(http->readAll());
        auth = auth.section("=", 3, 3).trimmed(); // get authToken for further requests
        emit loggedIn();
    }
    else if (jobLogin && header.statusCode() == 403)
    {
        jobLogin = false;
        emit loginError();
    }
    /* 400 is an error */
    else if (header.statusCode() == 400)
    {
        if (jobEventExists) // get part 2 of the search-return
        {
            httpData += http->readAll();
            if (httpData.contains("totalResults"))
            {
                int start = httpData.indexOf("totalResults>")+13;
                int end = httpData.indexOf("<", start)-1;
                int results = httpData.mid(start, end-start+1).toInt();

                if (results > 0)
                    emit eventExistent(true);
                else if (results == 0)
                    emit eventExistent(false);
                jobEventExists = false;
            }
        }
        else
        {
            emit error(header.toString()+http->readAll());
        }
    }
    //else
        //qDebug() << "smth. else code: " << header.statusCode() << " data: " << http->readAll();
}

void GoogleApi::sslErrors(QList<QSslError> errors)
{
    for (int pos=0; pos<errors.size(); pos++)
        emit error(errors.at(pos).errorString());
}
