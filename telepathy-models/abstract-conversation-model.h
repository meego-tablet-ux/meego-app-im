/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_abstract_conversation_model_h_HEADER_GUARD_
#define _Telepathy_Models_abstract_conversation_model_h_HEADER_GUARD_

#include "global.h"

#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Types>

#include <QAbstractListModel>

class EventItem;

class TELEPATHY_MODELS_EXPORT AbstractConversationModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractConversationModel)

public:
    enum Role {
        EventTypeRole = Qt::UserRole,
        SenderRole,
        SenderIdRole,
        SenderAvatarRole,
        SenderPresenceTypeRole,
        ReceiverRole,
        ReceiverIdRole,
        ReceiverAvatarRole,
        ReceiverPresenceTypeRole,
        DateTimeRole,
        ItemRole,
        MessageTextRole,
        MessageTypeRole,
        CallDurationRole,
        CallEndActorRole,
        CallEndActorAvatarRole,
        CallEndReasonRole,
        CallDetailedEndReasonRole,
        MissedCallRole,
        RejectedCallRole,
        CustomEventTextRole,
        CustomEventTypeRole
    };

    explicit AbstractConversationModel(QObject *parent = 0);
    virtual ~AbstractConversationModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    void addItem(const EventItem *item);
    void insertItems(QList<const EventItem *> items, int index = 0);
    bool deleteItem(const EventItem *item);

    QModelIndex index(const EventItem *item) const;

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // _Telepathy_Models_abstract_conversation_model_h_HEADER_GUARD_
