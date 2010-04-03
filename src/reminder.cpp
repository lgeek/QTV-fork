#include "reminder.h"
#include "msgbox.h"
#include "downloader.h"
#include "infoloader.h"
#include "googleapi.h"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QSystemTrayIcon>

//function to compare RecordData types
bool RecordDataLessThan(const RecordData& d1, const RecordData& d2);

Reminder::Reminder()
{
    msg = new MsgBox;
    downloader = new Downloader();
    gapi = new GoogleApi;

    connect(msg, SIGNAL(messageClicked()), this, SLOT(messageAccepted()));
    connect(msg, SIGNAL(messageClicked()), msg, SLOT(hide()));

    connect(gapi, SIGNAL(loggedIn()), this, SLOT(loggedIn()));
    connect(gapi, SIGNAL(eventExistent(bool)), this, SLOT(eventExistent(bool)));
    connect(gapi, SIGNAL(error(QString)), this, SLOT(handleError(QString)));
    connect(gapi, SIGNAL(ready()), this, SLOT(gapiReady()));
}

Reminder::~Reminder()
{
    delete msg;
}

void Reminder::checkEpisodes(QList<ICSData> *records, QList<RecordData> *recordDataList, QSystemTrayIcon *tray, SettingsData *settings)
{
    QDate date = QDate::currentDate();
    QDir dir;
    static QDate oldDate = QDate::currentDate();
    qint16 days;
    RemState tmp;
    ICSData record;
    ShowInfo info;

    if (oldDate != date)    // new day => clear the already-reminded-list
    {
        remState.clear();
        emit newDay();
    }
    oldDate = date;

    QString remindString="", remindStringHTML="";
    bool remindNecessary = false;

    // google inits
    gReminderList.clear();      // clear the old google-reminder-list
    reminderType = settings->googleReminderType;
    switch(settings->googleReminderUnit)
    {
        case 0:
            reminderMinutes = settings->googleReminderTime;
            break;
        case 1:
            reminderMinutes = settings->googleReminderTime * 60;
            break;
        case 2:
            reminderMinutes = settings->googleReminderTime * 60 * 24;
            break;
        case 3:
            reminderMinutes = settings->googleReminderTime * 60 * 24 * 7;
            break;
    }

    // connect trayIcon stuff: use a tempory reminderString until the user clicks the message
    connect(tray, SIGNAL(messageClicked()), this, SLOT(messageAccepted()));

    qSort(recordDataList->begin(), recordDataList->end(), RecordDataLessThan);

    /* create the torrent-directory if it's not already there */
    if (!dir.exists(QCoreApplication::applicationDirPath() + "/" + TORRENTDIR))
    {
        dir.mkdir(QCoreApplication::applicationDirPath() + "/" + TORRENTDIR);
    }

    /* check */
    for(int dataPos=0; dataPos<recordDataList->size(); dataPos++)
    {
        for(int pos=0; pos<records->size(); pos++)
        {
            record = records->at(pos);
            if (record.show == recordDataList->at(dataPos).show)
            {
                bool alreadyReminded = false;
                bool alreadyDownloaded = false;
                int start = 0, end = 0;
                days = date.daysTo(records->at(pos).start);

                /* populate google-reminder-list */
                if (days >= 0 && !settings->googleUsername.isEmpty() && !settings->googlePassword.isEmpty())
                {
                    gReminderList.append(record);
                }

                /* autoDownload */
                if (days >= settings->loadAfterDays*-1 && days <= 0 && settings->autoDownload)
                {
                    QString data, urlStr;
                    QStringList urls;
                    QString naming = record.show + "+S" +
                                     QString("%1").arg(record.season,2,10,QChar('0')) + "E" +
                                     QString("%1").arg(record.episode,2,10,QChar('0'));
                    QString target = QCoreApplication::applicationDirPath() + "/" + TORRENTDIR + "/" + naming + ".torrent";

                    // jump to the end if torrent already exists!
                    if (!QFile::exists(target))
                    {
                        updateStatusBar(tr("Searching torrent for ") + naming.replace("+", " "));
                        /*BTJUNKIE; con: less up to date than fulldls; pro: torrent verifying! */
                        if (settings->btjunkie == 1)
                        {
                            urlStr = BTJUNKIESEARCH_URL;
                            qDebug() << "@Reminder: searching torrent for " << naming << " on btjunkie.org";
                            urlStr += naming;
                            urlStr = urlStr.replace(QChar(' '), QChar('+'));

                            // get torrent overview site urls
                            downloader->download(urlStr, &data);
                            for (int p=0; p<data.count("<a href=\"/torrent"); p++)
                            {
                                start = data.indexOf("<a href=\"/torrent", end) + 9;
                                end = data.indexOf("\"", start) - 1;
                                QString str = data.mid(start, end-start+1);
                                if (!str.contains("files"))
                                {
                                    urls.append(str);
                                }
                            }

                            if (urls.size() == 0)
                                qDebug() << "@Reminder: not found";

                            for (int urlP=0; urlP<urls.size(); urlP++)
                            {
                                bool torrentIsOk = false;

                                // check torrent-status
                                downloader->download("http://btjunkie.org" + urls.at(urlP), &data);
                                if (data.contains("This torrent has been verified by the community"))
                                {
                                    torrentIsOk = true;
                                }
                                else
                                {
                                    qDebug() << "@Reminder: torrent hasn't been verified! Searching for good votes" << urls.at(urlP);
                                    start = data.indexOf("Good(");
                                    end = data.indexOf(")", start)-1;
                                    QString str = data.mid(start, end-start+1);
                                    str.remove(0, 5);
                                    qDebug() << "@Reminder: " << str.toInt() << " votes for good!";
                                    if (str.toInt() > 0)
                                        torrentIsOk = true;
                                }

                                // download torrent if it's ok
                                if (torrentIsOk)
                                {
                                    end = data.indexOf(".torrent") + 7;
                                    for (int p=end; p>0; p--)
                                    {
                                        if (data.at(p) == '\"')
                                        {
                                            start = p+1;
                                            urlStr = data.mid(start, end-start+1);
                                            break;
                                        }
                                    }
                                    // open with torrent-program
                                    if (urlStr.contains(".torrent") && !QFile::exists(target))
                                    {
                                        downloader->download(urlStr, target);

                                        // open torrent with the standard system program
                                        QDesktopServices::openUrl(QUrl::fromLocalFile(target));
                                        alreadyDownloaded = true;
                                        break;
                                    }
                                }
                            }
                        } // end btjunkie.org

                        /* try isohunt.com if nothing was found on btjunkie */
                        if (!alreadyDownloaded && settings->isohunt == 1)
                        {
                            urlStr = ISOHUNTSEARCH_URL;
                            qDebug() << "@Reminder: searching torrent for " << naming << " on isohunt.com";
                            urlStr += naming;
                            urlStr = urlStr.replace(QChar(' '), QChar('+'));

                            // get torrent overview site urls
                            downloader->download(urlStr, &data);
                            start = end = 0;
                            urls.clear();
                            for (int p=0; p<data.count("\'/torrent_details")/2; p++)
                            {
                                start = data.indexOf("\'/torrent_details", end) + 1;
                                end = data.indexOf("\'", start) - 1;
                                QString str = data.mid(start, end-start+1);
                                if (str.contains("?tab=comments"))
                                {
                                    urls.append(str);
                                }
                            }

                            if (urls.size() == 0)
                                qDebug() << "@Reminder: not found";

                            for (int urlP=0; urlP<urls.size(); urlP++)
                            {
                                bool torrentIsOk = false;

                                // check torrent-status
                                downloader->download("http://isohunt.com" + urls.at(urlP), &data);
                                start = data.indexOf("user rating:") + 13;
                                end = data.indexOf(")", start)-1;
                                QString str = data.mid(start, end-start+1);
                                if (str.toInt() > 0)
                                {
                                    qDebug() << "@Reminder: " << str.toInt() << " positive votes!";
                                    torrentIsOk = true;
                                }
                                // download torrent if it's ok
                                if (torrentIsOk)
                                {
                                    end = data.indexOf(".torrent');") + 7;
                                    for (int p=end; p>0; p--)
                                    {
                                        if (data.at(p) == '\'')
                                        {
                                            start = p+1;
                                            urlStr = data.mid(start, end-start+1);
                                            urlStr = "http://isohunt.com" + urlStr;
                                            break;
                                        }
                                    }
                                    // open with torrent-program
                                    if (urlStr.contains(".torrent") && !QFile::exists(target))
                                    {
                                        downloader->download(urlStr, target);
                                        // open torrent with the standard system program
                                        QDesktopServices::openUrl(QUrl::fromLocalFile(target));
                                        alreadyDownloaded = true;
                                        break;
                                    }
                                }
                            }
                        } // end isohunt.com

                        /* try thepiratebay.org if nothing was found on isonhunt.com */
                        if (!alreadyDownloaded && settings->piratebay == 1)
                        {
                            urlStr = PIRATEBAYSEARCH_URL;
                            naming = record.show + "+S" +
                                    QString("%1").arg(record.season,2,10,QChar('0')) + "E" +
                                    QString("%1").arg(record.episode,2,10,QChar('0'));

                            qDebug() << "@Reminder: searching torrent for " << naming << " on thepiratebay.org";
                            urlStr += naming;
                            urlStr = urlStr.replace(QChar(' '), QChar('+'));
                            urlStr += "/0/7/0";                                 // sort result from highest to lowest seeder

                            // get torrent overview site urls
                            downloader->download(urlStr, &data);
                            start = end = 0;
                            for (int p=0; p<data.count("<a href=\"/torrent"); p++)
                            {
                                start = data.indexOf("<a href=\"/torrent", end) + 9;
                                end = data.indexOf("\"", start) - 1;
                                QString str = data.mid(start, end-start+1);
                                urls.append(str);
                            }

                            if (urls.size() == 0)
                                qDebug() << "@Reminder: not found";

                            for (int urlP=0; urlP<urls.size(); urlP++)
                            {
                                // check torrent-status
                                downloader->download("http://thepiratebay.org" + urls.at(urlP), &data);
                                // if some specific comments are made let's assume the torrent's real and healthy
                                if (data.contains("thanks", Qt::CaseInsensitive) || data.contains("thankx", Qt::CaseInsensitive) ||
                                    data.contains("thx", Qt::CaseInsensitive) || data.contains("great", Qt::CaseInsensitive) ||
                                    data.contains("deal", Qt::CaseInsensitive) || data.contains("real", Qt::CaseInsensitive) ||
                                    data.contains("love", Qt::CaseInsensitive))
                                {
                                    // download torrent
                                    end = data.indexOf(".torrent") + 7;
                                    for (int p=end; p>0; p--)
                                    {
                                        if (data.at(p) == '\"')
                                        {
                                            start = p+1;
                                            urlStr = data.mid(start, end-start+1);
                                            break;
                                        }
                                    }
                                }
                                // open with torrent-program
                                if (urlStr.contains(".torrent") && !QFile::exists(target))
                                {
                                    downloader->download(urlStr, target);

                                    // open torrent with the standard system program
                                    QDesktopServices::openUrl(QUrl::fromLocalFile(target));
                                    alreadyDownloaded = true;
                                    break;
                                }
                            }
                        } // end piratebay.org
                    }
                } // end of autoLoader

                /* remind the user if the episode is today or in the future */
                if(days <= recordDataList->at(dataPos).days && days >= 0)
                {
                    /* check if the user has been reminded already */
                    for(int checkPos=0; checkPos<remState.size(); checkPos++)
                    {
                        if(remState.at(checkPos).show == record.summary)
                        {
                            if(remState.at(checkPos).reminded)
                            {
                                alreadyReminded = true;
                            }
                        }
                    }

                    if(!alreadyReminded)
                    {
                        remindNecessary = true;
                        if(days == 0)   // today!
                        {
                            info = InfoLoader::getInfo(records->at(pos).show);

                            remindString += records->at(pos).show +
                                          ": S" + QString("%1").arg(records->at(pos).season,2,10,QChar('0')) +
                                          "E" + QString("%1").arg(records->at(pos).episode,2,10,QChar('0')) +
                                          " on " + info.tvstation + " " + tr("today") + "!\n";
                            remindStringHTML += "<b>" + records->at(pos).show + "</b>" +
                                          ": S" + QString("%1").arg(records->at(pos).season,2,10,QChar('0')) +
                                          "E" + QString("%1").arg(records->at(pos).episode,2,10,QChar('0')) +
                                          " on " + info.tvstation + " " + tr("today") + "!<br>";
                        }
                        else if(days == 1)   // tomorrow
                        {
                            remindString += records->at(pos).show +
                                          ": S" + QString("%1").arg(records->at(pos).season,2,10,QChar('0')) +
                                          "E" + QString("%1").arg(records->at(pos).episode,2,10,QChar('0')) +
                                          " " + tr("tomorrow") + "\n";
                            remindStringHTML += "<b>" + records->at(pos).show + "</b>" +
                                          ": S" + QString("%1").arg(records->at(pos).season,2,10,QChar('0')) +
                                          "E" + QString("%1").arg(records->at(pos).episode,2,10,QChar('0')) +
                                          " " + tr("tomorrow") + "<br>";
                        }
                        else
                        {
                            remindString += records->at(pos).show +
                                          ": S" + QString("%1").arg(records->at(pos).season,2,10,QChar('0')) +
                                          "E" + QString("%1").arg(records->at(pos).episode,2,10,QChar('0')) +
                                          " in "+tr("%n day(s)","",days)+"\n";

                            remindStringHTML += "<b>" + records->at(pos).show + "</b>" +
                                          ": S" + QString("%1").arg(records->at(pos).season,2,10,QChar('0')) +
                                          "E" + QString("%1").arg(records->at(pos).episode,2,10,QChar('0')) +
                                          " in "+tr("%n day(s)","",days)+"<br>";
                        }

                        /* append a reminder-element to the list */
                        tmp.show = records->at(pos).summary;
                        tmp.reminded = true;
                        remStateTmp.append(tmp);
                    }
                }
            }
        }
    }
    /* work off google-reminder-list */
    gReminderListIndex = 0;
    if (!gReminderList.isEmpty())
        gapi->clientLogin(settings->googleUsername, settings->googlePassword, QString("")+ORGANIZATION_NAME+"-"+APPLICATION_NAME+"-"+APPLICATION_VERSION, "cl");

    if(remindNecessary)
    {
        remindString.chop(1);
        qDebug() << "@Reminder: " << remindString;

        // trayIconMessage
        if (settings->ballonMessageReminder)
            tray->showMessage(tr("New Episodes"),remindString, QSystemTrayIcon::Information, 1000000);

        // messageBox
        if (settings->messageBoxReminder)
            msg->showMessage(tr("New Episodes"),remindStringHTML);
    }
}

