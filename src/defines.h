#ifndef DEFINES_H
#define DEFINES_H

#include <QObject>
#include <QDate>
#include <QString>
#include <QDebug>

/* APPLICATION-DEFINES */
#define APPLICATION_NAME    "QTV"
#define APPLICATION_VERSION "0.3"   // only float values for version-checking here!
#define ORGANIZATION_DOMAIN  "http://code.google.com/p/qtv/"
#define ORGANIZATION_NAME    "-"

/* used files */
#define LOGFILE             "./data/logs.log"
#define ICSFILE             "./data/calendar.ics"
#define DATAFILE            "./data/data.bin"
#define SETTINGSFILE        "./data/settings.bin"
#define IMAGEDIR            "data/images"
#define INFODIR             "data/infos"
#define TORRENTDIR          "data/torrents"
#define SEARCH_URL          "http://torrent-finder.com/show.php?q="
#define SEARCH_ALT_URL      "http://serienjunkies.org/?s="
#define TVCOM_SEARCH_URL    "http://www.tv.com/search.php?stype=ajax_search&search_type=episode&qs="
#define GOOGLE_SEARCH_URL   "http://images.google.at/images?q="
#define FULLDLSSEARCH_URL   "http://www.fulldls.com/search-all-torrents/?qa="
#define BTJUNKIESEARCH_URL  "http://btjunkie.org/search?q="
#define ISOHUNTSEARCH_URL   "http://isohunt.com/torrents/?ihq="
#define PIRATEBAYSEARCH_URL "http://thepiratebay.org/search/"

/* default-stuff */
#define ICAL_URL            "http://next-episode.net/PAGES/misc/export_calendar?z.ics"
#define DEFAULT_DAYS        3
#define FAILURE_RETRIES     5
#define RETRY_DELAY         15000
#define DEFAULT_TRANSLATION "qtv_en"

/* default-settings */
#define DEFAULT_INFORMUPDATE            0
#define DEFAULT_MINUTES                 10
#define DEFAULT_PROXY                   ""
#define DEFAULT_PORT                    8080
#define DEFAULT_USERNAME                ""
#define DEFAULT_PASSWORD                ""
#define DEFAULT_SECONDS                 0
#define DEFAULT_USEPROXY                0
#define DEFAULT_INFORMSTABLE            1
#define DEFAULT_INFORMUNSTABLE          0
#define DEFAULT_BALLOONMESSAGEREMINDER  1
#define DEFAULT_MESSAGEBOXREMINDER      0
#define DEFAULT_AUTODOWNLOAD            0
#define DEFAULT_AUTOSTART               0
#define DEFAULT_LOADAFTERDAYS           3

/* web-links for version-checking */
#define URLSTABLE   "http://qtv.googlecode.com/svn/trunk/QTV/doc/currentStable.txt"
#define URLUNSTABLE "http://qtv.googlecode.com/svn/trunk/QTV/doc/currentUnstable.txt"

struct RecordData
{
    QString show;
    qint16 days;
};

struct SettingsData
{
    // application
    qint16 enableAutoStart;
    qint16 informStable;
    qint16 informUnstable;

    // reminder
    qint16 minutes;
    qint16 autoDownload;
    qint16 loadAfterDays;
    qint16 btjunkie;
    qint16 isohunt;
    qint16 piratebay;
    qint16 ballonMessageReminder;
    qint16 messageBoxReminder;

    // network
    qint16 useProxy;
    QString proxyHost, proxyUsername, proxyPassword;
    qint16 proxyPort;

    // google
    QString googleUsername;
    QString googlePassword;
    qint16 enableGReminder;
    qint16 googleReminderType;
    qint16 googleReminderUnit;
    qint16 googleReminderTime;
};

struct ICSData
{
    QDate start, end;
    QString summary, description, url;
    QString name; // NOT the name of the show; the name of the episode!

    /* the following objects will be read out of "summary" */
    QString show;
    int season;
    int episode;
};

struct ShowInfo
{
    QString summary;
    QString genre;
    QString tvstation;
    QString similar;

    ICSData next;
    ICSData previous;
};

#endif // DEFINES_H
