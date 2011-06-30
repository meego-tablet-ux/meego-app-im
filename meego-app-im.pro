TEMPLATE = subdirs 
SUBDIRS += telepathy-qml-lib telepathy-qml-plugin plugin panels-plugin approver tools
CONFIG += ordered dbus

qmlfiles.files += *.qml *.js settings
qmlfiles.path += $$INSTALL_ROOT/usr/share/$$TARGET

desktop.files += meego-app-im.desktop
desktop.path += $$INSTALL_ROOT/usr/share/applications

settingsdesktop.files += im-settings.desktop
settingsdesktop.path += $$INSTALL_ROOT/usr/share/meego-ux-settings/apps/

protocols.files += protocols
protocols.path += $$INSTALL_ROOT/usr/share/$$TARGET

INSTALLS += qmlfiles desktop settingsdesktop protocols

# dist stuff begins here
# Only following three lines needed to be editied
TRANSLATIONS += *.qml *.js *.desktop panels-plugin/*.h panels-plugin/*.cpp settings/*.qml
TRANSLATIONS += plugin/IM/*.qml plugin/*.h plugin/*.cpp plugin/IM/*.js
TRANSLATIONS += protocols/*qml protocols/*.desktop
TRANSLATIONS += panels-plugin/*.cpp panels-plugin/*.h
TRANSLATIONS += telepathy-qml-lib/*.h telepathy-qml-lib/*.cpp
TRANSLATIONS += telepathy-qml-plugin/*.h telepathy-qml-plugin/*.cpp
VERSION = 0.2.14.4
PROJECT_NAME = meego-app-im

dist.commands += rm -fR $${PROJECT_NAME}-$${VERSION} &&
dist.commands += git clone . $${PROJECT_NAME}-$${VERSION} &&
dist.commands += rm -fR $${PROJECT_NAME}-$${VERSION}/.git &&
dist.commands += mkdir -p $${PROJECT_NAME}-$${VERSION}/ts &&
dist.commands += lupdate $${TRANSLATIONS} -ts $${PROJECT_NAME}-$${VERSION}/ts/$${PROJECT_NAME}.ts &&
dist.commands += tar jcpvf $${PROJECT_NAME}-$${VERSION}.tar.bz2 $${PROJECT_NAME}-$${VERSION}
QMAKE_EXTRA_TARGETS += dist
