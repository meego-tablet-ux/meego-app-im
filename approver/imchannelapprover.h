/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef IMCHANNELAPPROVER_H
#define IMCHANNELAPPROVER_H

#include <QDBusServiceWatcher>
#include <TelepathyQt4/AbstractClientApprover>
#include <TelepathyQt4/ChannelDispatchOperation>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/IncomingFileTransferChannel>
#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4Yell/CallChannel>
#include "../telepathy-qml-lib/telepathymanager.h"
#include "../telepathy-qml-lib/notificationmanager.h"

class IMApproverAdaptor;
class IMProtocolsModel;

class IMChannelApprover : public QObject, public Tp::AbstractClientApprover
{
    Q_OBJECT
public:
    IMChannelApprover(bool autoApproveCalls = false);
    ~IMChannelApprover();

    void addDispatchOperation(const Tp::MethodInvocationContextPtr<> &context,
                              const Tp::ChannelDispatchOperationPtr &dispatchOperation);
    Tp::ChannelClassSpecList channelFilters() const;

Q_SIGNALS:
    void addedDispatchOperation(const Tp::ChannelDispatchOperationPtr dispatchOperation);
    void textChannelAvailable(const QString &accountId, Tp::TextChannelPtr channel);
    void callChannelAvailable(const QString &accountId, Tpy::CallChannelPtr channel);
    void fileTransferChannelAvailable(const QString &accountId, Tp::IncomingFileTransferChannelPtr channel);
    void incomingCall(const QString &accountId, const QString &contactId, const QString &operationPath);
    void invalidated(void);

public Q_SLOTS:
    void onCloseOperation(QString operationObjectPath);
    void setApplicationRunning(bool running);
    void registerApprover();

private Q_SLOTS:
    void onCallChannelReady(Tp::PendingOperation *op);
    void onCallChannelStateChanged(Tpy::CallState state);
    void onTextChannelReady(Tp::PendingOperation *op);
    void onFileTransferChannelReady(Tp::PendingOperation *op);
    void onInvalidated(void);
    void acceptCall(const QString &accountId, const QString &contactId);
    void rejectCall(const QString &accountId, const QString &contactId);
    void onServiceRegistered();
    void onServiceUnregistered();

private:
    QList<Tp::ChannelDispatchOperationPtr> mDispatchOps;
    bool mApplicationRunning;
    Tp::ClientRegistrarPtr mClientRegistrar;
    TelepathyManager *mTelepathyManager;
    IMApproverAdaptor *mAdaptor;
    QDBusServiceWatcher mIMServiceWatcher;
    bool mAutoApproveCalls;
    bool mPendingCall;
    IMProtocolsModel *mProtocolsModel;
};

#endif // IMCHANNELAPPROVER_H
