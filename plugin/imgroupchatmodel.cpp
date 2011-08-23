/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "imgroupchatmodel.h"

#include "imaccountsmodel.h"
#include "../telepathy-qml-lib/notificationmanager.h"
#include "accounts-model.h"
#include <TelepathyQt4/ReceivedMessage>

IMGroupChatModel::IMGroupChatModel(QObject *parent) :
    QAbstractListModel(parent), mNotificationManager(0)
{
    QHash<int, QByteArray> roles;
    roles[AccountsModel::ItemRole] = "item";
    roles[AccountsModel::IdRole] = "id";
    roles[AccountsModel::ValidRole] = "valid";
    roles[AccountsModel::EnabledRole] = "enabled";
    roles[AccountsModel::ConnectionManagerNameRole] = "connectionManager";
    roles[AccountsModel::ProtocolNameRole] = "protocol";
    roles[AccountsModel::DisplayNameRole] = "displayName";
    roles[AccountsModel::IconRole] = "icon";
    roles[AccountsModel::NicknameRole] = "nickname";
    roles[AccountsModel::ConnectsAutomaticallyRole] = "connectsAutomatically";
    roles[AccountsModel::ChangingPresenceRole] = "changingPresence";
    roles[AccountsModel::AutomaticPresenceRole] = "automaticPresence";
    roles[AccountsModel::CurrentPresenceRole] = "status";
    roles[AccountsModel::CurrentPresenceTypeRole] = "statusType";
    roles[AccountsModel::CurrentPresenceStatusMessageRole] = "statusMessage";
    roles[AccountsModel::RequestedPresenceRole] = "requestedStatus";
    roles[AccountsModel::RequestedPresenceTypeRole] = "requestedStatusType";
    roles[AccountsModel::RequestedPresenceStatusMessageRole] = "requestedStausMessage";
    roles[AccountsModel::ConnectionStatusRole] = "connectionStatus";
    roles[AccountsModel::ConnectionStatusReasonRole] = "connectionStatusReason";
    roles[AccountsModel::AliasRole] = "aliasName";
    roles[AccountsModel::AvatarRole] = "avatar";
    roles[AccountsModel::PresenceStatusRole] = "presenceStatus";
    roles[AccountsModel::PresenceTypeRole] = "presenceType";
    roles[AccountsModel::PresenceMessageRole] = "presenceMessage";
    roles[AccountsModel::SubscriptionStateRole] = "subscriptionState";
    roles[AccountsModel::PublishStateRole] = "publishState";
    roles[AccountsModel::BlockedRole] = "blocked";
    roles[AccountsModel::GroupsRole] = "groups";
    roles[AccountsModel::TextChatCapabilityRole] = "textChat";
    roles[AccountsModel::MediaCallCapabilityRole] = "mediaCall";
    roles[AccountsModel::AudioCallCapabilityRole] = "audioCall";
    roles[AccountsModel::VideoCallCapabilityRole] = "videoCall";
    roles[AccountsModel::VideoCallWithAudioCapabilityRole] = "videoCallWithAudio";
    roles[AccountsModel::UpgradeCallCapabilityRole] = "upgradeCall";
    roles[IMAccountsModel::PendingMessagesRole] = "pendingMessages";
    roles[IMAccountsModel::ChatOpenedRole] = "chatOpened";
    roles[IMAccountsModel::LastPendingMessageRole] = "lastPendingMessage";
    roles[IMAccountsModel::LastPendingMessageSentRole] = "lastPendingMessageSent";
    roles[IMAccountsModel::MissedVideoCallsRole] = "missedVideoCalls";
    roles[IMAccountsModel::MissedAudioCallsRole] = "missedAudioCalls";
    roles[IMAccountsModel::ExistingCallRole] = "existingCall";
    roles[IMAccountsModel::IncomingVideoCallRole] = "incomingVideoCall";
    roles[IMAccountsModel::IncomingAudioCallRole] = "incomingAudioCall";
    roles[IMAccountsModel::ModelDataRole] = "modelData";
    roles[IMAccountsModel::PresenceTypesListRole] = "presenceTypesList";
    roles[IMAccountsModel::AvatarsListRole] = "avatarsList";
    setRoleNames(roles);
}

