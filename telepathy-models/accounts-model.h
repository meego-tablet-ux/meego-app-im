/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_accounts_model_h_HEADER_GUARD_
#define _Telepathy_Models_accounts_model_h_HEADER_GUARD_

#include "accounts-model-item.h"
#include "contact-model-item.h"

#include <TelepathyQt4/Account>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/AccountSet>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Types>

#include <QAbstractListModel>

class TELEPATHY_MODELS_EXPORT AccountsModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountsModel)
    Q_PROPERTY(int accountCount READ accountCount NOTIFY accountCountChanged)
    Q_ENUMS(Role)

public:
    enum Role {
        // general roles
        ItemRole = Qt::UserRole,
        AvatarRole,
        IdRole,
        TextChatCapabilityRole,
        StreamedMediaCallCapabilityRole,
        StreamedMediaAudioCallCapabilityRole,
        StreamedMediaVideoCallCapabilityRole,
        StreamedMediaVideoCallWithAudioCapabilityRole,
        StreamedMediaUpgradeCallCapabilityRole,
        MediaCallCapabilityRole,
        AudioCallCapabilityRole,
        VideoCallCapabilityRole,
        VideoCallWithAudioCapabilityRole,
        UpgradeCallCapabilityRole,
        FileTransferCapabilityRole,

        // account roles
        ValidRole,
        EnabledRole,
        ConnectionManagerNameRole,
        ProtocolNameRole,
        ServiceNameRole,
        DisplayNameRole,
        IconRole,
        NicknameRole,
        ConnectsAutomaticallyRole,
        ChangingPresenceRole,
        AutomaticPresenceRole,
        AutomaticPresenceTypeRole,
        AutomaticPresenceStatusMessageRole,
        CurrentPresenceRole,
        CurrentPresenceTypeRole,
        CurrentPresenceStatusMessageRole,
        RequestedPresenceRole,
        RequestedPresenceTypeRole,
        RequestedPresenceStatusMessageRole,
        ConnectionStatusRole,
        ConnectionStatusReasonRole,
        ContactListStateRole,

        // contact roles
        AliasRole,
        PresenceStatusRole,
        PresenceTypeRole,
        PresenceMessageRole,
        SubscriptionStateRole,
        PublishStateRole,
        BlockedRole,
        GroupsRole,

        CustomRole // a placemark for custom roles in inherited models
    };

    explicit AccountsModel(const Tp::AccountSetPtr &accountSet, QObject *parent = 0);
    explicit AccountsModel(const Tp::AccountManagerPtr &am, QObject *parent = 0);
    virtual ~AccountsModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    Tp::AccountPtr accountForIndex(const QModelIndex &index) const;
    Tp::AccountPtr accountForContactIndex(const QModelIndex &index) const;
    Tp::ContactPtr contactForIndex(const QModelIndex &index) const;
    Tp::AccountPtr accountForContactItem(ContactModelItem* contactItem) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(TreeNode *node) const;
    virtual QModelIndex parent(const QModelIndex &index) const;

    int accountCount() const;
    Q_INVOKABLE QObject *accountItemForId(const QString &id) const;
    Q_INVOKABLE QObject *contactItemForId(const QString &accountId, const QString &contactId) const;

Q_SIGNALS:
    void accountCountChanged();
    void accountConnectionStatusChanged(const QString &accountId, int status);
    // expensive operation, updates all children from parents first .. last
    void hierarchicalDataChanged(const QModelIndex &first, const QModelIndex &last);
    void newAccountItem(const QString &accountId);

protected Q_SLOTS:
    void onLoadAccounts();
    void onNewAccount(const Tp::AccountPtr &account);
    void onItemChanged(TreeNode *node);
    virtual void onItemsAdded(TreeNode *parent, const QList<TreeNode *> &nodes);
    void onItemsRemoved(TreeNode *parent, int first, int last);

protected:
    void initialize();

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // _Telepathy_Models_accounts_model_h_HEADER_GUARD_
