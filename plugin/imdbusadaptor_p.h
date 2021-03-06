/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -a imdbusadaptor_p.h:imdbusadaptor.cpp -c IMDBusAdaptor im.xml
 *
 * qdbusxml2cpp is Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef IMDBUSADAPTOR_P_H_1310042484
#define IMDBUSADAPTOR_P_H_1310042484

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface com.meego.app.im
 */
class IMDBusAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.meego.app.im")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.meego.app.im\">\n"
"    <signal name=\"chatOpenRequested\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"accountId\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"contactId\"/>\n"
"    </signal>\n"
"    <method name=\"showChat\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"accountId\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"contactId\"/>\n"
"    </method>\n"
"    <method name=\"showGroupChat\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"accountId\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"groupChatId\"/>\n"
"    </method>\n"
"    <method name=\"reportMissedAudioCalls\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"accountId\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"contacts\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"times\"/>\n"
"    </method>\n"
"    <method name=\"reportMissedVideoCalls\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"accountId\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"contacts\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"times\"/>\n"
"    </method>\n"
"    <method name=\"userBusyCalling\">\n"
"      <arg direction=\"out\" type=\"b\" name=\"busy\"/>\n"
"    </method>\n"
"    <method name=\"dumpLogs\"/>\n"
"  </interface>\n"
        "")
public:
    IMDBusAdaptor(QObject *parent);
    virtual ~IMDBusAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void dumpLogs();
    void reportMissedAudioCalls(const QString &accountId, const QStringList &contacts, const QStringList &times);
    void reportMissedVideoCalls(const QString &accountId, const QStringList &contacts, const QStringList &times);
    void showChat(const QString &accountId, const QString &contactId);
    void showGroupChat(const QString &accountId, const QString &groupChatId);
    bool userBusyCalling();
Q_SIGNALS: // SIGNALS
    void chatOpenRequested(const QString &accountId, const QString &contactId);
};

#endif
