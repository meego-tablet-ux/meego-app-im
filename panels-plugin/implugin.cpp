/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <QtPlugin>

#include "implugin.h"

#include "imfeedmodelfilter.h"
#include "imfeedproxymodel.h"
#include "imservmodel.h"

#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/ChannelClassSpecList>
#include <TelepathyQt4/Types>
#include <TelepathyQt4/Debug>
#include <TelepathyQt4Yell/CallChannel>
#include <TelepathyQt4Yell/ChannelClassSpec>

IMPlugin::IMPlugin(QObject *parent)
    : QObject(parent),
      McaFeedPlugin(),
      mTpManager(0),
      mProtocolsModel(0),
      mServiceModel(0)
{
    Tp::registerTypes();
    Tpy::registerTypes();

    qDebug() << "IMPlugin constructor";

    initializeChannelObserver();
    mProtocolsModel = new IMProtocolsModel(this);
    mTpManager = TelepathyManager::instance();
    mTpManager->setProtocolNames(mProtocolsModel->protocolNames());

    mServiceModel = new IMServiceModel(mTpManager, mProtocolsModel, this);

    connect(mTpManager, SIGNAL(accountAvailable(Tp::AccountPtr)),
            SLOT(onAccountAvailable(Tp::AccountPtr)));
    connect(mTpManager, SIGNAL(accountAvailable(Tp::AccountPtr)),
            mServiceModel, SLOT(onAccountAvailable(Tp::AccountPtr)));

    // install translation catalogs
    loadTranslator();
    qApp->installTranslator(&mAppTranslator);
}

IMPlugin::~IMPlugin()
{
    qDebug() << "IMPlugin::~IMPlugin() terminating";
    mClientRegistrar->unregisterClients();
    mClientRegistrar->deleteLater();
    qDebug() << "IMPlugin::~IMPlugin() done terminating";
}

QAbstractItemModel *IMPlugin::serviceModel()
{
    return mServiceModel;
}

QAbstractItemModel *IMPlugin::createFeedModel(const QString &service)
{
    qDebug() << "IMPlugin::createFeedModel: " << service;
    if(!mObserver) {
        initializeChannelObserver();
    }

    // create the model if not present
    if (!mFeedModels.contains(service)) {
        foreach (Tp::AccountPtr account, mTpManager->accounts()) {
            if (!account.isNull() && account->isValid()
                    && account->uniqueIdentifier() == service) {
                onAccountAvailable(account);
                break;
            }
        }
    }

    // check once again, just in case the requested service is not a valid account id
    if (mFeedModels.contains(service)) {
        IMFeedProxyModel *proxyModel = new IMFeedProxyModel(mFeedModels[service], this);
        return proxyModel;
    }

    qDebug() << "IMPlugin::createFeedModel: Invalid service requested: " << service;
    return 0;
}

void IMPlugin::initializeChannelObserver()
{
    mClientRegistrar = Tp::ClientRegistrar::create();

    // setup the channel filters
    Tp::ChannelClassSpecList channelSpecList;

    channelSpecList.append(Tp::ChannelClassSpec::textChat());
    channelSpecList.append(Tp::ChannelClassSpec::textChatroom());
    channelSpecList.append(Tp::ChannelClassSpec::unnamedTextChat());
    channelSpecList.append(Tpy::ChannelClassSpec::mediaCall());
    channelSpecList.append(Tpy::ChannelClassSpec::audioCall());
    channelSpecList.append(Tpy::ChannelClassSpec::videoCall());
    channelSpecList.append(Tpy::ChannelClassSpec::videoCallWithAudio());
    channelSpecList.append(Tp::ChannelClassSpec::incomingFileTransfer());
    channelSpecList.append(Tp::ChannelClassSpec::outgoingFileTransfer());

    // create the channel observer
    mObserver = new PanelsChannelObserver(channelSpecList, true);
    Tp::AbstractClientPtr observer(mObserver);

    // register the observer
    // add the date and time to make sure it is unique
    QString observerName = QString("MeeGoIMPanelsObserver") +  QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    qDebug() << "IMPlugin::initizalizeChannelObserver: observer name is " << observerName;
    if(!mClientRegistrar->registerClient(observer, observerName)) {
        qDebug() << "IMPlugin::initizalizeChannelObserver: reattempting to register observer after error. Changing name in case there was a conflict";
        observerName += "B";
        if(!mClientRegistrar->registerClient(observer, observerName)) {
            qDebug() << "IMPlugin::initizalizeChannelObserver: error after second attempt to register observer " << observerName;
        }
    }
}

McaSearchableFeed *IMPlugin::createSearchModel(const QString &service, const QString &searchText)
{
    qDebug() << "IMPlugin::createSearchModel: " << service << "searchText: " << searchText;

    if (mFeedModels.contains(service)) {
        IMFeedModelFilter *filter = new IMFeedModelFilter(mFeedModels[service], this);
        filter->setSearchText(searchText);
        return filter;
    }
    return 0;
}

void IMPlugin::loadTranslator()
{
    mAppTranslator.load("meego-app-im_" + QLocale::system().name() + ".qm",
                       QLibraryInfo::location(QLibraryInfo::TranslationsPath));
}

void IMPlugin::onAccountAvailable(Tp::AccountPtr account)
{
    connect(account.data(), SIGNAL(removed()),
            SLOT(onAccountRemoved()));

    QString accountId = account->uniqueIdentifier();
    if (!mFeedModels.contains(accountId)) {
        IMFeedModel *model = new IMFeedModel(mObserver, account, this);
        mFeedModels[accountId] = model;
    }
}

void IMPlugin::onAccountRemoved()
{
    Tp::Account *account = qobject_cast<Tp::Account *>(sender());

    // make sure we are not acting on a null object
    if (account) {
        QString id = account->uniqueIdentifier();
        if (mFeedModels.contains(id)) {
            IMFeedModel *model = mFeedModels.value(id);
            delete model;
            mFeedModels.remove(id);
        }
    }
}

Q_EXPORT_PLUGIN2(im_panels_plugin, IMPlugin)
