#include "imageloader.h"
#include "infoloader.h"

#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QPixmap>

ImageLoader::ImageLoader()
{
}

void ImageLoader::run()
{
    int pos, amount = shows.size();
    QString url, savePath;
    downloader = new Downloader();
    QDir dir;
    ICSData record;

    qDebug() << "+ImageLoader::run: START";

    this->checkImages();

    /* create the image-directory if it's not already there */
    if (!dir.exists(QCoreApplication::applicationDirPath() + "/" + IMAGEDIR))
    {
        dir.mkdir(QCoreApplication::applicationDirPath() + "/" + IMAGEDIR);
    }

    /* search for the show */
    for (pos=0; pos<amount; pos++)
    {
        emit(setStatusBar(tr("Downloading image for ") + shows.at(pos)));
        if (!missingShows.contains(shows.at(pos)))
        {
            continue;
        }
        savePath = QCoreApplication::applicationDirPath() + "/" + IMAGEDIR + "/" + showNameToSaveName(shows.at(pos)) + ".jpg";

        QString showPage;

        for (int p=0; p<parser->size(); p++)
        {
            record = parser->at(p);
            if (record.show == shows.at(pos))
            {
                qDebug() << "+ImageLoader::run: " << record.url << " to: " << savePath;
                downloader->download(record.url, &showPage);
                break;
            }
        }

        // NEXT-EPISODE.COM
        bool gotImage = false;
        if (showPage.contains("thumb"))
        {
            int start = showPage.indexOf("thumb") - 41;
            int end = showPage.indexOf("\"", start) - 1;
            url = showPage.mid(start, end-start+1);
            if (url.contains("jpg",Qt::CaseInsensitive))
            {
                qDebug() << "+ImageLoader::run: " << url;
                gotImage = true;
                downloader->download(url, savePath);
            }
        }
        if (!gotImage)
        {
            int start=0, end=0;
            qDebug() << "+ImageLoader::run: no suitable image on page! searching with google";

            // GOOGLE first hit, jpg, face, medium
            url = record.show;
            url.replace(" ", "+");
            downloader->download(GOOGLE_SEARCH_URL + url, &showPage);
            end = showPage.indexOf(".jpg") + 3;
            start = showPage.lastIndexOf("http:", end);
            url = showPage.mid(start, end-start+1);
            qDebug() << "+ImageLoader::run: " << url << " to: " << savePath;
            downloader->download(url, savePath);
        }
    }
    emit(setStatusBar(tr("Ready")));
    qDebug() << "+imageLoader FINISH";
    missingShows.clear();
}

QPixmap ImageLoader::getImage(QString show)
{
    QString loadPath = QCoreApplication::applicationDirPath() + "/" + IMAGEDIR + "/" + showNameToSaveName(show) + ".jpg";
    QPixmap pixmap;

    // delete the file if it's not loadable!
    if (!pixmap.load(loadPath))
    {
        QFile::remove(loadPath);
    }
    else
    {
        // resize image
        if (pixmap.width() > 120)
            pixmap = pixmap.scaledToWidth(120);
        if (pixmap.height() > 120)
            pixmap = pixmap.scaledToHeight(120);

        return pixmap;
    }
    return NULL;
}

void ImageLoader::checkImages(void)
{
    qDebug() << "+ImageLoader::run: checking for missing images";

    for(int pos=0; pos<shows.size(); pos++)
    {
        QString loadPath = QCoreApplication::applicationDirPath() + "/" + IMAGEDIR + "/" + showNameToSaveName(shows.at(pos)) + ".jpg";

        // delete the file if it's not loadable!
        if (!QFile::exists(loadPath))
        {
            missingShows.append(shows.at(pos));
        }
    }
}

QString ImageLoader::showNameToSaveName(QString show)
{
    QString name = show;

    name.replace(" ", "_");
    name.replace(":", "_");
    name.replace("&", "_");
    name.replace("(", "");
    name.replace(")", "");
    name.replace("!", "");
    name.replace("?", "");
    name.replace("'", "");

    return name.toLower();
}
