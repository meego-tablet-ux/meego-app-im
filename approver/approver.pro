######################################################################
# Automatically generated by qmake (2.01a) Thu Jun 2 14:22:31 2011
######################################################################

TEMPLATE = app
TARGET = meego-app-im-approver
DEPENDPATH += . ../telepathy-qml-lib
INCLUDEPATH += .

#QT -= gui
QT += network dbus
CONFIG += qt link_pkgconfig

# Input
HEADERS += imchannelapprover.h imapproveradaptor_p.h
SOURCES += imchannelapprover.cpp imapproveradaptor.cpp main.cpp

LIBS += -L../telepathy-qml-lib -ltelepathy-qml
PKGCONFIG += TelepathyQt4 TelepathyQt4Yell mlite glib-2.0

target.path = $$INSTALL_ROOT/usr/bin
target.files = $$TARGET

clientfiles.files = *.client
clientfiles.path = $$INSTALL_ROOT/usr/share/telepathy/clients/

services.path = $$INSTALL_ROOT/usr/share/dbus-1/services
services.files += *.service

INSTALLS += target clientfiles services
