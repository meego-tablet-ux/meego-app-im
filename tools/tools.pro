TEMPLATE = subdirs
SUBDIRS = 

tools.files += meego-app-im-dumplogs
tools.path += $$INSTALL_ROOT/usr/bin

QMAKE_STRIP = echo 

INSTALLS += tools
