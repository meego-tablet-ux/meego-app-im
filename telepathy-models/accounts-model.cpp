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

#include <TelepathyQt4Yell/Models/AccountsModel>

#include "TelepathyQt4Yell/Models/_gen/accounts-model.moc.hpp"

#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingReady>

#include <TelepathyQt4Yell/Models/AccountsModelItem>
#include <TelepathyQt4Yell/Models/ContactModelItem>

namespace Tpy
{

struct TELEPATHY_QT4_YELL_MODELS_NO_EXPORT AccountsModel::Private
{
    Private(const Tp::AccountManagerPtr &am)
    {
        mAccountSet = am->validAccounts();
    }

    Private(const Tp::AccountSetPtr &accountSet)
        : mAccountSet(accountSet)
    {
    }

    TreeNode *node(const QModelIndex &index) const;

    Tp::AccountSetPtr mAccountSet;
    TreeNode *mTree;
};

TreeNode *AccountsModel::Private::node(const QModelIndex &index) const
{
    TreeNode *node = reinterpret_cast<TreeNode *>(index.internalPointer());
    return node ? node : mTree;
}

/**
 * \class AccountsModel
 * \ingroup models
 * \headerfile TelepathyQt4Yell/accounts-model.h <TelepathyQt4Yell/AccountsModel>
 *
 * \brief The accounts model represents a tree of the valid accounts and their contacts
 *
 * AccountsModel holds the valid Telepathy accounts and the valid contacts of those accounts.
 * It will update automatically when accounts and contacts are added or removed, or when accounts go
 * offline or online.
 *
 */

/**
  * Construct an AccountsModel object
  * \param accountSet an AccountSet with the accounts that will be represented in the model
  * \param parent parent object
  */
AccountsModel::AccountsModel(const Tp::AccountSetPtr &accountSet, QObject *parent)
    : QAbstractItemModel(parent),
      mPriv(new AccountsModel::Private(accountSet))
{
    initialize();
}

/**
  * Construct an AccountsModel object. This is a convenience method.
  * \param am an AccountManager. An AccountSet will be extracted by calling am->validAccounts().
  * \param parent parent object
  */
AccountsModel::AccountsModel(const Tp::AccountManagerPtr &am, QObject *parent)
    : QAbstractItemModel(parent),
      mPriv(new AccountsModel::Private(am))
{
    initialize();
}

AccountsModel::~AccountsModel()
{
    delete mPriv->mTree;
    delete mPriv;
}

void AccountsModel::initialize()
{
    mPriv->mTree = new TreeNode;
    connect(mPriv->mTree,
            SIGNAL(changed(Tpy::TreeNode*)),
            SLOT(onItemChanged(Tpy::TreeNode*)));
    connect(mPriv->mTree,
            SIGNAL(childrenAdded(Tpy::TreeNode*,QList<Tpy::TreeNode*>)),
            SLOT(onItemsAdded(Tpy::TreeNode*,QList<Tpy::TreeNode*>)));
    connect(mPriv->mTree,
            SIGNAL(childrenRemoved(Tpy::TreeNode*,int,int)),
            SLOT(onItemsRemoved(Tpy::TreeNode*,int,int)));
    connect(mPriv->mAccountSet.data(),
            SIGNAL(accountAdded(const Tp::AccountPtr)),
            SLOT(onNewAccount(Tp::AccountPtr)));

    QHash<int, QByteArray> roles;
    roles[ItemRole] = "item";
    roles[IdRole] = "id";
    roles[ValidRole] = "valid";
    roles[EnabledRole] = "enabled";
    roles[ConnectionManagerNameRole] = "connectionManager";
    roles[ProtocolNameRole] = "protocol";
    roles[ServiceNameRole] = "service";
    roles[DisplayNameRole] = "displayName";
    roles[IconRole] = "icon";
    roles[NicknameRole] = "nickname";
    roles[ConnectsAutomaticallyRole] = "connectsAutomatically";
    roles[ChangingPresenceRole] = "changingPresence";
    roles[AutomaticPresenceRole] = "automaticStatus";
    roles[AutomaticPresenceTypeRole] = "automaticStatusType";
    roles[AutomaticPresenceStatusMessageRole] = "automaticStatusMessage";
    roles[CurrentPresenceRole] = "status";
    roles[CurrentPresenceTypeRole] = "statusType";
    roles[CurrentPresenceStatusMessageRole] = "statusMessage";
    roles[RequestedPresenceRole] = "requestedStatus";
    roles[RequestedPresenceTypeRole] = "requestedStatusType";
    roles[RequestedPresenceStatusMessageRole] = "requestedStatusMessage";
    roles[ConnectionStatusRole] = "connectionStatus";
    roles[ConnectionStatusReasonRole] = "connectionStatusReason";
    roles[ContactListStateRole] = "contactListState";
    roles[AliasRole] = "aliasName";
    roles[AvatarRole] = "avatar";
    roles[PresenceStatusRole] = "presenceStatus";
    roles[PresenceTypeRole] = "presenceType";
    roles[PresenceMessageRole] = "presenceMessage";
    roles[SubscriptionStateRole] = "subscriptionState";
    roles[PublishStateRole] = "publishState";
    roles[BlockedRole] = "blocked";
    roles[GroupsRole] = "groups";
    roles[TextChatCapabilityRole] = "textChat";
    roles[StreamedMediaCallCapabilityRole] = "streamedMediaCall";
    roles[StreamedMediaAudioCallCapabilityRole] = "streamedMediaAudioCall";
    roles[StreamedMediaVideoCallCapabilityRole] = "streamedMediaVideoCall";
    roles[StreamedMediaVideoCallWithAudioCapabilityRole] = "streamedMediaVideoCallWithAudio";
    roles[StreamedMediaUpgradeCallCapabilityRole] = "streamedMediaUpgradeCall";
    roles[MediaCallCapabilityRole] = "mediaCall";
    roles[AudioCallCapabilityRole] = "audioCall";
    roles[VideoCallCapabilityRole] = "videoCall";
    roles[VideoCallWithAudioCapabilityRole] = "videoCallWithAudio";
    roles[UpgradeCallCapabilityRole] = "upgradeCall";
    roles[FileTransferCapabilityRole] = "fileTransfer";
    setRoleNames(roles);

    QTimer::singleShot(0, this, SLOT(onLoadAccounts()));
}

/**
  * Get each account in the internally stored AccountSet object and
  * call onNewAccount to add the accounts to the model
  */
void AccountsModel::onLoadAccounts()
{
    // load existing accounts
    foreach (Tp::AccountPtr account, mPriv->mAccountSet->accounts()) {
        onNewAccount(account);
    }
}

/**
  * Add the account to the model
  * \param account a AccountPtr object that will be represented in the model
  */
void AccountsModel::onNewAccount(const Tp::AccountPtr &account)
{
    AccountsModelItem *item = new AccountsModelItem(account);
    connect(item, SIGNAL(connectionStatusChanged(QString,int)),
            this, SIGNAL(accountConnectionStatusChanged(QString,int)));
    onItemsAdded(mPriv->mTree, QList<TreeNode *>() << item);
    emit newAccountItem(account->uniqueIdentifier());
}

/**
  * emit a dataChanged signal for the tree node
  * \param node TreeNode object that changed
  */
void AccountsModel::onItemChanged(TreeNode *node)
{
    QModelIndex accountIndex = index(node);
    emit dataChanged(accountIndex, accountIndex);
}

/**
  * When items are added to the tree, this method, calls beginInsertRows and endInsertRows
  * to inform the changes. The account count is also refreshed.
  * \param parent model item parent of the nodes
  * \param nodes list of nodes added to the model
  */
void AccountsModel::onItemsAdded(Tpy::TreeNode *parent, const QList<Tpy::TreeNode *> &nodes)
{
    QModelIndex parentIndex = index(parent);
    int currentSize = rowCount(parentIndex);
    //debug() << "adding rows from" << currentSize << "to" << (currentSize + nodes.size() - 1);
    beginInsertRows(parentIndex, currentSize, currentSize + nodes.size() - 1);
    foreach (TreeNode *node, nodes) {
        parent->addChild(node);
    }
    endInsertRows();
    emit accountCountChanged();
}

/**
  * When items are removed from the tree, this method, calls beginRemoeRows and endRemoveRows
  * to inform the changes. The account count is also refreshed.
  * \param parent model item parent of the nodes
  * \param nodes list of nodes removed from the model
  */
void AccountsModel::onItemsRemoved(TreeNode *parent, int first, int last)
{
    QModelIndex parentIndex = index(parent);
    QList<TreeNode *> removedItems;
    beginRemoveRows(parentIndex, first, last);
    for (int i = last; i >= first; i--) {
        parent->childAt(i)->remove();
    }
    endRemoveRows();
    emit accountCountChanged();
}

/**
  * Quantity of accounts in the model
  */
int AccountsModel::accountCount() const
{
    return mPriv->mTree->size();
}

/**
  * Returns an account model item that matches a given account id.
  * It will return a null object if there is no match.
  * \param id an AccountPtr uniqueIdentifier that should match an account in the model.
  */
QObject *AccountsModel::accountItemForId(const QString &id) const
{
    for (int i = 0; i < mPriv->mTree->size(); ++i) {
        AccountsModelItem *item = qobject_cast<AccountsModelItem*>(mPriv->mTree->childAt(i));
        if (!item) {
            continue;
        }

        if (item->data(IdRole) == id) {
            return item;
        }
    }

    return 0;
}

/**
  * Returns a contact model item that matches a given account and contact id.
  * It will return a null object if there is no match.
  * \param accountId an AccountPtr uniqueIdentifier that should match an account in the model.
  * \param contactId a ContactPtr id that should match a contact in the account
  */
QObject *AccountsModel::contactItemForId(const QString &accountId, const QString &contactId) const
{
    AccountsModelItem *accountItem = qobject_cast<AccountsModelItem*>(accountItemForId(accountId));
    if (!accountItem) {
        return 0;
    }

    for (int i = 0; i < accountItem->size(); ++i) {
        ContactModelItem *item = qobject_cast<ContactModelItem*>(accountItem->childAt(i));
        if (!item) {
            continue;
        }

        if (item->data(IdRole) == contactId) {
            return item;
        }
    }

    return 0;
}

/**
  * This will always return 1
  */
int AccountsModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

/**
  * It returns the number of nodes for a given node.
  * \param parent The parent node
  */
int AccountsModel::rowCount(const QModelIndex &parent) const
{
    return mPriv->node(parent)->size();
}

/**
  * Returns data for an index and role. To retrieve the data, the node of the index is found and
  * the corresponding data method is called.
  * If the index or role is invalid, an empty QVariant is returned
  * \param index index of the node
  * \param role a valid role for the node
  */
QVariant AccountsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    return mPriv->node(index)->data(role);
}