void Reminder::eventExistent(bool exists)
{
    if (exists)
    {
        qDebug() << "event exists: " << gReminderList[gReminderListIndex].show  << gReminderList[gReminderListIndex].name;
        gReminderListIndex++;
        if (gReminderListIndex < gReminderList.size())
            gapi->eventExists(gReminderList[gReminderListIndex].show + QString(" S%1E%2").arg(gReminderList[gReminderListIndex].season,2,10,QChar('0'))
                          .arg(gReminderList[gReminderListIndex].episode,2,10,QChar('0')));
    }
    else
    {
        qDebug("event don't exists");
        QString data = InfoLoader::getInfo(gReminderList[gReminderListIndex].show).tvstation;
        QString tvStation = data.mid(0, data.indexOf("at") - 1).trimmed();
        int hour = data.section(" ", 2, 2).section(":", 0, 0).toInt();
        int minute = data.section(" ", 2, 2).section(":", 1, 1).toInt();
        if (data.section(" ", 3, 3) == "pm")
            hour += 12;

        QDateTime start(gReminderList[gReminderListIndex].start, QTime(hour, minute, 0, 0).addSecs(-2*60*60));
        QDateTime end(gReminderList[gReminderListIndex].start, start.time().addSecs(30*60));
        gapi->createCalendarEvent(gReminderList[gReminderListIndex].show + QString(" S%1E%2").arg(gReminderList[gReminderListIndex].season,2,10,QChar('0'))    // title
                                                                       .arg(gReminderList[gReminderListIndex].episode,2,10,QChar('0')),
                                  gReminderList[gReminderListIndex].name,                        // content
                                  tvStation,                                                     // location
                                  start,
                                  end,
                                  reminderType,
                                  reminderMinutes);

        qDebug() << "create event " << gReminderList[gReminderListIndex].show << gReminderList[gReminderListIndex].name;
    }
}

void Reminder::gapiReady(void)
{
    qDebug() << "created";
    gReminderListIndex++;
    if (gReminderListIndex < gReminderList.size())
        gapi->eventExists(gReminderList[gReminderListIndex].show + QString(" S%1E%2").arg(gReminderList[gReminderListIndex].season,2,10,QChar('0'))
                                                                       .arg(gReminderList[gReminderListIndex].episode,2,10,QChar('0')));
}

void Reminder::loggedIn(void)
{
    qDebug() << "@Reminder: (gapi) logged in!";
    // gReminderListIndex is ALWAYS 0 here!
    if (gReminderList.size() > 0)
        gapi->eventExists(gReminderList[gReminderListIndex].show + QString(" S%1E%2").arg(gReminderList[gReminderListIndex].season,2,10,QChar('0'))
                                                                       .arg(gReminderList[gReminderListIndex].episode,2,10,QChar('0')));
    else
        qDebug() << "list is empty!";
}

void Reminder::handleError(QString error)
{
    qDebug() << error;
}

void Reminder::messageAccepted(void)
{
    // append the temporary string to the real one
    remState.append(remStateTmp);
}
