QT += network
CONFIG += warn_on
TEMPLATE = app
DESTDIR = bin
TARGET = QTV
OBJECTS_DIR = obj
MOC_DIR = obj
UI_HEADERS_DIR = obj
INCLUDEPATH = src
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/downloader.cpp \
    src/icsparser.cpp \
    src/about.cpp \
    src/settings.cpp \
    src/showview.cpp \
    src/queue.cpp \
    src/reminder.cpp \
    src/viewabstract.cpp \
    src/rememberview.cpp \
    src/showstoday.cpp \
    src/msgbox.cpp \
    src/imageloader.cpp \
    src/infoloader.cpp \
    src/googleapi.cpp \
    src/detailsview.cpp
HEADERS += src/mainwindow.h \
    src/downloader.h \
    src/icsparser.h \
    src/about.h \
    src/settings.h \
    src/showview.h \
    src/queue.h \
    src/defines.h \
    src/reminder.h \
    src/viewabstract.h \
    src/rememberview.h \
    src/showstoday.h \
    src/msgbox.h \
    src/imageloader.h \
    src/infoloader.h \
    src/googleapi.h \
    src/detailsview.h
FORMS += src/uis/mainwindow.ui \
    src/uis/about.ui \
    src/uis/settings.ui \
    src/uis/queue.ui \
    src/uis/showstoday.ui \
    src/uis/msgbox.ui \
    src/uis/detailsview.ui
RESOURCES += src/resources.qrc
RC_FILE = src/qtv.rc
TRANSLATIONS = src/translations/qtv_de.ts \
    src/translations/qtv_fr.ts \
    src/translations/qtv_en.ts \
    src/translations/qtv_pt_BR.ts
OTHER_FILES += doc/ChangeLog.txt \
    doc/currentStable.txt \
    doc/currentUnstable.txt
