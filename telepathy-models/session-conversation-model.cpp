/*
 * This file is part of TelepathyQt4Yell Models
 *
 * Copyright (C) 2010 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <TelepathyQt4Yell/Models/SessionConversationModel>

#include "TelepathyQt4Yell/Models/_gen/session-conversation-model.moc.hpp"

#include <TelepathyQt4Yell/Models/TextEventItem>
#include <TelepathyQt4Yell/Models/CustomEventItem>

#include <TelepathyQt4/AvatarData>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/ReceivedMessage>

#include <QPixmap>
#include <QtAlgorithms>

namespace Tpy
{

struct TELEPATHY_QT4_YELL_MODELS_NO_EXPORT SessionConversationModel::Private
{
    Private(const Tp::ContactPtr &self, const Tp::TextChannelPtr &channel)
        : mSelf(self),
          mChannel(channel),
          mChannelQueueConnected(false),
          mNumPendingMessages(0)
    {
    }

    Tp::ContactPtr mSelf;
    Tp::TextChannelPtr mChannel;
    bool mChannelQueueConnected;
    int mNumPendingMessages;
};

/**
 * \class SessionConversationModel
 * \ingroup models
 * \headerfile TelepathyQt4Yell/session-conversation-model.h <TelepathyQt4Yell/SessionConversationModel>
 *
 * \brief A model for a text chat conversation
 *
 */

/**
  * Construct a SessionConversationModel object
  * \param self The contact on the local end of the conversation
  * \param channel The text channel of the conversation
  * \param parent the parent object
  */
SessionConversationModel::SessionConversationModel(const Tp::ContactPtr &self, const Tp::TextChannelPtr &channel, QObject *parent)
    : AbstractConversationModel(parent),
      mPriv(new Private(self,channel))
{
    connect(mPriv->mChannel.data(),
            SIGNAL(messageReceived(Tp::ReceivedMessage)),
            SLOT(onMessageReceived(Tp::ReceivedMessage)));
    connect(mPriv->mChannel.data(),
            SIGNAL(chatStateChanged(Tp::ContactPtr,Tp::ChannelChatState)),
            SLOT(onChatStateChanged(Tp::ContactPtr,Tp::ChannelChatState)));
}

SessionConversationModel::~SessionConversationModel()
{
    delete mPriv;
}

/**
  * Send a message using the text channel and add an EventItem object to the model
  * For more info, see the Tp::TextChannel:sendMessage documentation
  */
void SessionConversationModel::sendMessage(const QString &text, Tp::ChannelTextMessageType type, Tp::MessageSendingFlags flags)
{
    Tp::ContactPtr receiver;
    if (!mPriv->mChannel.isNull() &&
        mPriv->mChannel->targetHandle() != 0 &&
        !mPriv->mChannel->connection().isNull() &&
        !mPriv->mChannel->connection()->contactManager().isNull()) {
        uint handle = mPriv->mChannel->targetHandle();
        receiver = mPriv->mChannel->connection()->contactManager()->lookupContactByHandle(handle);
    }
    TextEventItem *item = new TextEventItem(mPriv->mSelf, receiver,
        QDateTime::currentDateTime(), text, type, this);
    addItem(item);

    mPriv->mChannel->send(item->messageText(), type, flags);
}

/**
  * Returns the contact of the local end of the conversation
  * \param parent the parent object
  */
Tp::ContactPtr SessionConversationModel::selfContact() const
{
    return mPriv->mSelf;
}

void SessionConversationModel::onMessageReceived(const Tp::ReceivedMessage &message)
{
    if (mPriv->mChannelQueueConnected) {
        // TODO: For the moment skip if the message is a delivery report
        // Later they could be used to report status on sent messages
        if (message.messageType() != Tp::ChannelTextMessageTypeDeliveryReport) {
            TextEventItem *item = new TextEventItem(message.sender(), mPriv->mSelf,
                message.sent(), message.text(), message.messageType(), this);
            addItem(item);
        }
        mPriv->mChannel->acknowledge(QList<Tp::ReceivedMessage>() << message);
    } else {
        emit numPendingMessagesChanged();
    }
}

/**
  * When the chat state of the channel changes, if it was changed by a contact on the remote end,
  * a custom event item is added to the model with the data of the change
  * \param contact The contact that requested the chat state change
  * \param state The current chat state
  */
void SessionConversationModel::onChatStateChanged(const Tp::ContactPtr &contact, Tp::ChannelChatState state)
{
    // ignore events originating from self
    if (contact == mPriv->mSelf) {
        return;
    }

    if (state == Tp::ChannelChatStateGone) {
        QString message = QString::fromLatin1("%1 left the chat").arg(contact->alias());
        CustomEventItem *item = new CustomEventItem(contact, mPriv->mSelf,
            QDateTime::currentDateTime(), message, CustomEventItem::CustomEventUserLeftChat, this);
        addItem(item);
    }
}

/**
  * Disconnect the model from the channel messages queue so that messages on the queue will not
  * be acknowledged and entered into the model automatically.
  * This is useful when a conversation is paused, and the application may want to show messages still
  * unread by the user, eg. the messages still in the queue
  */
void SessionConversationModel::disconnectChannelQueue()
{
    mPriv->mChannelQueueConnected = false;
}

/**
  * Reconnect the model to the channel queue and acknowledge messages on the queue
  */
void SessionConversationModel::connectChannelQueue()
{
    mPriv->mChannelQueueConnected = true;

    // flush the queue and enter all messages into the model
    // display messages already in queue
    bool messagesReceived = false;
    foreach (Tp::ReceivedMessage message, mPriv->mChannel->messageQueue()) {
        onMessageReceived(message);
        messagesReceived = true;
    }

    if (messagesReceived) {
        qDebug() << "emiting numPendingMessagesChanged";
        emit numPendingMessagesChanged();
    }
}

/**
  * Returns whether the channel message queue is currently connected to the model.
  * If connected, incoming messages will be automatically added to model
  */
bool SessionConversationModel::channelQueueConnected() const
{
    return mPriv->mChannelQueueConnected;
}

/**
  * Number of messages in the channel message queue
  */
int SessionConversationModel::numPendingMessages() const
{
    if (!mPriv->mChannelQueueConnected && !mPriv->mChannel.isNull()) {
        return mPriv->mChannel->messageQueue().count();
    }

    return 0;
}

}
