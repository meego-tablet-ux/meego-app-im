TEMPLATE = lib
TARGET = telepathy-models
VERSION = 0.1.0

target.path = $$INSTALL_ROOT/usr/lib

SOURCES = abstract-conversation-model.cpp \
          accounts-model-item.cpp \
          accounts-model.cpp \
          avatar-image-provider.cpp \
          call-event-item.cpp \
          contact-model-item.cpp \
          custom-event-item.cpp \
          event-item.cpp \
          flat-model-proxy.cpp \
          session-conversation-model.cpp \
          text-event-item.cpp \
          tree-node.cpp

HEADERS = abstract-conversation-model.h \
          accounts-model-item.h \
          accounts-model.h \
          avatar-image-provider.h \
          call-event-item.h \
          contact-model-item.h \
          custom-event-item.h \
          event-item.h \
          flat-model-proxy.h \
          global.h \
          session-conversation-model.h \
          text-event-item.h \
          tree-node.h

QT += dbus #network
CONFIG += qt link_pkgconfig create_prl

OBJECTS_DIR = .obj
MOC_DIR = .moc

PKGCONFIG += TelepathyQt4 TelepathyQt4Yell
#LIBS += -ltelepathy-qt4yell-models -ltelepathy-qt4-yell-farstream -ltelepathy-logger-qt4-models -ltelepathy-logger-qt4

INSTALLS += target
