/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <telepathy-farstream/telepathy-farstream.h>
#include "telepathymanager.h"

#include <TelepathyQt4/Account>
#include <TelepathyQt4/ChannelClassSpecList>
#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ConnectionCapabilities>
#include <TelepathyQt4/ConnectionFactory>
#include <TelepathyQt4/ContactFactory>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4Yell/ChannelFactory>
#include <glib-object.h>
#include <gst/gst.h>
#include <QSetIterator>

TelepathyManager::TelepathyManager(bool fullStart)
    : mConnCount(0),
      mChannelHandler(0),
      mFullStart(fullStart),
      mFinished(false)
{
    qDebug() << "TelepathyManager::TelepathyManager: ";

    mAccountFeatures << Tp::Account::FeatureCore
                     << Tp::Account::FeatureAvatar
                     << Tp::Account::FeatureProtocolInfo
                     << Tp::Account::FeatureCapabilities
                     << Tp::Account::FeatureProfile;
    mContactFeatures << Tp::Contact::FeatureAlias
                     << Tp::Contact::FeatureAvatarToken
                     << Tp::Contact::FeatureSimplePresence
                     << Tp::Contact::FeatureAvatarData
                     << Tp::Contact::FeatureCapabilities;
    mConnectionFeatures << Tp::Connection::FeatureCore
                        << Tp::Connection::FeatureSelfContact
                        << Tp::Connection::FeatureSimplePresence
                        << Tp::Connection::FeatureRoster
                        << Tp::Connection::FeatureRosterGroups;
                        //<< Tp::Connection::FeatureAccountBalance;

#if 1
    // remove following code once tp-qt4-yell is merged into tp-qt4
    tf_init();

    Tpy::ChannelFactoryPtr channelFactory =
        Tpy::ChannelFactory::create(QDBusConnection::sessionBus());
    mAccountManager = Tp::AccountManager::create(
            Tp::AccountFactory::create(QDBusConnection::sessionBus(), mAccountFeatures),
            Tp::ConnectionFactory::create(QDBusConnection::sessionBus(), mConnectionFeatures),
            channelFactory,
            Tp::ContactFactory::create(mContactFeatures));
#else
    mAccountManager = Tp::AccountManager::create();
#endif

    connect(this, SIGNAL(finished()), SLOT(onFinished()));
    connect(mAccountManager->becomeReady(Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

TelepathyManager::~TelepathyManager()
{
    qDebug() << "TelepathyManager::~TelepathyManager: ";
}

void TelepathyManager::onAccountManagerReady(Tp::PendingOperation *op)
{
    qDebug() << "TelepathyManager::onAccountManagerReady: ";

    if (op->isError()) {
        qWarning() << "AccountManager cannot become ready";
        return;
    }

    mAccounts = mAccountManager->allAccounts();
    mConnCount = mAccounts.count();

    // only prepare the accounts if it's a full start
    if (mFullStart) {
        initializeAccounts();
    }

    emit accountManagerReady();

    connect(mAccountManager.data(),
            SIGNAL(newAccount(Tp::AccountPtr)),
            SLOT(onNewAccount(Tp::AccountPtr)));

    // if there is no account, the loading is finished
    if (!mConnCount)
        emit finished();
}

void TelepathyManager::initializeAccounts()
{
    qDebug() << "TelepathyManager::initializeAccounts: ";

    QList<Tp::AccountPtr>::const_iterator ac_it;
    for (ac_it = mAccounts.constBegin(); ac_it != mAccounts.constEnd(); ++ac_it) {
        qDebug() << "TelepathyManager::initializeAccounts: become ready "
                 << " account=" << (*ac_it)->uniqueIdentifier()
                 << " features=" << mAccountFeatures;
        connect((*ac_it)->becomeReady(mAccountFeatures),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onAccountReady(Tp::PendingOperation*)));
    }
}

void TelepathyManager::initializeConnection(const Tp::ConnectionPtr &connection)
{
    qDebug() << "TelepathyManager::initializeConnection: "
             << " connection=" << connection.data()
             << " features=" << mConnectionFeatures;

    connect(connection->becomeReady(mConnectionFeatures),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onConnectionReady(Tp::PendingOperation*)));
}

void TelepathyManager::initializeContacts(const Tp::ContactManagerPtr &contactManager)
{
    qDebug() << "TelepathyManager::initializeContacts: become ready "
             << " features=" << mContactFeatures;

    Tp::Contacts contacts = contactManager->allKnownContacts();
    QSetIterator<Tp::ContactPtr> contactIterator(contacts);
    while (contactIterator.hasNext()) {
        Tp::ContactPtr contact = contactIterator.next();
        qDebug() << " contact " << contact << contact.data() << " id=" << contact->id();
    }

    if(contactManager->connection()->actualFeatures().contains(Tp::Connection::FeatureSelfContact)) {
        Tp::ContactPtr selfContact = contactManager->connection()->selfContact();
        contacts.insert(selfContact);
    }

    connect(contactManager->upgradeContacts(contacts.toList(), mContactFeatures),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onContactsUpgraded(Tp::PendingOperation*)));
    emit upgradingContacts();
}

