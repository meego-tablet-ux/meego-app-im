/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_accounts_model_item_h_HEADER_GUARD_
#define _Telepathy_Models_accounts_model_item_h_HEADER_GUARD_

#include <TelepathyQt4Yell/ConnectionCapabilities>
#include "tree-node.h"

#include <TelepathyQt4/Account>
#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Types>

class TELEPATHY_MODELS_EXPORT AccountsModelItem : public TreeNode
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountsModelItem)

public:
    AccountsModelItem(const Tp::AccountPtr &account);
    virtual ~AccountsModelItem();

    Q_INVOKABLE virtual QVariant data(int role) const;
    virtual bool setData(int role, const QVariant &value);
    Q_INVOKABLE Tp::AccountPtr account() const;

    void setEnabled(bool value);

    Q_INVOKABLE void setNickname(const QString &value);

    Q_INVOKABLE void setAutomaticPresence(int type, const QString &status, const QString &statusMessage);
    Q_INVOKABLE void setRequestedPresence(int type, const QString &status, const QString &statusMessage);
    Q_INVOKABLE bool isPresenceSupported(int type) const;

Q_SIGNALS:
    void serviceNameChanged(const QString &serviceName);
    void profileChanged(const Tp::ProfilePtr &profile);
    void displayNameChanged(const QString &displayName);
    void iconNameChanged(const QString &iconName);
    void nicknameChanged(const QString &nickname);
    void normalizedNameChanged(const QString &normalizedName);
    void validityChanged(bool validity);
    void stateChanged(bool state);
    void capabilitiesChanged(const Tpy::ConnectionCapabilities &capabilities);
    void connectsAutomaticallyPropertyChanged(bool connectsAutomatically);
    void firstOnline();
    void parametersChanged(const QVariantMap &parameters);
    void changingPresence(bool value);
    void automaticPresenceChanged(const Tp::Presence &automaticPresence);
    void currentPresenceChanged(const Tp::Presence &currentPresence);
    void requestedPresenceChanged(const Tp::Presence &requestedPresence);
    void onlinenessChanged(bool online);
    void avatarChanged(const Tp::Avatar &avatar);
    void connectionStatusChanged(const QString &accountId, int status);

private Q_SLOTS:
    void startup();
    void onRemoved();
    void onChanged();

    void onConnectionChanged(const Tp::ConnectionPtr &connection);
    void onConnectionInvalidated();
    void onStatusChanged(Tp::ConnectionStatus status);
    void onCapabilitiesChanged();

    void onContactManagerStateChanged(Tp::ContactListState state);
    void onContactsChanged(const Tp::Contacts &added,
                           const Tp::Contacts &removed);

private:
    void clearContacts();
    void addKnownContacts();

    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // _Telepathy_Models_accounts_model_item_h_HEADER_GUARD_
