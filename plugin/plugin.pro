TEMPLATE = lib
TARGET = IM
QT += declarative dbus network
CONFIG += qt plugin link_pkgconfig mobility
PKGCONFIG += TelepathyQt4 TelepathyQt4Yell TelepathyQt4YellFarstream qt-gst-qml-sink \
             QtGLib-2.0 glib-2.0 TelepathyLoggerQt4 contextsubscriber-1.0 mlite libresourceqt1 \
             icu-i18n icu-uc meegolocale
LIBS += -ltelepathy-qt4-yell-models -L../telepathy-qml-lib -ltelepathy-qml -ltelepathy-qt4-yell-farstream -ltelepathy-logger-qt4
INCLUDEPATH += /usr/include/mlite
TARGET = $$qtLibraryTarget($$TARGET)
DESTDIR = $$TARGET
OBJECTS_DIR = .obj
MOC_DIR = .moc
MOBILITY = multimedia

plugin.files += $$TARGET
plugin.path += $$[QT_INSTALL_IMPORTS]/MeeGo/App/

service.path = /usr/share/dbus-1/services
service.files += com.meego.app.im.service

INSTALLS += plugin service

SOURCES += components.cpp \
    accounthelper.cpp \
    contactssortfilterproxymodel.cpp \
    contactssortproxymodel.cpp \
    accountsmodelfactory.cpp \
    imaccountsmodel.cpp \
    imavatarimageprovider.cpp \
    accountssortfilterproxymodel.cpp \
    imgroupchatmodelitem.cpp \
    imgroupchatmodel.cpp \
    textfile.cpp \
    imdbus.cpp \
    imdbusadaptor.cpp

HEADERS += components.h \
    accounthelper.h \
    contactssortfilterproxymodel.h \
    contactssortproxymodel.h \
    accountsmodelfactory.h \
    imaccountsmodel.h \
    imavatarimageprovider.h \
    accountssortfilterproxymodel.h \
    imgroupchatmodelitem.h \
    imgroupchatmodel.h \
    textfile.h \
    imdbus.h \
    imdbusadaptor_p.h
