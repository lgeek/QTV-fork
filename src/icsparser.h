#ifndef ICSPARSER_H
#define ICSPARSER_H

#include "defines.h"
#include "downloader.h"

#define MAX_RECORDS 1024

class QFile;

class ICSParser : public QObject
{
    Q_OBJECT

public:
    ICSParser(QString path);
    ~ICSParser(void);

    QList<ICSData> *getRecords(void) { return &records; }
    QString getShowImage(const QString show);

    void append(ICSData record) { records.append(record); }
    int size(void)  { return records.size(); }
    ICSData at(int number) { return records.at(number); }
    bool contains(ICSData record);

    static bool ICSDataLessThan(const ICSData& d1, const ICSData& d2);
    static QString upperAfter(QString str, QChar ch);
    static QString upperSomeChars(QString str);
    static QString htmlToUnicode(QString str);

public slots:
    void parse(void);      

private:
    QString path;
    QList<ICSData> records;
    Downloader *downloader;     // just for loading the top-tv-shows!

signals:
    void finished();
};

#endif // ICSPARSER_H