/**
  * It returns an AccountPtr object for a given index
  * \param index the account model item index
  */
Tp::AccountPtr AccountsModel::accountForIndex(const QModelIndex &index) const
{
    TreeNode *accountNode = mPriv->node(index);
    AccountsModelItem *item = qobject_cast<AccountsModelItem *>(accountNode);
    if (item) {
        return item->account();
    } else {
        return Tp::AccountPtr();
    }
}

/**
  * It returns an AccountPtr object for a given contact index
  * \param index the contact model item index
  */
Tp::AccountPtr AccountsModel::accountForContactIndex(const QModelIndex &index) const
{
    TreeNode *contactNode = mPriv->node(index);
    AccountsModelItem *item = qobject_cast<AccountsModelItem *>(contactNode->parent());
    if (item) {
        return item->account();
    } else {
        return Tp::AccountPtr();
    }
}

/**
  * It returns an ContactPtr object for a given index
  * \param index the contact model item index
  */
Tp::ContactPtr AccountsModel::contactForIndex(const QModelIndex& index) const
{
    TreeNode *contactNode = mPriv->node(index);
    ContactModelItem *item = qobject_cast<ContactModelItem *>(contactNode);
    if (item) {
        return item->contact();
    } else {
        return Tp::ContactPtr();
    }
}

