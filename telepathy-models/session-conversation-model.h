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

#ifndef _TelepathyQt4Yell_Models_session_conversation_model_h_HEADER_GUARD_
#define _TelepathyQt4Yell_Models_session_conversation_model_h_HEADER_GUARD_

#ifndef IN_TELEPATHY_QT4_YELL_MODELS_HEADER
#error IN_TELEPATHY_QT4_YELL_MODELS_HEADER
#endif

#include <TelepathyQt4Yell/Models/Global>

#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Types>
#include <TelepathyQt4Yell/Models/AbstractConversationModel>

#include <QAbstractListModel>

namespace Tpy
{

class EventItem;

class TELEPATHY_QT4_YELL_MODELS_EXPORT SessionConversationModel : public AbstractConversationModel
{
    Q_OBJECT
    Q_DISABLE_COPY(SessionConversationModel)
    Q_PROPERTY(int numPendingMessages READ numPendingMessages NOTIFY numPendingMessagesChanged)

public:
    explicit SessionConversationModel(const Tp::ContactPtr &self, const Tp::TextChannelPtr &channel, QObject *parent = 0);
    virtual ~SessionConversationModel();

    Q_INVOKABLE void sendMessage(const QString &text, Tp::ChannelTextMessageType type=Tp::ChannelTextMessageTypeNormal, Tp::MessageSendingFlags flags=0);
    Q_INVOKABLE void disconnectChannelQueue();
    Q_INVOKABLE void connectChannelQueue();
    Q_INVOKABLE bool channelQueueConnected() const;
    int numPendingMessages() const;

    Tp::ContactPtr selfContact() const;

Q_SIGNALS:
    void numPendingMessagesChanged();

protected Q_SLOTS:
    virtual void onChatStateChanged(const Tp::ContactPtr &contact, Tp::ChannelChatState state);

private Q_SLOTS:
    void onMessageReceived(const Tp::ReceivedMessage &message);

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

}

#endif // _TelepathyQt4Yell_Models_session_conversation_model_h_HEADER_GUARD_