void TelepathyManager::initializeChannelHandler()
{
    qDebug() << "TelepathyManager::initializeChannelHandler: initializing channel handler";

    mChannelHandler = new ChannelHandler();
    Tp::AbstractClientPtr handler(mChannelHandler);
    mClientRegistrar->registerClient(handler, "MeeGoIM");
    emit handlerRegistered();

    //connect when a text channel is available to process group chats
    connect(mChannelHandler, SIGNAL(textChannelAvailable(QString,Tp::TextChannelPtr)),
            SLOT(onTextChannelAvailable(QString,Tp::TextChannelPtr)));
}

void TelepathyManager::onNewAccount(const Tp::AccountPtr &account)
{
    qDebug() << "TelepathyManager::onNewAccount: "
                << " account=" << account->uniqueIdentifier();

    // add to the list and initialize
    mAccounts.append(account);
    connect(account->becomeReady(mAccountFeatures),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountReady(Tp::PendingOperation*)));
}

void TelepathyManager::onAccountReady(Tp::PendingOperation *op)
{
    qDebug() << "TelepathyManager::onAccountReady:";

    if (op->isError()) {
        qWarning() << "Account cannot become ready";
        return;
    }

    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady *>(op);
    Tp::AccountPtr accountPtr = Tp::AccountPtr::dynamicCast(pr->object());

    // only cascade the initialization if it is a full start
    if (mFullStart) {
        if (!accountPtr->connection().isNull()
                && accountPtr->connection()->isValid()
                && accountPtr->connection()->status() == Tp::ConnectionStatusConnected) {
            initializeConnection(accountPtr->connection());
        } else {
            --mConnCount;
        }
    }
    if (mConnCount == 0) {
        emit finished();
    }


    connect(accountPtr.data(), SIGNAL(connectionChanged(const Tp::ConnectionPtr &)),
        SLOT(onAccountConnectionChanged(const Tp::ConnectionPtr &)));
    onAccountConnectionChanged(accountPtr->connection());

    emit accountReady(accountPtr.data());
}

void TelepathyManager::onAccountConnectionChanged(const Tp::ConnectionPtr &conn)
{
    qDebug() << "TelepathyManager::onAccountConnectionChanged";

    if (conn == NULL) {
        qDebug() << "NULL connection";
    }
    else {
        connect(conn.data(),
            SIGNAL(statusChanged(Tp::ConnectionStatus)),
            SLOT(onConnectionStatusChanged(Tp::ConnectionStatus)));
        connect(conn.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(onConnectionInvalidated(Tp::DBusProxy*)));
        onConnectionStatusChanged(conn->status());
    }
}

void TelepathyManager::onConnectionReady(Tp::PendingOperation *op)
{
    qDebug() << "TelepathyManager::onConnectionReady: op=" << op;

    if (!op || op->isError()) {
        qWarning() << "Connection cannot become ready";
        return;
    }

    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady *>(op);
    if (!pr) {
        qWarning() << "Connection cannot become ready, null pending ready object";
        return;
    }
    Tp::ConnectionPtr conn = Tp::ConnectionPtr::dynamicCast(pr->object());
    if (conn.isNull()) {
        qWarning() << "Connection cannot become ready, null connection";
        return;
    }

    qDebug() << " Connection ready "
             << " connection=" << conn.data()
             << " cmName=" << conn->cmName()
             << " protocolName=" << conn->protocolName()
             << " features=" << conn->actualFeatures()
             << " isReady=" << conn->isReady();
    mConnections.append(conn);

    connect(conn->contactManager().data(),
            SIGNAL(allKnownContactsChanged(const Tp::Contacts&,
                                           const Tp::Contacts&,
                                           const Tp::Channel::GroupMemberChangeDetails&)),
            SLOT(onAllKnownContactsChanged(const Tp::Contacts&,
                                           const Tp::Contacts&)));
    // only cascade the initialization if it is a full start
    if (mFullStart) {
        // initialize contacts
        initializeContacts(conn->contactManager());
    }
    emit connectionReady(conn);
}

