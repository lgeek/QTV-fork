#include "infoloader.h"

#include <QFile>
#include <QDir>
#include <QCoreApplication>

InfoLoader::InfoLoader()
{
}

void InfoLoader::run()
{
    int pos, amount = shows.size();
    QString url, savePath;
    downloader = new Downloader();
    QDir dir;
    QFile file;
    ICSData record;
    ShowInfo tmp;

    qDebug() << "#InfoLoader::run: START";

    /* create the image-directory if it's not already there */
    if (!dir.exists(QCoreApplication::applicationDirPath() + "/" + INFODIR))
    {
        dir.mkdir(QCoreApplication::applicationDirPath() + "/" + INFODIR);
    }

    /* search for the show */
    for (pos=0; pos<amount; pos++)
    {
        emit(setStatusBar(tr("Downloading info for ") + shows.at(pos)));
        if (!missingShows.contains(shows.at(pos)))
        {
            continue;
        }
        savePath = QCoreApplication::applicationDirPath() + "/" + INFODIR + "/" + showNameToSaveName(shows.at(pos)) + ".dat";
        file.setFileName(savePath);


            QString showPage;

            for (int p=0; p<parser->size(); p++)
            {
                record = parser->at(p);
                if (record.show == shows.at(pos))
                {
                    qDebug() << "#InfoLoader::run: " << record.url;
                    downloader->download(record.url, &showPage);
                    break;
                }
            }

            /* parse page */
            /* summary */
            if (showPage.contains("more info"))
            {
                int start=0, end=0;

                end = showPage.indexOf("more info") + 13;
                for (int pos=end; pos>0; pos--)
                {
                    if (showPage.at(pos) == '\n' && showPage.at(pos-1) == '>')
                    {
                        start = pos+1;
                        break;
                    }
                }
                tmp.summary = showPage.mid(start, end-start+1);
                tmp.summary.replace("“","\"");
                tmp.summary.replace("”","\"");
                tmp.summary.replace("’","'");
                tmp.summary.replace("–","-");
                tmp.summary.replace("…", "...");
                tmp.summary = tmp.summary.trimmed();
            }
            else
            {
                qDebug() << "#InfoLoader::run: no info on page!";
            }

            /* genre */
            if (showPage.contains("<b>Genre:"))
            {
                int start = showPage.indexOf("<b>Genre:");
                int end = showPage.indexOf("</a>", start) + 3;

                tmp.genre = showPage.mid(start, end-start+1).trimmed();
                tmp.genre = tmp.genre.right(tmp.genre.length() - 14);
            }

            /* tvstation */
            if (showPage.contains("airs on:</b>"))
            {
                int start = showPage.indexOf("airs on:</b>") + 12;
                int end = showPage.indexOf("</span>", start) - 1;

                tmp.tvstation = showPage.mid(start, end-start+1).trimmed();
            }

            /* similar shows */
            if (showPage.contains("You may also like:"))
            {
                int start = showPage.indexOf("You may also like:") + 18;
                int end = showPage.indexOf("<br>", start) - 1;

                tmp.similar = showPage.mid(start, end-start+1).trimmed();
                tmp.similar.replace("</b>", "");
                tmp.similar.replace("&nbsp;", "");

                if (tmp.similar.contains("No suggestions yet"))
                    tmp.similar.clear();
            }

            /* next episode */
            tmp.next.name.clear();
            tmp.previous.name.clear();
            if (showPage.contains("Next episode:"))
            {
                if (showPage.contains("sorry", Qt::CaseInsensitive) || showPage.contains("available", Qt::CaseInsensitive))
                {
                }
                else
                {
                    tmp.next.show = record.show;
                    // name
                    int start = showPage.indexOf("Next episode:");
                    start = showPage.indexOf("rel=\"nofollow\">", start) + 15;
                    int end = showPage.indexOf("</a>", start) - 1;
                    tmp.next.name = showPage.mid(start, end-start+1);
                    // upper some chars
                    tmp.next.name = ICSParser::upperSomeChars(tmp.next.name);

                    // date
                    start = showPage.indexOf("Date:", start);
                    start = showPage.indexOf("width=\"300\">", start) + 12;
                    end = showPage.indexOf("</td>", start) - 1;
                    QString data = showPage.mid(start, end-start+1);
                    int year = data.section(" ", 3, 3).toInt();
                    int month = MonthToInt(data.section(" ", 1, 1));
                    data = data.section(" ", 2, 2);
                    data.chop(1);
                    int day = data.toInt();
                    tmp.next.start.setDate(year, month, day);

                    // season
                    start = showPage.indexOf("Season:", start);
                    start = showPage.indexOf("width=\"300\">", start) + 12;
                    end = showPage.indexOf("</td>", start) - 1;
                    tmp.next.season = showPage.mid(start, end-start+1).section(" ", 0, 0).trimmed().toInt();

                    //episode
                    start = showPage.indexOf("Number:", start);
                    start = showPage.indexOf("width=\"300\">", start) + 12;
                    end = showPage.indexOf("</td>", start) - 1;
                    QString episode = showPage.mid(start, end-start+1).section(" ", 0, 0).trimmed();
                    if (episode.endsWith(QChar(',')))
                        episode.chop(1);
                    tmp.next.episode = episode.toInt();
                }
            }

            /* previous episode */
            if (showPage.contains("Previous episode:"))
            {
                tmp.previous.show = record.show;
                // name
                int start = showPage.indexOf("Previous episode:");
                start = showPage.indexOf("rel=\"nofollow\">", start) + 15;
                int end = showPage.indexOf("</a>", start) - 1;
                tmp.previous.name = showPage.mid(start, end-start+1);
                // upper some chars
                tmp.previous.name = ICSParser::upperSomeChars(tmp.previous.name);

                // date
                start = showPage.indexOf("Date:", start);
                start = showPage.indexOf("width=\"300\">", start) + 12;
                end = showPage.indexOf("</td>", start) - 1;
                QString data = showPage.mid(start, end-start+1);
                int year = data.section(" ", 3, 3).toInt();
                int month = MonthToInt(data.section(" ", 1, 1));
                data = data.section(" ", 2, 2);
                data.chop(1);
                int day = data.toInt();
                tmp.previous.start.setDate(year, month, day);

                // season
                start = showPage.indexOf("Season:", start);
                start = showPage.indexOf("width=\"300\">", start) + 12;
                end = showPage.indexOf("</td>", start) - 1;
                tmp.previous.season = showPage.mid(start, end-start+1).section(" ", 0, 0).trimmed().toInt();

                //episode
                start = showPage.indexOf("Number:", start);
                start = showPage.indexOf("width=\"300\">", start) + 12;
                end = showPage.indexOf("</td>", start) - 1;
                QString episode = showPage.mid(start, end-start+1).section(" ", 0, 0).trimmed();
                if (episode.endsWith(QChar(',')))
                    episode.chop(1);
                tmp.previous.episode = episode.toInt();
            }


            /* save info struct to file */
            if (file.open(QIODevice::WriteOnly))
            {
                QDataStream out(&file);    // write the data serialized to the file

                out << tmp.genre;
                out << tmp.similar;
                out << tmp.summary;
                out << tmp.tvstation;
                out << tmp.next.show;
                out << tmp.next.name;
                out << tmp.next.episode;
                out << tmp.next.season;
                out << tmp.next.start;
                out << tmp.previous.show;
                out << tmp.previous.name;
                out << tmp.previous.episode;
                out << tmp.previous.season;
                out << tmp.previous.start;
                file.close();
            }
            else
            {
                qDebug() << "#InfoLoader::run: error opening file!";
            }
    }
    emit(setStatusBar(tr("Ready")));
    qDebug() << "#InfoLoader::run: FINISH";
    missingShows.clear();
}

