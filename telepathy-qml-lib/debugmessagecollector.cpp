/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "debugmessagecollector.h"

#include "telepathymanager.h"

#include <QtDBus>
#include <QDBusPendingCallWatcher>
#include <TelepathyQt4/ConnectionManager>
#include <TelepathyQt4/PendingStringList>
#include <sys/time.h>

#define DEBUG_OBJECT_PATH "/org/freedesktop/Telepathy/debug"

Q_DECLARE_METATYPE(DebugProxy*);

bool DebugMessageCollector::mMessageHandlerInstalled = false;
QtMsgHandler DebugMessageCollector::mPreviousMsgHandler = 0;
DebugMessageList DebugMessageCollector::mAppMessages;

DebugMessageCollector::DebugMessageCollector(TelepathyManager *tpManager) :
    mTpManager(tpManager),
    mPendingGetMessageCalls(0),
    mPendingDumpToFiles(false)
{
    qDebug() << "DebugMessageCollector::DebugMessageCollector";

    if (!tpManager) {
        return;
    }

    createDebugProxy("mc", "org.freedesktop.Telepathy.MissionControl5");

    QList<Tp::AccountPtr> accounts = mTpManager->accounts();
    foreach(Tp::AccountPtr account, accounts) {
        if (account->connection()) {
            createDebugProxy(account->connection()->cmName(), account->connection()->busName());
        }
    }

    createDebugProxy("logger", "org.freedesktop.Telepathy.Logger");

    connect(tpManager,
            SIGNAL(connectionAvailable(Tp::ConnectionPtr)),
            SLOT(onConnectionAvailable(Tp::ConnectionPtr)));
}

DebugMessageCollector::~DebugMessageCollector()
{
    qDebug() << "DebugMessageCollector::~DebugMessageCollector";

    foreach(DebugProxyInfo proxyInfo, mDebugProxies) {
        delete proxyInfo.debugProxy;
    }
}

void DebugMessageCollector::createDebugProxy(const QString &name, const QString &busName)
{
    qDebug() << "DebugMessageCollector::createDebugProxy: name" << name << "busName=" << busName;

    foreach(DebugProxyInfo info, mDebugProxies) {
        if (info.busName == busName) {
            qDebug() << "DebugMessageCollector::createDebugProxy: duplicated";
            return;
        }
    }

    QDBusConnection connection = QDBusConnection::sessionBus();
    DebugProxyInfo info;
    info.name = name;
    info.busName = busName;
    info.debugProxy = new DebugProxy(busName, DEBUG_OBJECT_PATH, connection, this);
    if (!info.debugProxy) {
        qDebug() << "DebugMessageCollector::createDebugProxy: not possible to create DebugProxy";
        return ;
    }

    setupDebugProxy(info.debugProxy);

    mDebugProxies.append(info);;
}

void DebugMessageCollector::setupDebugProxy(DebugProxy *debugProxy)
{
    //qDebug() << "DebugMessageCollector::setupDebugProxy: " << debugProxy;

    if(!debugProxy) {
        return;
    }

    getMessagesFromDebugProxy(debugProxy);

    debugProxy->setProperty("Enabled", true);

    connect(debugProxy,
            SIGNAL(destroyed()),
            SLOT(onDebugProxyDestroyed()));
    connect(debugProxy,
            SIGNAL(NewDebugMessage(double,QString,uint,QString)),
            SLOT(onNewDebugMessage(double,QString,uint,QString)));
}

void DebugMessageCollector::getMessagesFromDebugProxy(DebugProxy *debugProxy)
{
    //qDebug() << "DebugMessageCollector::getMessagesFromDebugProxy: " << debugProxy;

    if(!debugProxy) {
        return;
    }

    QDBusPendingReply<DebugMessageList> dbusReply = debugProxy->GetMessages();
    if (dbusReply.isError()) {
        qDebug() << "DebugMessageCollector::getMessagesFromDebugProxy: error in call GetMessages" << dbusReply.error().name() << dbusReply.error().message();
        return;
    }

    if (dbusReply.isFinished()) {
        qDebug() << "DebugMessageCollector::getMessagesFromDebugProxy: messageList length=" << dbusReply.value().length();
        mMessages[debugProxy].append(dbusReply.value());
        dumpCheck();
        return;
    }

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(dbusReply, this);
    if (!watcher) {
        qDebug() << "DebugMessageCollector::getMessagesFromDebugProxy: error setting up dbus watcher";
        return;
    }

    watcher->setProperty("debugProxy", QVariant::fromValue(debugProxy));

    connect(watcher,
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            SLOT(onGetMessagesFinished(QDBusPendingCallWatcher*)));

    mPendingGetMessageCalls++;
}

void DebugMessageCollector::onGetMessagesFinished(QDBusPendingCallWatcher *watcher)
{
    //qDebug() << "DebugMessageCollector::onGetMessagesFinished: " << watcher;

    QDBusPendingReply<DebugMessageList> reply = *watcher;
    if (reply.isError()) {
        qDebug() << "DebugMessageCollector::onGetMessagesFinished: error in reply";
        return;
    }

    DebugProxy *debugProxy = watcher->property("debugProxy").value<DebugProxy*>();
    if (!debugProxy) {
        qDebug() << "DebugMessageCollector::onGetMessagesFinished: debugProxy not found";
        return;
    }

    qDebug() << "DebugMessageCollector::getMessagesFromDebugProxy: messageList length=" << reply.value().length();
    mMessages[debugProxy].append(reply.value());

    mPendingGetMessageCalls--;
    dumpCheck();
}