void TelepathyManager::onConnectionStatusChanged(Tp::ConnectionStatus status)
{
    qDebug() << "TelepathyManager::onConnectionStatusChanged: status=" << status;

    Tp::Connection *conn = qobject_cast<Tp::Connection *>(sender());

    if (conn && status == Tp::ConnectionStatusConnected) {
        initializeConnection(Tp::ConnectionPtr(conn));
    }
}

void TelepathyManager::onConnectionInvalidated(Tp::DBusProxy *proxy)
{
    qDebug() << "TelepathyManager::onConnectionInvalidated";

    Tp::Connection *conn = qobject_cast<Tp::Connection *>(proxy);
    if (conn) {
        disconnect(conn, 0, this, 0);
   }
}

void TelepathyManager::onContactsUpgraded(Tp::PendingOperation *op)
{
    qDebug() << "TelepathyManager::onContactsUpgraded: op=" << op;

    if (!op || op->isError()) {
        qWarning() << "Contacts cannot be upgraded";
        return;
    }

    --mConnCount;

    Tp::PendingContacts *pendingContacts = qobject_cast<Tp::PendingContacts *>(op);
    QList<Tp::ContactPtr> contacts = pendingContacts->contacts();
    mContacts.append(contacts);

    emit contactsUpgraded(contacts);

    if (mConnCount <= 0) {
        emit finished();
    }
}

void TelepathyManager::onAllKnownContactsChanged(const Tp::Contacts &added, const Tp::Contacts &removed)
{
    Q_UNUSED(removed)

    if (!added.count()) {
        return;
    }

    Tp::ContactManagerPtr contactManager = (*added.begin())->manager();

    // no need to connect to onContactsUpgraded at this point
    // the finished signal was already emitted
    contactManager->upgradeContacts(added.toList(), mContactFeatures);
}

void TelepathyManager::registerClients()
{
    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactoryPtr::constCast(mAccountManager->channelFactory());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);
    mClientRegistrar = Tp::ClientRegistrar::create(mAccountManager);

    initializeChannelHandler();

    int argc = 0;
    char **argv = { 0 };
    g_type_init();
    gst_init(&argc, &argv);
}

void TelepathyManager::onTextChannelAvailable(const QString &accountId, Tp::TextChannelPtr channel)
{
    Q_UNUSED(accountId);

    qDebug() << "TelepathyManager::onTextChannelAvailable(): new text channel available for account " << accountId;

    // when a new channel is available,
    // if it is a conference channels, the contacts should be upgraded
    if(channel->isConference()) {
        upgradeContacts(channel->groupContacts().toList());
        upgradeContacts(channel->groupLocalPendingContacts().toList());
        upgradeContacts(channel->groupRemotePendingContacts().toList());
        mGroupChannels.append(channel);
        connect(channel.data(),
                SIGNAL(groupMembersChanged(Tp::Contacts,Tp::Contacts,Tp::Contacts,Tp::Contacts,Tp::Channel::GroupMemberChangeDetails)),
                SLOT(onGroupChatMembersChanged(Tp::Contacts,Tp::Contacts,Tp::Contacts,Tp::Contacts,Tp::Channel::GroupMemberChangeDetails)));
    } else {
        // check if the contact is not upgraded and upgrade it if necessary
        QList<Tp::ContactPtr> contactsList;
        foreach(Tp::ContactPtr contact, channel->groupContacts().toList()) {
            if (!contact->actualFeatures().contains(Tp::Contact::FeatureAlias)) {
                contactsList.append(contact);
            }
        }
        if(!contactsList.isEmpty()) {
            upgradeContacts(contactsList);
        }
    }
}