ShowInfo InfoLoader::getInfo(QString show)
{
    QString loadPath = QCoreApplication::applicationDirPath() + "/" + INFODIR + "/" + showNameToSaveName(show) + ".dat";
    QFile file(loadPath);
    ShowInfo tmp;

    if (QFile::exists(loadPath))
    {
        if (file.open(QIODevice::ReadOnly))
        {
            QDataStream in(&file);

            in >> tmp.genre;
            in >> tmp.similar;
            in >> tmp.summary;
            in >> tmp.tvstation;
            in >> tmp.next.show;
            in >> tmp.next.name;
            in >> tmp.next.episode;
            in >> tmp.next.season;
            in >> tmp.next.start;
            in >> tmp.previous.show;
            in >> tmp.previous.name;
            in >> tmp.previous.episode;
            in >> tmp.previous.season;
            in >> tmp.previous.start;

            file.close();
        }
        else    // delete file if it can't be opened
        {
            QFile::remove(loadPath);
        }
    }
    return tmp;
}

void InfoLoader::checkInfos()
{
    qDebug() << "#InfoLoader::run: checking for missing info-files";

    for(int pos=0; pos<shows.size(); pos++)
    {
        QString loadPath = QCoreApplication::applicationDirPath() + "/" + INFODIR + "/" + showNameToSaveName(shows.at(pos)) + ".dat";

        // delete the file if it's not loadable!
        if (!QFile::exists(loadPath))
        {
            missingShows.append(shows.at(pos));
        }
    }
}

QString InfoLoader::showNameToSaveName(QString show)
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

int InfoLoader::MonthToInt(QString month)
{
    int ret;

    if (month == "Jan")
        ret = 1;
    else if(month == "Feb")
        ret = 2;
    else if(month == "Mar")
        ret = 3;
    else if(month == "Apr")
        ret = 4;
    else if(month == "May")
        ret = 5;
    else if(month == "Jun")
        ret = 6;
    else if(month == "Jul")
        ret = 7;
    else if(month == "Aug")
        ret = 8;
    else if(month == "Sep")
        ret = 9;
    else if(month == "Oct")
        ret = 10;
    else if(month == "Nov")
        ret = 11;
    else if(month == "Dec")
        ret = 12;
    else
        ret = 0;

    return ret;
}
