/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef DEBUGMESSAGECOLLECTOR_H
#define DEBUGMESSAGECOLLECTOR_H

#include <QList>
#include <QObject>
#include "debugproxy.h"
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/Types>

class DebugProxy;
class TelepathyManager;

class DebugMessageCollector : QObject
{
    Q_OBJECT

public:
    DebugMessageCollector(TelepathyManager *tpManager);
    ~DebugMessageCollector();

    static void setupMessagHandler();

    void dumpToFiles();

private slots:
    void onNewDebugMessage(double time, const QString &domain, uint level, const QString &message);
    void onDebugProxyDestroyed();
    void onGetMessagesFinished(QDBusPendingCallWatcher *);
    void onConnectionAvailable(Tp::ConnectionPtr connection);

private:
    void createDebugProxy(const QString &name, const QString &busName);
    void setupDebugProxy(DebugProxy *debugProxy);
    void getMessagesFromDebugProxy(DebugProxy *debugProxy);
    void dumpCheck();
    void dumpToConsole();
    void dumpMessagesToFile(const QString &fileName, const DebugMessageList &messageList);

    static void customMessageHandler(QtMsgType type, const char *msg);

    struct DebugProxyInfo {
        QString name;
        QString busName;
        DebugProxy *debugProxy;
    };

    static bool mMessageHandlerInstalled;
    static QtMsgHandler mPreviousMsgHandler;
    static DebugMessageList mAppMessages;

    TelepathyManager *mTpManager;
    QList<DebugProxyInfo> mDebugProxies;
    QHash<DebugProxy*, DebugMessageList> mMessages;
    int mPendingGetMessageCalls;
    bool mPendingDumpToFiles;
};

#endif // DEBUGMESSAGECOLLECTOR_H
