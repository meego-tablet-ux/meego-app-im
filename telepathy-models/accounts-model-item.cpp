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

#include <TelepathyQt4Yell/Models/AccountsModelItem>

#include "TelepathyQt4Yell/Models/_gen/accounts-model-item.moc.hpp"

#include <TelepathyQt4Yell/Models/AccountsModel>
#include <TelepathyQt4Yell/Models/AvatarImageProvider>
#include <TelepathyQt4Yell/Models/ContactModelItem>

#include <TelepathyQt4/Account>
#include <TelepathyQt4/ContactManager>

namespace Tpy
{

struct TELEPATHY_QT4_YELL_MODELS_NO_EXPORT AccountsModelItem::Private
{
    Private(const Tp::AccountPtr &account)
        : mAccount(account),
          mAccountCaps(mAccount->capabilities().allClassSpecs())
    {
    }

    void setStatus(const QString &value);
    void setStatusMessage(const QString &value);

    Tp::AccountPtr mAccount;
    Tp::ContactManagerPtr mManager;
    Tpy::ConnectionCapabilities mAccountCaps;
};

void AccountsModelItem::Private::setStatus(const QString &value)
{
    Tp::Presence presence = mAccount->currentPresence();
    presence.setStatus(presence.type(), value, presence.statusMessage());
    mAccount->setRequestedPresence(presence);
}

void AccountsModelItem::Private::setStatusMessage(const QString &value)
{
    Tp::Presence presence = mAccount->currentPresence();
    presence.setStatus(presence.type(), presence.status(), value);
    mAccount->setRequestedPresence(presence);
}

/**
 * \class AccountsModelItem
 * \ingroup models
 * \headerfile TelepathyQt4Yell/accounts-model-item.h <TelepathyQt4Yell/AccountsModelItem>
 *
 * \brief This is a model item that represents a Telepathy Account
 *
 */

/**
  * Construct an AccountsModelItem object
  * \param account a Tp::AccountPtr object that references the Telepathy account
  */
AccountsModelItem::AccountsModelItem(const Tp::AccountPtr &account)
    : mPriv(new Private(account))
{
    connect(mPriv->mAccount.data(),
            SIGNAL(removed()),
            SLOT(onRemoved()));
    connect(mPriv->mAccount.data(),
            SIGNAL(serviceNameChanged(QString)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(serviceNameChanged(QString)),
            SIGNAL(serviceNameChanged(QString)));
    connect(mPriv->mAccount.data(),
            SIGNAL(profileChanged(Tp::ProfilePtr)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(profileChanged(Tp::ProfilePtr)),
            SIGNAL(profileChanged(Tp::ProfilePtr)));
    connect(mPriv->mAccount.data(),
            SIGNAL(displayNameChanged(QString)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(displayNameChanged(QString)),
            SIGNAL(displayNameChanged(QString)));
    connect(mPriv->mAccount.data(),
            SIGNAL(iconNameChanged(QString)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(iconNameChanged(QString)),
            SIGNAL(iconNameChanged(QString)));
    connect(mPriv->mAccount.data(),
            SIGNAL(nicknameChanged(QString)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(nicknameChanged(QString)),
            SIGNAL(nicknameChanged(QString)));
    connect(mPriv->mAccount.data(),
            SIGNAL(normalizedNameChanged(QString)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(normalizedNameChanged(QString)),
            SIGNAL(normalizedNameChanged(QString)));
    connect(mPriv->mAccount.data(),
            SIGNAL(validityChanged(bool)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(validityChanged(bool)),
            SIGNAL(validityChanged(bool)));
    connect(mPriv->mAccount.data(),
            SIGNAL(stateChanged(bool)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(stateChanged(bool)),
            SIGNAL(stateChanged(bool)));
    connect(mPriv->mAccount.data(),
            SIGNAL(capabilitiesChanged(Tp::ConnectionCapabilities)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(capabilitiesChanged(Tp::ConnectionCapabilities)),
            SLOT(onCapabilitiesChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(connectsAutomaticallyPropertyChanged(bool)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(connectsAutomaticallyPropertyChanged(bool)),
            SIGNAL(connectsAutomaticallyPropertyChanged(bool)));
    connect(mPriv->mAccount.data(),
            SIGNAL(parametersChanged(QVariantMap)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(parametersChanged(QVariantMap)),
            SIGNAL(parametersChanged(QVariantMap)));
    connect(mPriv->mAccount.data(),
            SIGNAL(changingPresence(bool)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(changingPresence(bool)),
            SIGNAL(changingPresence(bool)));
    connect(mPriv->mAccount.data(),
            SIGNAL(automaticPresenceChanged(Tp::Presence)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(automaticPresenceChanged(Tp::Presence)),
            SIGNAL(automaticPresenceChanged(Tp::Presence)));
    connect(mPriv->mAccount.data(),
            SIGNAL(currentPresenceChanged(Tp::Presence)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(currentPresenceChanged(Tp::Presence)),
            SIGNAL(currentPresenceChanged(Tp::Presence)));
    connect(mPriv->mAccount.data(),
            SIGNAL(requestedPresenceChanged(Tp::Presence)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(requestedPresenceChanged(Tp::Presence)),
            SIGNAL(requestedPresenceChanged(Tp::Presence)));
    connect(mPriv->mAccount.data(),
            SIGNAL(onlinenessChanged(bool)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(onlinenessChanged(bool)),
            SIGNAL(onlinenessChanged(bool)));
    connect(mPriv->mAccount.data(),
            SIGNAL(avatarChanged(Tp::Avatar)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(avatarChanged(Tp::Avatar)),
            SIGNAL(avatarChanged(Tp::Avatar)));
    connect(mPriv->mAccount.data(),
            SIGNAL(onlinenessChanged(bool)),
            SLOT(onChanged()));
    connect(mPriv->mAccount.data(),
            SIGNAL(onlinenessChanged(bool)),
            SIGNAL(onlinenessChanged(bool)));
    connect(mPriv->mAccount.data(),
            SIGNAL(connectionChanged(Tp::ConnectionPtr)),
            SLOT(onConnectionChanged(Tp::ConnectionPtr)));
    QTimer::singleShot(0, this, SLOT(startup()));
}

AccountsModelItem::~AccountsModelItem()
{
    delete mPriv;
}

void AccountsModelItem::startup()
{
    onConnectionChanged(mPriv->mAccount->connection());
}

QVariant AccountsModelItem::data(int role) const
{
    switch (role) {
        case AccountsModel::ItemRole:
            return QVariant::fromValue(
                const_cast<QObject *>(
                    static_cast<const QObject *>(this)));
        case AccountsModel::IdRole:
            return mPriv->mAccount->uniqueIdentifier();
        case AccountsModel::AvatarRole:
            return AvatarImageProvider::urlFor(mPriv->mAccount);
        case AccountsModel::ValidRole:
            return mPriv->mAccount->isValid();
        case AccountsModel::EnabledRole:
            return mPriv->mAccount->isEnabled();
        case AccountsModel::ConnectionManagerNameRole:
            return mPriv->mAccount->cmName();
        case AccountsModel::ProtocolNameRole:
            return mPriv->mAccount->protocolName();
        case AccountsModel::ServiceNameRole:
            return mPriv->mAccount->serviceName();
        case AccountsModel::DisplayNameRole:
        case Qt::DisplayRole:
            return mPriv->mAccount->displayName();
        case AccountsModel::IconRole:
            return mPriv->mAccount->iconName();
        case AccountsModel::NicknameRole:
            return mPriv->mAccount->nickname();
        case AccountsModel::ConnectsAutomaticallyRole:
            return mPriv->mAccount->connectsAutomatically();
        case AccountsModel::ChangingPresenceRole:
            return mPriv->mAccount->isChangingPresence();
        case AccountsModel::AutomaticPresenceRole:
            return mPriv->mAccount->automaticPresence().status();
        case AccountsModel::AutomaticPresenceTypeRole:
            return mPriv->mAccount->automaticPresence().type();
        case AccountsModel::AutomaticPresenceStatusMessageRole:
            return mPriv->mAccount->automaticPresence().statusMessage();
        case AccountsModel::CurrentPresenceRole:
            return mPriv->mAccount->currentPresence().status();
        case AccountsModel::CurrentPresenceTypeRole:
            return mPriv->mAccount->currentPresence().type();
        case AccountsModel::CurrentPresenceStatusMessageRole:
            return mPriv->mAccount->currentPresence().statusMessage();
        case AccountsModel::RequestedPresenceRole:
            return mPriv->mAccount->requestedPresence().status();
        case AccountsModel::RequestedPresenceTypeRole:
            return mPriv->mAccount->requestedPresence().type();
        case AccountsModel::RequestedPresenceStatusMessageRole:
            return mPriv->mAccount->requestedPresence().statusMessage();
        case AccountsModel::ConnectionStatusRole: {
            if (!mPriv->mAccount->connection().isNull()
                    && mPriv->mAccount->connection()->isValid()) {
                return mPriv->mAccount->connection()->status();
            } else {
                return Tp::ConnectionStatusDisconnected;
            }
        }
        case AccountsModel::ConnectionStatusReasonRole: {
            if (!mPriv->mAccount->connection().isNull()) {
                return mPriv->mAccount->connection()->statusReason();
            } else {
                return mPriv->mAccount->connectionStatusReason();
            }
        }
        case AccountsModel::ContactListStateRole: {
            if (!mPriv->mManager.isNull()) {
                return mPriv->mManager->state();
            } else {
                return Tp::ContactListStateNone;
            }
        }
        case AccountsModel::TextChatCapabilityRole:
            return mPriv->mAccount->capabilities().textChats();
        case AccountsModel::StreamedMediaCallCapabilityRole:
            return mPriv->mAccount->capabilities().streamedMediaCalls();
        case AccountsModel::StreamedMediaAudioCallCapabilityRole:
            return mPriv->mAccount->capabilities().streamedMediaAudioCalls();
        case AccountsModel::StreamedMediaVideoCallCapabilityRole:
            return mPriv->mAccount->capabilities().streamedMediaVideoCalls();
        case AccountsModel::StreamedMediaVideoCallWithAudioCapabilityRole:
            return mPriv->mAccount->capabilities().streamedMediaVideoCallsWithAudio();
        case AccountsModel::StreamedMediaUpgradeCallCapabilityRole:
            return mPriv->mAccount->capabilities().upgradingStreamedMediaCalls();
        case AccountsModel::MediaCallCapabilityRole:
            return mPriv->mAccountCaps.mediaCalls();
        case AccountsModel::AudioCallCapabilityRole:
            return mPriv->mAccountCaps.audioCalls();
        case AccountsModel::VideoCallCapabilityRole:
            return mPriv->mAccountCaps.videoCalls();
        case AccountsModel::VideoCallWithAudioCapabilityRole:
            return mPriv->mAccountCaps.videoCallsWithAudio();
        case AccountsModel::UpgradeCallCapabilityRole:
            return mPriv->mAccountCaps.upgradingCalls();
        case AccountsModel::FileTransferCapabilityRole:
            return mPriv->mAccountCaps.fileTransfers();
        default:
            return QVariant();
    }
}

/**
  * Set data on a contact model item
  * Currently, only the following roles are actually allowed; EnabledRole, RequestedPresenceRole,
  * RequestedPresenceStatusMessageRole, NicknameRole.
  * \param role the role to set the data in
  * \param value the value that will be set
  */
bool AccountsModelItem::setData(int role, const QVariant &value)
{
    switch (role) {
    case AccountsModel::EnabledRole:
        setEnabled(value.toBool());
        return true;
    case AccountsModel::RequestedPresenceRole:
        mPriv->setStatus(value.toString());
        return true;
    case AccountsModel::RequestedPresenceStatusMessageRole:
        mPriv->setStatusMessage(value.toString());
        return true;
    case AccountsModel::NicknameRole:
        setNickname(value.toString());
        return true;
    default:
        return false;
    }
}

/**
  * It returns the Tp::AccountPtr object of the item
  */
Tp::AccountPtr AccountsModelItem::account() const
{
     return mPriv->mAccount;
}

/**
  * Enable/disable the account
  * /param value true to enable the account, false to disable it
  */
void AccountsModelItem::setEnabled(bool value)
{
    mPriv->mAccount->setEnabled(value);
}

/**
  * Set the nickname of the account
  */
void AccountsModelItem::setNickname(const QString &value)
{
    mPriv->mAccount->setNickname(value);
}

/**
  * Set automatic presence of the account
  */
void AccountsModelItem::setAutomaticPresence(int type, const QString &status, const QString &statusMessage)
{
    Tp::Presence presence;
    presence.setStatus((Tp::ConnectionPresenceType) type, status, statusMessage);
    mPriv->mAccount->setAutomaticPresence(presence);
}

/**
  * Set requested presence of the account
  */
void AccountsModelItem::setRequestedPresence(int type, const QString &status, const QString &statusMessage)
{
    Tp::Presence presence;
    presence.setStatus((Tp::ConnectionPresenceType) type, status, statusMessage);
    mPriv->mAccount->setRequestedPresence(presence);
}

/**
  * It returns whether a presence type is allowed for this account
  */
bool AccountsModelItem::isPresenceSupported(int type) const
{
    Tp::PresenceSpecList list = mPriv->mAccount->allowedPresenceStatuses();
    Q_FOREACH (const Tp::PresenceSpec &p, list) {
        if (p.presence().type() == type && p.maySetOnSelf())
            return true;
    }
    return false;
}

void AccountsModelItem::onRemoved()
{
    int index = parent()->indexOf(this);
    emit childrenRemoved(parent(), index, index);
}

void AccountsModelItem::onChanged()
{
    emit changed(this);
}

void AccountsModelItem::onContactsChanged(const Tp::Contacts &addedContacts,
        const Tp::Contacts &removedContacts)
{
    foreach (const Tp::ContactPtr &contact, removedContacts) {
        for (int i = 0; i < size(); ++i) {
            ContactModelItem *item = qobject_cast<ContactModelItem *>(childAt(i));
            if (item->contact() == contact) {
                emit childrenRemoved(this, i, i);
                break;
            }
        }
    }

    // get the list of contact ids in the children
    QStringList idList;
    int numElems = size();
    for (int i = 0; i < numElems; ++i) {
        ContactModelItem *item = qobject_cast<ContactModelItem *>(childAt(i));
        if (item) {
            idList.append(item->contact()->id());
        }
    }

    QList<TreeNode *> newNodes;
    foreach (const Tp::ContactPtr &contact, addedContacts) {
        if (!idList.contains(contact->id())) {
            newNodes.append(new ContactModelItem(contact));
        }
    }
    if (newNodes.count()) {
        emit childrenAdded(this, newNodes);
    }
}

void AccountsModelItem::onStatusChanged(Tp::ConnectionStatus status)
{
    onChanged();
    emit connectionStatusChanged(mPriv->mAccount->uniqueIdentifier(), status);
}

/**
  * Called when an account's connection changes
  * \param connection The new connection object of the account
  */
void AccountsModelItem::onConnectionChanged(const Tp::ConnectionPtr &connection)
{
    // if the connection is invalid or disconnected, clear the contacts list
    if (connection.isNull()
            || !connection->isValid()
            || connection->status() == Tp::ConnectionStatusDisconnected) {
        if (size() > 0) {
            emit childrenRemoved(this, 0, size() - 1);
        }
        return;
    }

    connect(connection.data(), SIGNAL(statusChanged(Tp::ConnectionStatus)),
            SLOT(onStatusChanged(Tp::ConnectionStatus)));
    connect(connection.data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(onConnectionInvalidated()));
    onStatusChanged(connection->status());

    mPriv->mManager = connection->contactManager();
    connect(mPriv->mManager.data(),
            SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts,
                                           Tp::Channel::GroupMemberChangeDetails)),
            SLOT(onContactsChanged(Tp::Contacts,Tp::Contacts)));
    connect(mPriv->mManager.data(),
            SIGNAL(stateChanged(Tp::ContactListState)),
            SLOT(onContactManagerStateChanged(Tp::ContactListState)));
    onContactManagerStateChanged(mPriv->mManager->state());
}

/**
  * Called when the connection of the account is invalidated
  */
void AccountsModelItem::onConnectionInvalidated()
{
    onStatusChanged(Tp::ConnectionStatusDisconnected);
}

void AccountsModelItem::onContactManagerStateChanged(Tp::ContactListState state)
{
    onChanged();
    if (state == Tp::ContactListStateSuccess) {
        clearContacts();
        addKnownContacts();
    }
}

void AccountsModelItem::clearContacts()
{
    if (!mPriv->mAccount->connection().isNull() &&
        mPriv->mAccount->connection()->isValid()) {
        Tp::ContactManagerPtr manager = mPriv->mAccount->connection()->contactManager();
        Tp::Contacts contacts = manager->allKnownContacts();

        // remove the items no longer present
        for (int i = 0; i < size(); ++i) {
            bool exists = false;
            ContactModelItem *item = qobject_cast<ContactModelItem *>(childAt(i));
            if (item) {
                Tp::ContactPtr itemContact = item->contact();
                if (contacts.contains(itemContact)) {
                    exists = true;
                }
            }
            if (!exists) {
                emit childrenRemoved(this, i, i);
            }
        }
    } else {
        emit childrenRemoved(this, 0, size() - 1);
    }
}

void AccountsModelItem::addKnownContacts()
{
    // reload the known contacts if it has a connection
    QList<TreeNode *> newNodes;
    if (!mPriv->mAccount->connection().isNull() &&
        mPriv->mAccount->connection()->isValid()) {
        Tp::ContactManagerPtr manager = mPriv->mAccount->connection()->contactManager();
        Tp::Contacts contacts = manager->allKnownContacts();

        // get the list of contact ids in the children
        QStringList idList;
        int numElems = size();
        for (int i = 0; i < numElems; ++i) {
            ContactModelItem *item = qobject_cast<ContactModelItem *>(childAt(i));
            if (item) {
                idList.append(item->contact()->id());
            }
        }

        // only add the contact item if it is new
        foreach (const Tp::ContactPtr &contact, contacts) {
            if (!idList.contains(contact->id())) {
                newNodes.append(new ContactModelItem(contact));
            }
        }
    }

    if (newNodes.count() > 0) {
        emit childrenAdded(this, newNodes);
    }
}

void AccountsModelItem::onCapabilitiesChanged()
{
    mPriv->mAccountCaps = mPriv->mAccount->capabilities().allClassSpecs();
    emit capabilitiesChanged(mPriv->mAccountCaps);
}

}
