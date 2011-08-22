/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */


#include "simplecontactslistmodel.h"

#include "accounts-model.h"

SimpleContactsListModel::SimpleContactsListModel(const QList<Tp::ContactPtr> contacts, QObject *parent):
    QAbstractListModel(parent)
{
    beginInsertRows(QModelIndex(), 0, contacts.count() - 1);
    // create the contact model items and fill up the list
    foreach(Tp::ContactPtr contact, contacts) {
        ContactModelItem *item = new ContactModelItem(contact);
        mItems.append(item);
    }
    endInsertRows();

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
    roles[AccountsModel::AliasRole] = "aliasName";
    roles[AccountsModel::AvatarRole] = "avatar";
    roles[AccountsModel::PresenceStatusRole] = "presenceStatus";
    roles[AccountsModel::PresenceTypeRole] = "presenceType";
    roles[AccountsModel::PresenceMessageRole] = "presenceMessage";
    roles[AccountsModel::TextChatCapabilityRole] = "textChat";
    roles[AccountsModel::MediaCallCapabilityRole] = "mediaCall";
    roles[AccountsModel::AudioCallCapabilityRole] = "audioCall";
    roles[AccountsModel::VideoCallCapabilityRole] = "videoCall";
    roles[AccountsModel::VideoCallWithAudioCapabilityRole] = "videoCallWithAudio";
    roles[AccountsModel::UpgradeCallCapabilityRole] = "upgradeCall";
    setRoleNames(roles);
}

SimpleContactsListModel::~SimpleContactsListModel()
{

}

int SimpleContactsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mItems.count();
}

QVariant SimpleContactsListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= mItems.count()) {
        return QVariant();
    }

    ContactModelItem *item = mItems[index.row()];

    switch(role) {
        default: {
            return item->data(role);
        }
    }
}
