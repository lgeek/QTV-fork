#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include "downloader.h"
#include "icsparser.h"

#include <QObject>
#include <QThread>
#include <QStringList>

struct ImageUrl
{
    QString show;
    QString image;
};

class ImageLoader : public QThread
{
    Q_OBJECT
public:
    ImageLoader();

    void setParser(ICSParser *_parser) { parser=_parser; }
    static QPixmap getImage(QString show);
    void checkImages();
    void setShows(QStringList shows) { this->shows = shows; }
    void setShowsToLoad(QStringList shows) { this->missingShows = shows; }
    void appendShowToLoad(QString show) { this->missingShows.append(show); }
    void appendShowsToLoad(QStringList shows) { this->missingShows.append(shows); }

public slots:
    void run();
    void loadAllShows() { this->missingShows = this->shows; }

signals:
    void setStatusBar(QString text);

private:
    Downloader *downloader;
    ICSParser *parser;
    QStringList missingShows;
    QStringList shows;

    static QString showNameToSaveName(QString show);
};

#endif // IMAGELOADER_H
