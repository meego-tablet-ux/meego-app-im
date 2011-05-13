/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "panelschannelobserver.h"

#include <TelepathyQt4/Account>
#include <TelepathyQt4/Channel>
#include <TelepathyQt4/ChannelClassSpecList>
#include <TelepathyQt4/PendingReady>

PanelsChannelObserver::PanelsChannelObserver(const Tp::ChannelClassSpecList &channelFilter, bool shouldRecover)
    : AbstractClientObserver(channelFilter, shouldRecover)
{

}

PanelsChannelObserver::~PanelsChannelObserver()
{
}

void PanelsChannelObserver::observeChannels(const Tp::MethodInvocationContextPtr<> &context,
                                            const Tp::AccountPtr &account,
                                            const Tp::ConnectionPtr &connection,
                                            const QList<Tp::ChannelPtr> &channels,
                                            const Tp::ChannelDispatchOperationPtr &dispatchOperation,
                                            const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                                            const ObserverInfo &observerInfo)
{
    Q_UNUSED(connection);
    Q_UNUSED(dispatchOperation);
    Q_UNUSED(requestsSatisfied);
    Q_UNUSED(observerInfo);
    qDebug("PanelsChannelObserver::observeChannels(): New channel to handle");

    foreach (Tp::ChannelPtr channel, channels) {
        Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (!textChannel.isNull()) {
            textChannel->setProperty("accountId", account->uniqueIdentifier());
            mTextChannels.append(textChannel);
            emit newTextChannel(account->uniqueIdentifier(), textChannel);
            continue;
        }


        Tpy::CallChannelPtr callChannel = Tpy::CallChannelPtr::dynamicCast(channel);
        if (!callChannel.isNull()) {
            qDebug("PanelsChannelObserver::observeChannels(): New Call Channel detected");
            callChannel->setProperty("accountId", account->uniqueIdentifier());
            mCallChannels.append(callChannel);
            emit newCallChannel(account->uniqueIdentifier(), callChannel);
            continue;
        }

        Tp::IncomingFileTransferChannelPtr fileTransferChannel = Tp::IncomingFileTransferChannelPtr::dynamicCast(channel);
        if (!fileTransferChannel.isNull()) {
            fileTransferChannel->setProperty("accountId", account->uniqueIdentifier());
            mFileTransferChannels.append(fileTransferChannel);
            emit newFileTransferChannel(account->uniqueIdentifier(), fileTransferChannel);
            continue;
        }
    }

    context->setFinished();
}

void PanelsChannelObserver::emitExistingChannels()
{
    foreach (Tp::TextChannelPtr channel, mTextChannels) {
        if (!channel.isNull() && channel->isValid()) {
            QString accountId = channel->property("accountId").toString();
            emit newTextChannel(accountId, channel);
        }
    }

    foreach (Tpy::CallChannelPtr channel, mCallChannels) {
        if (!channel.isNull() && channel->isValid()) {
            QString accountId = channel->property("accountId").toString();
            emit newCallChannel(accountId, channel);
        }
    }

    foreach (Tp::IncomingFileTransferChannelPtr channel, mFileTransferChannels) {
        if (!channel.isNull() && channel->isValid()) {
            QString accountId = channel->property("accountId").toString();
            emit newFileTransferChannel(accountId, channel);
        }
    }
}
