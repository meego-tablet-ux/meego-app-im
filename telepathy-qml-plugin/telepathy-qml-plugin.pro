TEMPLATE = lib
TARGET = TelepathyQML
QT += declarative dbus
CONFIG += qt plugin link_pkgconfig mobility
PKGCONFIG += TelepathyQt4 TelepathyQt4Yell gstreamer-0.10 gstreamer-interfaces-0.10 libresourceqt1
LIBS += -L../telepathy-models -ltelepathy-models -L../telepathy-qml-lib -ltelepathy-qml
TARGET = $$qtLibraryTarget($$TARGET)
DESTDIR = $$TARGET
OBJECTS_DIR = .obj
MOC_DIR = .moc
INCLUDEPATH += ../telepathy-qml-lib ../telepathy-models
MOBILITY += multimedia

plugin.files += $$TARGET
plugin.path += $$[QT_INSTALL_IMPORTS]/
INSTALLS += plugin

SOURCES += components.cpp
HEADERS += components.h
