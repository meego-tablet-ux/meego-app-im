/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef TELEPATHYMANAGER_H
#define TELEPATHYMANAGER_H

#include <QObject>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ConnectionManager>
#include <TelepathyQt4/Types>
#include "channelhandler.h"

class TelepathyManager : public QObject
{
    Q_OBJECT

public:
    explicit TelepathyManager(bool fullStart = false);
    ~TelepathyManager();
    void setAccountFeatures(const Tp::Features &features);
    void setConnectionFeatures(const Tp::Features &features);
    void setContactFeatures(const Tp::Features &features);
    Tp::AccountManagerPtr accountManager(void);
    ChannelHandler *channelHandler() const;
    QList<Tp::AccountPtr> accounts(void);
    QList<Tp::ConnectionPtr> connections(void);
    QList<Tp::ContactPtr> contacts(void);

    // These two methods are to provide information to the MeeGo Contacts application
    // Please check with them before modifying the API or the format of the returned values
    Q_INVOKABLE QVariantMap availableAccounts() const;
    Q_INVOKABLE QStringList availableContacts(QString accountId) const;
    void setProtocolNames(QMap<QString, QString> names);
    QString accountDisplayName(const QString &iconName, const QString &displayName) const;
    bool isFinished() const;

public Q_SLOTS:
    void initializeAccounts();
    void initializeConnection(const Tp::ConnectionPtr &conn);
    void initializeContacts(const Tp::ContactManagerPtr &contactManager);
    void initializeChannelHandler();
    void registerClients(void);

Q_SIGNALS:
    void accountManagerReady();
    void accountReady(Tp::Account *account);
    void connectionReady(Tp::ConnectionPtr connection);
    void upgradingContacts(void);
    void contactsUpgraded(QList<Tp::ContactPtr>);
    void finished();
    void handlerRegistered();

protected:
    void upgradeContacts(const QList<Tp::ContactPtr> contacts);
    Q_INVOKABLE QString accountServiceName(const QString &iconName) const;
    Q_INVOKABLE int accountsOfType(const QString &iconName) const;

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onNewAccount(const Tp::AccountPtr &account);
    void onAccountReady(Tp::PendingOperation *op);
    void onConnectionReady(Tp::PendingOperation *op);
    void onConnectionStatusChanged(Tp::ConnectionStatus status);
    void onContactsUpgraded(Tp::PendingOperation *op);
    void onAllKnownContactsChanged(const Tp::Contacts &added, const Tp::Contacts &removed);
    void onTextChannelAvailable(const QString &accountId, Tp::TextChannelPtr channel);
    void onGroupChatMembersChanged(const Tp::Contacts &groupMembersAdded,
                                   const Tp::Contacts &groupLocalPendingMembersAdded,
                                   const Tp::Contacts &groupRemotePendingMembersAdded,
                                   const Tp::Contacts &groupMembersRemoved,
                                   const Tp::Channel::GroupMemberChangeDetails &details);
    void onFinished();
    void onAccountConnectionChanged(const Tp::ConnectionPtr &conn);
    void onConnectionInvalidated(Tp::DBusProxy *proxy);

private:
    Tp::AccountManagerPtr mAccountManager;
    Tp::Features mAccountManagerFeatures;
    QList<Tp::AccountPtr> mAccounts;
    QList<Tp::ChannelPtr> mGroupChannels;
    QList<Tp::ConnectionPtr> mConnections;
    QList<Tp::ContactPtr> mContacts;
    Tp::Features mAccountFeatures;
    Tp::Features mContactFeatures;
    Tp::Features mConnectionFeatures;
    int mConnCount;
    ChannelHandler *mChannelHandler;
    Tp::ClientRegistrarPtr mClientRegistrar;
    bool mFullStart;
    bool mFinished;
    QMap<QString,QString> mProtocols;
};

#endif // TELEPATHYMANAGER_H
