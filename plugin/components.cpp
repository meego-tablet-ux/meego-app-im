/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "components.h"
#include "accountsmodelfactory.h"
#include "accountssortfilterproxymodel.h"
#include "contactssortfilterproxymodel.h"
#include "contactssortproxymodel.h"
#include "accounthelper.h"
#include "imaccountsmodel.h"
#include "imavatarimageprovider.h"
#include "textfile.h"
#include "imdbus.h"

#include "../telepathy-qml-lib/chatagent.h"
#include "../telepathy-qml-lib/debugmessage.h"
#include "../telepathy-qml-lib/debugmessagecollector.h"
#include "../telepathy-qml-lib/settingshelper.h"
#include "../telepathy-qml-lib/resourcesetmanager.h"

#include <TelepathyLoggerQt4/Init>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ChannelFactory>
#include <TelepathyQt4/Debug>
#include "flat-model-proxy.h"
#include <TelepathyQt4Yell/Types>
#include <TelepathyQt4Yell/CallChannel>

#include <QtDebug>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/qdeclarative.h>
#include <QSettings>
#include <QtGstQmlSink/qmlgstvideoitem.h>

#include <glib-object.h>

//#include "imtextedit.h"

void Components::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    qDebug() << "MeeGoIM initializeEngine" << uri;
    Q_ASSERT(engine);

    // needed for tp-logger
    g_type_init();

    DebugMessage::registerMetaType();
    // next line install custom handler to log messages in memory
    DebugMessageCollector::setupMessagHandler();

    Tp::registerTypes();
    //Tp::enableDebug(true);
    Tp::enableWarnings(true);

    Tpy::registerTypes();

    Tpl::init();

    mLastUsedAccountSignalEmitted = false;
    mTpManager = new TelepathyManager(this);
    connect(mTpManager, SIGNAL(accountManagerReady()), SLOT(onAccountManagerReady()));

    mProtocolsModel = new IMProtocolsModel(this);
    mTpManager->setProtocolNames(mProtocolsModel->protocolNames());

    AccountsModelFactory *accountFactory = new AccountsModelFactory(mTpManager);
    connect(accountFactory, SIGNAL(modelCreated(IMAccountsModel*)),SLOT(onAccountsModelReady(IMAccountsModel*)));

    mRootContext = engine->rootContext();
    Q_ASSERT(mRootContext);

    mRootContext->setContextProperty(QString::fromLatin1("accountsModel"), (QObject *) 0);

    mRootContext->setContextProperty(QString::fromLatin1("telepathyManager"), mTpManager);
    mRootContext->setContextProperty(QString::fromLatin1("protocolsModel"), mProtocolsModel);
    mRootContext->setContextProperty(QString::fromLatin1("accountFactory"), accountFactory);

    // create the notification manager
    mNotificationManager = new NotificationManager(this);
    mRootContext->setContextProperty(QString::fromLatin1("notificationManager"),
                                      mNotificationManager);

    // create the notification manager
    mRootContext->setContextProperty(QString::fromLatin1("settingsHelper"),
                                      SettingsHelper::self());

    // get last used account
    QSettings settings("MeeGo", "meego-app-im");
    mLastUsedAccount = settings.value("LastUsed/Account", QString()).toString();

    // get the network status and load it
    mNetworkStateProperty = new ContextProperty("Internet.NetworkState", this);
    mNetworkStateProperty->subscribe();
    connect(mNetworkStateProperty, SIGNAL(valueChanged()), this, SLOT(onNetworkStatusChanged()));
    onNetworkStatusChanged();
}

void Components::registerTypes(const char *uri)
{
    qmlRegisterType<AccountHelper>(uri, 0, 1, "AccountHelper");
    qmlRegisterType<QmlGstVideoItem>(uri, 0, 1, "VideoItem");
    qmlRegisterType<TextFile>(uri, 0, 1, "TextFile");
    qmlRegisterType<IMDBus>(uri, 0, 1, "IMDBus");
    qmlRegisterType<ResourceSetManager>(uri, 0, 1, "ResourceSetManager");
    qmlRegisterUncreatableType<IMAccountsModel>(uri, 0, 1, "IMAccountsModel", "This is a read-only type");
    qmlRegisterUncreatableType<ContactModelItem>(uri, 0, 1,"ContactModelItem", "This is a read-only type");
    qmlRegisterUncreatableType<TelepathyManager>(uri, 0, 1, "TelepathyManager", "This is a read-only type");
    qmlRegisterUncreatableType<ChatAgent>(uri, 0, 1, "ChatAgent", "This is a read-only type");
    qmlRegisterUncreatableType<SimpleContactsListModel>(uri, 0, 1, "SimpleContactsListModel", "This is a read-only type");
}

void Components::onAccountManagerReady()
{
    // register the avatar image provider
    IMAvatarImageProvider::registerProvider(mRootContext->engine(), mTpManager->accountManager());
}