void DebugMessageCollector::onDebugProxyDestroyed()
{
    DebugProxy *debugProxy = qobject_cast<DebugProxy *> (sender());
    qDebug() << "DebugMessageCollector::onDebugProxyDestroyed: " << debugProxy;

    for(int i = 0;i < mDebugProxies.length();i++) {
        DebugProxyInfo info = mDebugProxies.at(i);
        if (info.debugProxy == debugProxy) {
            // will just delete from the debug proxies list, but better to keep the messages
            qDebug() << "DebugMessageCollector::onDebugProxyDestroyed: found at position " << i;
            mDebugProxies.removeAt(i);
            return;
        }
    }

    qDebug() << "DebugMessageCollector::onDebugProxyDestroyed: not found";
}

void DebugMessageCollector::onNewDebugMessage(double time, const QString &domain, uint level, const QString &message)
{
    //qDebug() << "DebugMessageCollector::onNewDebugMessage: " << time << domain << level << message;

    DebugProxy *debugProxy = qobject_cast<DebugProxy*> (sender());
    if (!debugProxy) {
        return;
    }

    DebugMessage debugMessage(time, domain, DebugLevel(level), message);
    mMessages[debugProxy].append(debugMessage);
}

void DebugMessageCollector::onConnectionAvailable(Tp::ConnectionPtr connection)
{
    qDebug() << "DebugMessageCollector::onConnectionAvailable: " << connection;

    if (!connection) {
        return;
    }

    createDebugProxy(connection->cmName(), connection->busName());
}

void DebugMessageCollector::dumpCheck()
{
    if (!mPendingGetMessageCalls) {
        if (mPendingDumpToFiles) {
            dumpToFiles();
        }
    }
}

void DebugMessageCollector::dumpToConsole()
{
    foreach(DebugProxyInfo proxyInfo, mDebugProxies) {
        qDebug() << "**** Begin dump " << proxyInfo.busName << "***";
        DebugMessageList messageList = mMessages[proxyInfo.debugProxy];
        foreach(DebugMessage message, messageList) {
            int ms = (int) ((message.time - (int) message.time)*1e6);
            time_t sec = (long) message.time;
            QDateTime dateTime = QDateTime::fromTime_t(sec);
            QString dateTimeString = QString(dateTime.toString(Qt::ISODate) + ".%1").arg(ms,6,10,QChar('0'));
            qDebug() << dateTimeString << (message.domain + ":%1").arg(message.level) << message.message;
        }
        qDebug() << "**** End dump " << proxyInfo.busName << "***";
    }
}

void DebugMessageCollector::dumpToFiles()
{
    if (mPendingGetMessageCalls) {
        qDebug() << "DebugMessageCollector::dumpToFiles postponed due to pending calls " << mPendingGetMessageCalls;
        mPendingDumpToFiles = true;
        return;
    }

    mPendingDumpToFiles = false;

    dumpMessagesToFile("/tmp/log-im.app.txt", mAppMessages);

    foreach(DebugProxyInfo proxyInfo, mDebugProxies) {
        QString fileName = QString("/tmp/log-im.%2.txt").arg(proxyInfo.name);
        DebugMessageList messageList = mMessages[proxyInfo.debugProxy];
        dumpMessagesToFile(fileName, messageList);
    }
}

void DebugMessageCollector::dumpMessagesToFile(const QString &fileName, const DebugMessageList &messageList)
{
    qDebug() << "DebugMessageCollector::dumpMessagesToFile: " << fileName;
    QFile data(fileName);
    if (data.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
        QTextStream out(&data);
        foreach(DebugMessage message, messageList) {
            int ms = (int) ((message.time - (int) message.time)*1e6);
            time_t sec = (long) message.time;
            QDateTime dateTime = QDateTime::fromTime_t(sec);
            QString dateTimeString = QString(dateTime.toString(Qt::ISODate) + ".%1").arg(ms,6,10,QChar('0'));
            out << dateTimeString << " " << (message.domain + ":%1").arg(message.level) << " " << message.message << "\n";
        }
        data.close();
    }
}

void DebugMessageCollector::setupMessagHandler()
{
    QDateTime x = QDateTime::currentDateTime();

    if (!mMessageHandlerInstalled) {
        mPreviousMsgHandler = qInstallMsgHandler(DebugMessageCollector::customMessageHandler);
        mMessageHandlerInstalled = true;
    }
}

void DebugMessageCollector::customMessageHandler(QtMsgType type, const char *msg)
{
    struct timeval gtod;
    gettimeofday(&gtod, NULL);
    double time = gtod.tv_sec + double(gtod.tv_usec) / 1e6;

    DebugLevel level = DebugLevelDebug;
    if (type == QtDebugMsg) {
        level = DebugLevelDebug;
    } else if (type == QtWarningMsg) {
        level = DebugLevelWarning;
    } else if (type == QtCriticalMsg) {
        level = DebugLevelCritical;
    } else if (type == QtFatalMsg) {
        level = DebugLevelError;
    }
    DebugMessage message(time, "app", level, msg);
    mAppMessages.append(message);

    if (mPreviousMsgHandler) {
        mPreviousMsgHandler(type, msg);
    }
}