void TelepathyManager::onGroupChatMembersChanged(const Tp::Contacts &groupMembersAdded,
                                                 const Tp::Contacts &groupLocalPendingMembersAdded,
                                                 const Tp::Contacts &groupRemotePendingMembersAdded,
                                                 const Tp::Contacts &groupMembersRemoved,
                                                 const Tp::Channel::GroupMemberChangeDetails &details)
{
    Q_UNUSED(groupMembersRemoved);
    Q_UNUSED(details);
    QList<Tp::ContactPtr> contactsList = groupMembersAdded.toList();
    contactsList.append(groupLocalPendingMembersAdded.toList());
    contactsList.append(groupRemotePendingMembersAdded.toList());

    QListIterator<Tp::ContactPtr> contactIterator(contactsList);
    while (contactIterator.hasNext()) {
        Tp::ContactPtr contact = contactIterator.next();
        qDebug() << " group chat contact " << contact << contact.data() << " id=" << contact->id();
    }

    upgradeContacts(contactsList);
}

void TelepathyManager::upgradeContacts(const QList<Tp::ContactPtr> contacts)
{
    qDebug("upgrading contacts");
    qDebug("contacts to upgrade: %d", contacts.count());
    if(!contacts.isEmpty()) {

        Tp::ContactPtr contact = contacts.at(0);
        Tp::ContactManagerPtr contactManager = contact->manager();

        connect(contactManager->upgradeContacts(contacts, mContactFeatures),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onContactsUpgraded(Tp::PendingOperation*)));
        emit upgradingContacts();
    }
}

void TelepathyManager::setAccountFeatures(const Tp::Features &features)
{
    mAccountFeatures = features;
}

void TelepathyManager::setContactFeatures(const Tp::Features &features)
{
    mContactFeatures = features;
}

void TelepathyManager::setConnectionFeatures(const Tp::Features &features)
{
    mConnectionFeatures = features;
}

Tp::AccountManagerPtr TelepathyManager::accountManager(void)
{
    return mAccountManager;
}

QList<Tp::AccountPtr> TelepathyManager::accounts(void)
{
    return mAccounts;
}

QList<Tp::ConnectionPtr> TelepathyManager::connections(void)
{
    return mConnections;
}

QList<Tp::ContactPtr> TelepathyManager::contacts(void)
{
    return mContacts;
}

ChannelHandler *TelepathyManager::channelHandler() const
{
    return mChannelHandler;
}

QVariantMap TelepathyManager::availableAccounts() const
{
    QVariantMap accounts;

    foreach(Tp::AccountPtr account, mAccounts) {

        accounts.insert(account->uniqueIdentifier(),
                        accountDisplayName(account->iconName(), account->displayName()));
    }
    return accounts;
}

QStringList TelepathyManager::availableContacts(QString accountId) const
{
    QStringList contactStrings;

    foreach(Tp::AccountPtr account, mAccounts) {
        if(account->uniqueIdentifier() == accountId) {
            if(!account->connection().isNull()) {
                QList<Tp::ContactPtr> contacts = account->connection()->contactManager()->allKnownContacts().toList();
                foreach(Tp::ContactPtr contact, contacts) {
                    QString friendName = QString("(" + contact->alias() + ") " + contact->id());
                    contactStrings.append(friendName);
                }
            }
        }
    }
    return contactStrings;
}

void TelepathyManager::setProtocolNames(QMap<QString, QString> names)
{
    mProtocols = names;
}

int TelepathyManager::accountsOfType(const QString &iconName) const
{
    int count = 0;
    foreach(Tp::AccountPtr account, mAccounts) {
        if (account->iconName() == iconName) {
            count++;
        }
    }
    return count;
}

QString TelepathyManager::accountServiceName(const QString &iconName) const
{
   if (mProtocols.contains(iconName)) {
       return mProtocols[iconName];
   }

   return QString();
}

QString TelepathyManager::accountDisplayName(const QString &iconName, const QString &displayName) const
{
    QString accountService = accountServiceName(iconName);
    if(accountsOfType(iconName) > 1) {
        // i18n: first arg is the display name, second is the service name
        return tr("%1 - %2").arg(displayName, accountService);
    } else {
        return accountService;
    }
}

bool TelepathyManager::isFinished() const
{
    return mFinished;
}

void TelepathyManager::onFinished()
{
    mFinished = true;
}
