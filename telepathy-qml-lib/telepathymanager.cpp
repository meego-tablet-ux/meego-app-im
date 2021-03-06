/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

// next include must be first
#include <telepathy-farstream/telepathy-farstream.h>
#include "telepathymanager.h"
#include "debugmessagecollector.h"

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

TelepathyManager *TelepathyManager::mTelepathyManager = 0;

TelepathyManager::TelepathyManager(QObject *parent)
    : QObject(parent),
      mChannelHandler(0),
      mFinished(false),
      mDebugMessageCollector(0)
{
    qDebug() << "TelepathyManager::TelepathyManager: ";

    if (!mTelepathyManager) {
        mTelepathyManager = this;
    }

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

    // TODO: replace the channel factory once tp-qt4-yell is merged into tp-qt4
    qDebug() << "TelepathyManager::TelepathyManager: initialize tp-qt4-yell";
    qDebug() << "TelepathyManager::TelepathyManager: creating Tpy::ChannelFactory";
    Tpy::ChannelFactoryPtr channelFactory =
        Tpy::ChannelFactory::create(QDBusConnection::sessionBus());

    qDebug() << "TelepathyManager::TelepathyManager: creating Tp::AccountManager";
    mAccountManager = Tp::AccountManager::create(
            Tp::AccountFactory::create(QDBusConnection::sessionBus(), mAccountFeatures),
            Tp::ConnectionFactory::create(QDBusConnection::sessionBus(), mConnectionFeatures),
            channelFactory,
            Tp::ContactFactory::create(mContactFeatures));

    connect(this, SIGNAL(finished()), SLOT(onFinished()));
    connect(mAccountManager->becomeReady(Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

TelepathyManager::~TelepathyManager()
{
    qDebug() << "TelepathyManager::~TelepathyManager: ";
}

TelepathyManager *TelepathyManager::instance()
{
    if (!mTelepathyManager) {
        qDebug() << "Autocreating TelepathyManager instance";
        mTelepathyManager = new TelepathyManager();
    }

    return mTelepathyManager;
}

void TelepathyManager::onAccountManagerReady(Tp::PendingOperation *op)
{
    qDebug() << "TelepathyManager::onAccountManagerReady: ";

    if (op->isError()) {
        qWarning() << "AccountManager cannot become ready";
        return;
    }

    foreach (Tp::AccountPtr account, mAccountManager->allAccounts()) {
        // the onNewAccount method will take care of connecting everything that is necessary
        onNewAccount(account);
    }

    emit accountManagerReady();

    connect(mAccountManager.data(),
            SIGNAL(newAccount(Tp::AccountPtr)),
            SLOT(onNewAccount(Tp::AccountPtr)));

    emit finished();
}

void TelepathyManager::initializeChannelHandler()
{
    qDebug() << "TelepathyManager::initializeChannelHandler: initializing channel handler";

    mChannelHandler = new ChannelHandler();
    Tp::AbstractClientPtr handler(mChannelHandler);
    mClientRegistrar->registerClient(handler, "MeeGoIM");
    emit handlerRegistered();
}

void TelepathyManager::onNewAccount(const Tp::AccountPtr &account)
{
    if (account.isNull() || !account->isValid()) {
        return;
    }

    qDebug() << "TelepathyManager::onNewAccount: "
                << " account=" << account->uniqueIdentifier();

    connect(account.data(),
            SIGNAL(connectionChanged(const Tp::ConnectionPtr&)),
            SLOT(onConnectionChanged(const Tp::ConnectionPtr&)));
    connect(account.data(),
            SIGNAL(removed()),
            SLOT(onAccountRemoved()));

    // no need to check if the connection is null, as it is checked inside the function
    Tp::ConnectionPtr connection = account->connection();
    onConnectionChanged(connection);

    // add to the list and initialize
    mAccounts.append(account);
    emit accountAvailable(account);
}

void TelepathyManager::onConnectionChanged(const Tp::ConnectionPtr &connection)
{
    qDebug() << "TelepathyManager::onConnectionChanged: connection=" << connection.data();

    if (!connection.isNull()) {
        emit connectionAvailable(connection);
    }
}

void TelepathyManager::registerClients()
{
    tf_init();

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactoryPtr::constCast(mAccountManager->channelFactory());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);
    mClientRegistrar = Tp::ClientRegistrar::create(mAccountManager);

    initializeChannelHandler();

    int argc = 0;
    char **argv = { 0 };
    g_type_init();
    gst_init(&argc, &argv);
}

void TelepathyManager::dumpLogs()
{
    qDebug() << "TelepathyManager::dumpLogs";

    if (!mDebugMessageCollector) {
        mDebugMessageCollector = new DebugMessageCollector(this);
    }

    if (mDebugMessageCollector) {
        mDebugMessageCollector->dumpToFiles();
    }
}

Tp::AccountManagerPtr TelepathyManager::accountManager(void)
{
    return mAccountManager;
}

QList<Tp::AccountPtr> TelepathyManager::accounts(void)
{
    return mAccounts;
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
        //: %1 is the display name for contact - %2 is the service name
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

void TelepathyManager::onAccountRemoved()
{
    Tp::AccountPtr account(qobject_cast<Tp::Account*> (sender()));
    mAccounts.removeOne(account);
}
