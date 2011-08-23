/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_session_conversation_model_h_HEADER_GUARD_
#define _Telepathy_Models_session_conversation_model_h_HEADER_GUARD_

#include "global.h"

#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Types>
#include "abstract-conversation-model.h"

#include <QAbstractListModel>

class EventItem;

class TELEPATHY_MODELS_EXPORT SessionConversationModel : public AbstractConversationModel
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

#endif // _Telepathy_Models_session_conversation_model_h_HEADER_GUARD_
