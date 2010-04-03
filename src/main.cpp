#include <QtGui/QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDateTime>
#include "mainwindow.h"

void handleMessages(QtMsgType type, const char *msg);

int main(int argc, char *argv[])
{
    qInstallMsgHandler(handleMessages); // use own message-handler
    QApplication app(argc, argv);

    /* load the correspondending translation or at least the default one if possible */
    QString locale = QLocale::system().name();
    QTranslator translator;

    if(!translator.load(QCoreApplication::applicationDirPath() + "/qtv_" + locale))
        translator.load(QCoreApplication::applicationDirPath() + "/" + DEFAULT_TRANSLATION);
    app.installTranslator(&translator);

    MainWindow w;
    return app.exec();
}

void handleMessages(QtMsgType type, const char *msg)
{
    Q_UNUSED(type)
    FILE *f;

    fprintf(stderr, "%s\n", msg);

    // write to logFile
    f = fopen(qPrintable(QCoreApplication::applicationDirPath() + "/" + LOGFILE), "a");
    if (f != NULL)
    {
        fprintf(f, "%s: %s\n", qPrintable(QDateTime::currentDateTime().toString("hh:mm:ss")) , msg);
        fclose(f);
    }
}
