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

#ifndef _TelepathyQt4Yell_Models_abstract_conversation_model_h_HEADER_GUARD_
#define _TelepathyQt4Yell_Models_abstract_conversation_model_h_HEADER_GUARD_

#ifndef IN_TELEPATHY_QT4_YELL_MODELS_HEADER
#error IN_TELEPATHY_QT4_YELL_MODELS_HEADER
#endif

#include <TelepathyQt4Yell/Models/Global>

#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Types>

#include <QAbstractListModel>

namespace Tpy
{

class EventItem;

class TELEPATHY_QT4_YELL_MODELS_EXPORT AbstractConversationModel : public QAbstractListModel
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

}

#endif // _TelepathyQt4Yell_Models_abstract_conversation_model_h_HEADER_GUARD_