QVariant IMGroupChatModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= mChildren.count()) {
        return QVariant();
    }

    IMGroupChatModelItem *item = mChildren[index.row()];

    switch(role) {
        default: {
            return item->data(role);
        }
    }
}

int IMGroupChatModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mChildren.count();
}

void IMGroupChatModel::setNotificationManager(NotificationManager *manager)
{
    mNotificationManager = manager;
}

void IMGroupChatModel::onTextChannelAvailable(const QString &accountId, Tp::TextChannelPtr channel)
{
    if(channel->isConference()) {
        // accept the group request
        if(channel->groupLocalPendingContacts().count() > 0) {
            Tp::Contacts contacts = channel->groupLocalPendingContacts();
            foreach(Tp::ContactPtr contact, contacts.values()) {
                if(contact == channel->groupSelfContact()) {
                    channel->groupAddContacts(QList<Tp::ContactPtr>() << contact);
                }
            }
        }

        // add item to model
        IMGroupChatModelItem *item = new IMGroupChatModelItem(accountId, channel);
        connect(item, SIGNAL(changed(IMGroupChatModelItem*)),
                SLOT(onItemChanged(IMGroupChatModelItem*)));
        connect(item, SIGNAL(pendingMessagesChanged()),
                SLOT(onPendingMessagesChanged()));
        connect(channel.data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
                SLOT(onChannelInvalidated()));

        beginInsertRows(QModelIndex(), mChildren.count(), mChildren.count());
        mChildren.append(item);
        endInsertRows();

        // if there are pending messages, notify them
        onPendingMessagesChanged(item);
    }
}

void IMGroupChatModel::onChannelInvalidated()
{
    Tp::Channel *channel = qobject_cast<Tp::Channel *>(sender());
    foreach(IMGroupChatModelItem *item, mChildren) {
        if(channel->objectPath() == item->channel()->objectPath()) {
            beginRemoveRows(QModelIndex(), mChildren.indexOf(item), mChildren.indexOf(item));
            mChildren.removeOne(item);
            delete item;
            endRemoveRows();
        }
    }
}

QModelIndex IMGroupChatModel::index(IMGroupChatModelItem *item)
{
    if(item) {
        return createIndex(mChildren.indexOf(item), 0, item);
    }
    return QModelIndex();
}

void IMGroupChatModel::onItemChanged(IMGroupChatModelItem *item)
{
    QModelIndex itemIndex = index(item);
    if(itemIndex.isValid()) {
        emit dataChanged(itemIndex, itemIndex);
        beginResetModel();
        endResetModel();
    }
}

void IMGroupChatModel::onPendingMessagesChanged()
{

    IMGroupChatModelItem *item = qobject_cast<IMGroupChatModelItem*>(sender());
    if (!item) {
        return;
    }
    onPendingMessagesChanged(item);
}

void IMGroupChatModel::onPendingMessagesChanged(IMGroupChatModelItem *item)
{
    QList<Tp::ReceivedMessage> messages = item->pendingMessages();

    if (!mNotificationManager || !messages.count()) {
        return;
    }
    Tp::ReceivedMessage message = messages.last();

    // do not log old messages
    if (message.isRescued() || message.isScrollback()) {
        return;
    }

    // do not log delivery reports
    if(message.messageType() == Tp::ChannelTextMessageTypeDeliveryReport) {
        return;
    }

    mNotificationManager->notifyPendingGroupMessage(item->accountId(),
                                                    item->channel()->objectPath(),
                                                    message.sender()->alias(),
                                                    message.sent(),
                                                    message.text());
}