/**
  * It returns the item flags for a given index
  * \param index the model item index
  */
Qt::ItemFlags AccountsModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

/**
  * Set data on the model items
  * \param index the model item index
  * \param value the value to be set
  * \param role the role that the data will be set in
  */
bool AccountsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid()) {
        return mPriv->node(index)->setData(role, value);
    }

    return false;
}

QModelIndex AccountsModel::index(int row, int column, const QModelIndex &parent) const
{
    TreeNode *parentNode = mPriv->node(parent);
    return createIndex(row, column, parentNode->childAt(row));
}

/**
  * It returns the index of the given node
  * \param node the tree node
  */
QModelIndex AccountsModel::index(TreeNode *node) const
{
    if (node->parent()) {
        return createIndex(node->parent()->indexOf(node), 0, node);
    }
    else {
        return QModelIndex();
    }
}

QModelIndex AccountsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    TreeNode *currentNode = mPriv->node(index);
    if (currentNode->parent()) {
        return AccountsModel::index(currentNode->parent());
    } else {
        // no parent: return root node
        return QModelIndex();
    }
}

/**
  * It returns an AccountPtr object for a given contact model item
  * \param contactItem The contact model item for which the account object will be returned
  */
Tp::AccountPtr AccountsModel::accountForContactItem(ContactModelItem *contactItem) const
{
    AccountsModelItem *accountItem = qobject_cast<AccountsModelItem*>(contactItem->parent());
    if (accountItem) {
        return accountItem->account();
    } else {
        return Tp::AccountPtr();
    }
}

}