void Components::onAccountsModelReady(IMAccountsModel *model)
{
    mAccountsModel = model;
    mAccountsModel->setTelepathyManager(mTpManager);
    connect(mAccountsModel, SIGNAL(newAccountItem(QString)),
            SLOT(onNewAccount(QString)));

    FlatModelProxy *flatModel = new FlatModelProxy(model);
    mMergedModel = new MergedModel(this);
    mMergedModel->addModel(flatModel);

    mGroupChatModel = new IMGroupChatModel(this);
    mGroupChatModel->setNotificationManager(mNotificationManager);
    mMergedModel->addModel(mGroupChatModel);

    // initialize the accounts sorted model
    AccountsSortFilterProxyModel *accountsSortedModel = new AccountsSortFilterProxyModel(model, this);

    ContactsSortProxyModel *sortedContactsModel = new ContactsSortProxyModel(mTpManager, mMergedModel, this);
    if (!sortedContactsModel) {
        qWarning() << "Error creating sorted contacts model";
        return;
    }

    mContactsModel = new ContactsSortFilterProxyModel(mTpManager, sortedContactsModel, this);

    // the load order is inverted so that signals emitted by the accountsModel can guarantee that the
    // contactsModel is present
    mRootContext->setContextProperty(QString::fromLatin1("contactsModel"), mContactsModel);
    emit contactsModelCreated();
    mRootContext->setContextProperty(QString::fromLatin1("accountsModel"), model);
    mRootContext->setContextProperty(QString::fromLatin1("accountsSortedModel"), accountsSortedModel);
    emit accountsModelCreated();

    connect(mTpManager, SIGNAL(handlerRegistered()), SLOT(onHandlerRegistered()));
    connect(this, SIGNAL(networkStatusChanged(bool)),
            mAccountsModel, SLOT(onNetworkStatusChanged(bool)));

    mAccountsModel->setNotificationManager(mNotificationManager);

    // this signals that all components have been loaded at this point
    // in turn, this signal can be used to call the tpManager and register
    // the handler and approver, if necessary
    mAccountsModel->onComponentsLoaded();

}

/**
  * This method checks whether the last used account is connected. If it is connected
  * it will trigger a signal to open the list of contacts for that account.
  * That is done through a signal in the contacts proxy model as a workaround because
  * this class cannot send a signal to the QML files on its own.
  */
void Components::loadLastUsedAccount(const QString accountId, IMAccountsModel *model)
{
    // disconnect the signal. One way or the other, this should only be called once
    disconnect(mAccountsModel, SIGNAL(newAccountItem(QString)),
               this, SLOT(onNewAccount(QString)));

    // locate the account object matching the id
    for (int i = 0; i < model->accountCount(); ++i) {
        QModelIndex index = model->index(i, 0, QModelIndex());
        Tp::AccountPtr account = model->accountForIndex(index);
        if (account->uniqueIdentifier() == accountId) {
            // only send the signal if the account is connected
            if (!account->connection().isNull()
                    && account->connection()->isValid()
                    && account->connection()->status() == Tp::ConnectionStatusConnected) {
                mContactsModel->filterByLastUsedAccount(accountId);
                return;
            }
        }
    }

    // send a signal with an empty accountId otherwise.
    // this is needed in case there are command line parameters
    // which would not be processed otherwise
    mLastUsedAccountSignalEmitted = true;
    mContactsModel->filterByLastUsedAccount(QString());
}

void Components::onNetworkStatusChanged()
{
    QString networkState = mNetworkStateProperty->value().toString();
    bool isOnline = true;
    if (!networkState.isEmpty() && networkState == "disconnected") {
        isOnline = false;
    }
    mRootContext->setContextProperty(QString::fromLatin1("networkOnline"), QVariant(isOnline));
    emit networkStatusChanged(isOnline);
}

void Components::onHandlerRegistered()
{
    // only do it if the handler has been created
    if (mTpManager->channelHandler()) {
        connect(mTpManager->channelHandler(),
                SIGNAL(textChannelAvailable(QString,Tp::TextChannelPtr)),
                mAccountsModel,
                SLOT(onTextChannelAvailable(QString,Tp::TextChannelPtr)));
        connect(mTpManager->channelHandler(),
                SIGNAL(callChannelAvailable(QString,Tpy::CallChannelPtr)),
                mAccountsModel,
                SLOT(onCallChannelAvailable(QString,Tpy::CallChannelPtr)));
        connect(mTpManager->channelHandler(),
                SIGNAL(incomingFileTransferChannelAvailable(QString,Tp::IncomingFileTransferChannelPtr)),
                mAccountsModel,
                SLOT(onIncomingFileTransferChannelAvailable(QString,Tp::IncomingFileTransferChannelPtr)));
        connect(mTpManager->channelHandler(),
                SIGNAL(outgoingFileTransferChannelAvailable(QString,Tp::OutgoingFileTransferChannelPtr,Tp::ChannelRequestPtr)),
                mAccountsModel,
                SLOT(onOutgoingFileTransferChannelAvailable(QString,Tp::OutgoingFileTransferChannelPtr, Tp::ChannelRequestPtr)));
        connect(mTpManager->channelHandler(),
                SIGNAL(serverAuthChannelAvailable(QString,Tp::ChannelPtr)),
                mAccountsModel,
                SLOT(onServerAuthChannelAvailable(QString,Tp::ChannelPtr)));
        connect(mTpManager->channelHandler(),
                SIGNAL(textChannelAvailable(QString,Tp::TextChannelPtr)),
                mGroupChatModel,
                SLOT(onTextChannelAvailable(QString,Tp::TextChannelPtr)));
    }
}

void Components::onNewAccount(const QString &accountId)
{
    // only do this if not emitted yet
    if (!mLastUsedAccountSignalEmitted) {
        // if the setting is empty, emit now
        if (mLastUsedAccount.isEmpty()) {
            loadLastUsedAccount(mLastUsedAccount, mAccountsModel);
        } else if (accountId == mLastUsedAccount) {
            // only emit if it matches the saved setting
            loadLastUsedAccount(accountId, mAccountsModel);
        }
    }
}

Q_EXPORT_PLUGIN2(components, Components);
