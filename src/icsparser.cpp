#include "icsparser.h"

#include <QFile>
#include <stdio.h>

// Typical entry for one episode in the ics-file
/*
    BEGIN:VEVENT
    DTSTART:20090503
    DTEND:20090503
    SUMMARY:American Dad - 4x18
    DESCRIPTION:American Dad - Weiner of Our Discontent - Season 4, Episode 18
    URL:http://next-episode.net/american-dad
    UID:americandad_4_18
    SEQUENCE:0
    DTSTAMP:20090516T093323Z
    TRANSP:TRANSPARENT
    CATEGORIES:American Dad Episodes, TV Shows
    END:VEVENT
*/

ICSParser::ICSParser(QString _path)
{
    path = _path;

    downloader = new Downloader();
}

ICSParser::~ICSParser(void)
{
}

void ICSParser::parse(void)
{
    int recPos = -1;
    QByteArray rawData;
    QString string;
    int y, m, d, offset;
    ICSData tmp, tmp2;

    QFile file(path);
    records.clear();

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "@ICSParser::parse: parsing-file couldn't be opened";
        return;
    }

    for (;!file.atEnd();)
    {
        rawData = file.readLine();

        // dirty hack. normally we've to decode unicode which seems to doesn't work
        rawData = htmlToUnicode(QString(rawData)).toLocal8Bit();

        //rawData.replace("&#039;", "'"); // Apostroph
        //rawData.replace("&amp;", "&");  // &
        //rawData.replace("&#133", "\""); // "


        if (rawData.contains("&#"))
            qDebug() << "@ICSParser::parse: found some unicode-chars: " << rawData;

        // make some characters uppercase after special characters
        rawData = upperSomeChars(rawData).toAscii();

        if (rawData == "BEGIN:VEVENT\n")   // a new record starts!
        {
            recPos++;
        }
        else if (rawData.indexOf("DTSTART:") != -1 && recPos>-1)
        {
            string = QString(rawData.right(rawData.length() - QString("DTSTART:").length()));
            d = string.right(3).toInt();                        // the last char is '\n'
            string.chop(3);
            m = string.right(2).toInt();
            string.chop(2);
            y = string.toInt();
            tmp.start.setDate(y, m, d);
        }
        else if (rawData.indexOf("DTEND:") != -1 && recPos>-1)
        {
            string = QString(rawData.right(rawData.length() - QString("DTSTART:").length()));
            d = string.right(3).toInt();                        // the last char is '\n'
            string.chop(3);
            m = string.right(2).toInt();
            string.chop(2);
            y = string.toInt();
            tmp.end.setDate(y, m, d);
        }
        else if (rawData.indexOf("SUMMARY:") != -1 && recPos>-1)
        {
            tmp.summary = QString(rawData.right(rawData.length() - QString("SUMMARY:").length()));
            tmp.summary.chop(1);
            offset = tmp.summary.count('-') - 1;
            tmp.show = tmp.summary.left(tmp.summary.lastIndexOf('-'));
            tmp.show.chop(1);                                           // get rid of the following space
            tmp.season = tmp.summary.section('-',1+offset,1+offset).section('x',0,0).toInt();
            tmp.episode = tmp.summary.section('-',1+offset,1+offset).section('x',1,1).toInt();
        }
        else if (rawData.indexOf("DESCRIPTION:") != -1 && recPos>-1)
        {
            tmp.description = QString(rawData.right(rawData.length() - QString("DESCRIPTION:").length()));
            tmp.description.chop(1);
            // read the episodes name
            tmp.name = tmp.description.right(tmp.description.length() - tmp.show.length() - 3);
            tmp.name.chop(tmp.name.length() - tmp.name.indexOf("Season"));
            tmp.name.chop(3);
        }
        else if (rawData.indexOf("URL:") != -1 && recPos>-1)
        {
            tmp.url = QString(rawData.right(rawData.length() - QString("URL:").length()));
            tmp.url.chop(1);
        }
        else if(rawData.indexOf("END:VEVENT") != -1)
        {
            records.append(tmp);
        }

    }
    file.close();

    /* load top-tv-shows */
    int start, end;
    QString data;
    downloader->download(QUrl("http://next-episode.net/"), &data);
    if (data.contains("Top TV Shows:"))
    {
        start = data.indexOf("Top TV Shows:");
        end = data.indexOf("</a><br><br><br>", start);
        data = data.mid(start, end-start);

        data.replace("<a>","");
        data.replace("</a>","");
        data.replace("<br>","");
        data.replace("</br>","");
        data.replace("<b>","");
        data.replace("</b>","");
        data.replace("<a href=","");
        data.replace("<","");
        data.replace(">","");
        data.replace(" style=\"color: #666666\"","");
        data.replace(" rel=\"nofollow\"","");
        data.replace("Top TV Shows:","");

        for(int pos=1; pos<=data.count("\""); pos+=2)
        {            
            tmp2.url = data.section("\"", pos, pos);
            tmp2.show = data.section("\"", pos+1, pos+1);

            // make some characters uppercase after special characters
            tmp2.show = upperSomeChars(tmp2.show).toAscii();

            tmp2.description = tmp2.name = tmp2.summary = "";
            tmp2.episode = tmp2.season = 0;

            records.append(tmp2);
        }
    }
    /* sort parsed data chronological with the quicksort algorithm provided by Qt */
    /* consider seasons and episodes                                              */
    qSort(records.begin(),records.end(),ICSDataLessThan);

    emit(finished());
}

QString ICSParser::upperAfter(QString str, QChar ch)
{
    if (str.contains(ch))
    {
        int pos = 0;
        for (int i=0; i<str.count(ch); i++)
        {
            pos = str.indexOf(ch, pos) + 1;
            if (pos < str.length())
                str[pos] = str.at(pos).toUpper();
        }
    }
    return str;
}

QString ICSParser::upperSomeChars(QString str)
{
    QString ret;

    // try to make some characters uppercase after special characters
    ret = upperAfter(QString(str), ' ').toAscii();
    ret = upperAfter(QString(ret), '.').toAscii();
    ret = upperAfter(QString(ret), '/').toAscii();
    ret = upperAfter(QString(ret), '(').toAscii();
    ret = upperAfter(QString(ret), '-').toAscii();

    return ret;
}

bool RecordEquals(ICSData record1, ICSData record2)
{
    QString r1 = record1.show + record1.season + record1.episode;
    QString r2 = record2.show + record2.season + record2.episode;

    if (r1 == r2)
        return true;
    else
        return false;
}

bool ICSParser::contains(ICSData record)
{
    for (int pos=0; pos<records.size(); pos++)
    {
        if (RecordEquals(records.at(pos), record))
        {
            return true;
        }
    }

    return false;
}

bool ICSParser::ICSDataLessThan(const ICSData& d1, const ICSData& d2)
{
  if(d1.start < d2.start)
    return true;

  if((d1.start == d2.start) && (d1.season < d2.season))  //consider seasons
    return true;

  if((d1.start == d2.start) && (d1.season == d2.season) && (d1.episode < d2.episode))  //consider episodes
    return true;

  return false;
}

QString ICSParser::htmlToUnicode(QString str)
{
    QString src, tgt, srcVal;
    int start=0, end=0;
    int value;

    for (int pos=0; pos<str.count("&#"); pos++)
    {
        start = str.indexOf("&#", start+1);
        end = str.indexOf(";", start);
        src = str.mid(start, end-start+1);
        srcVal = src;
        if (src[src.length()-1] == ';')
            srcVal.chop(1);

        value = srcVal.section("#", 1, 1).toInt();

        str.replace(src, QChar(value));
    }
    return str;
}
