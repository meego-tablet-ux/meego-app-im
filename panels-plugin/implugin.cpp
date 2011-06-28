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
      m_tpManager(0),
      m_protocolsModel(0),
      m_serviceModel(0)
{
    Tp::registerTypes();
    Tpy::registerTypes();

    qDebug() << "IMPlugin constructor";

    initializeChannelObserver();
    m_protocolsModel = new IMProtocolsModel(this);
    m_tpManager = TelepathyManager::instance();
    m_tpManager->setProtocolNames(m_protocolsModel->protocolNames());

    m_serviceModel = new IMServiceModel(m_tpManager, m_protocolsModel, this);

    connect(m_tpManager, SIGNAL(accountAvailable(Tp::AccountPtr)),
            m_serviceModel, SLOT(onAccountAvailable(Tp::AccountPtr)));

    // install translation catalogs
    loadTranslator();
    qApp->installTranslator(&appTranslator);
}

IMPlugin::~IMPlugin()
{
    qDebug() << "IMPlugin::~IMPlugin() terminating";
    mClientRegistrar->unregisterClients();
    delete m_serviceModel;
    delete m_protocolsModel;
    mClientRegistrar->deleteLater();
    qDebug() << "IMPlugin::~IMPlugin() done terminating";
}

QAbstractItemModel *IMPlugin::serviceModel()
{
    return m_serviceModel;
}

QAbstractItemModel *IMPlugin::createFeedModel(const QString &service)
{
    qDebug() << "IMPlugin::createFeedModel: " << service;
    if(!mObserver) {
        initializeChannelObserver();
    }

    foreach (Tp::AccountPtr account, m_tpManager->accounts()) {
        if (!account.isNull() && account->isValid() && account->uniqueIdentifier() == service) {
            IMFeedModel *model = new IMFeedModel(mObserver, account, this);

            mFeedModels[service] = model;
            return model;
        }
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
    mClientRegistrar->registerClient(observer, observerName);
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
    appTranslator.load("meego-app-im_" + QLocale::system().name() + ".qm",
                       QLibraryInfo::location(QLibraryInfo::TranslationsPath));
}

Q_EXPORT_PLUGIN2(im_panels_plugin, IMPlugin)
