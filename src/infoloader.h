#ifndef INFOLOADER_H
#define INFOLOADER_H

#include "downloader.h"
#include "icsparser.h"

#include <QThread>
#include <QStringList>

class InfoLoader : public QThread
{
    Q_OBJECT
public:
    InfoLoader();

    void setParser(ICSParser *_parser) { parser=_parser; }
    static ShowInfo getInfo(QString show);
    void checkInfos();
    void setShows(QStringList shows) { this->shows = shows; }
    void setShowsToLoad(QStringList shows) { this->missingShows = shows; }
    void appendShowToLoad(QString show) { this->missingShows.append(show); }
    void appendShowsToLoad(QStringList shows) { this->missingShows.append(shows); }

public slots:
    void run();

signals:
    void setStatusBar(QString text);

private:
    Downloader *downloader;
    ICSParser *parser;
    QStringList missingShows;
    QStringList shows;

    static QString showNameToSaveName(QString show);
    int MonthToInt(QString month);
};

#endif // INFOLOADER_H
