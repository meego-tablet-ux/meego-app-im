/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "contactssortfilterproxymodel.h"

#include "imaccountsmodel.h"
#include "imgroupchatmodel.h"
#include "../telepathy-qml-lib/settingshelper.h"

#include <TelepathyQt4/AvatarData>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4Yell/Models/AccountsModel>
#include <TelepathyQt4Yell/Models/ContactModelItem>

#include <meegolocale.h>

#include <QtGui>

ContactsSortFilterProxyModel::ContactsSortFilterProxyModel(TelepathyManager *manager,
                                                           QAbstractItemModel *model,
                                                           QObject *parent)
    : QSortFilterProxyModel(parent),
      mManager(manager),
      mModel(model),
      mServiceName(""),
      mStringFilter(""),
      mShowOffline(SettingsHelper::self()->showOfflineContacts()),
      mContactsOnly(false),
      mBlockedOnly(false)
{
    QTimer::singleShot(0, this, SLOT(startup()));
}

ContactsSortFilterProxyModel::~ContactsSortFilterProxyModel()
{
}

void ContactsSortFilterProxyModel::startup()
{
    setSourceModel(mModel);
}

bool ContactsSortFilterProxyModel::filterAcceptsColumn(int, const QModelIndex &) const
{
    return true;
}

bool ContactsSortFilterProxyModel::filterAcceptsRow(int sourceRow,
                                                    const QModelIndex &sourceParent) const
{
    bool filtered = false;
    // when showing blocked contacts do not filter by connection
    if (mBlockedOnly) {
        // verify the index is valid before doing anything else
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        if (index.isValid()) {
            Tpy::ContactModelItem *contactItem = qobject_cast<Tpy::ContactModelItem *>(
                        index.data(Tpy::AccountsModel::ItemRole).value<QObject *>());

            if (contactItem) {
                Tp::ContactPtr contact = contactItem->contact();
                if(contact.isNull() || !contact->isBlocked()) {
                    return false;
                }

                if (contact->manager().isNull() ||
                    contact->manager()->connection().isNull() ||
                    contact == contact->manager()->connection()->selfContact()) {
                    return false;
                }

                if (mSkippedContacts.contains(contact->id())) {
                    return false;
                }

                if (filtered && !mStringFilter.isEmpty()) {
                    if (contact->alias().contains(mStringFilter, Qt::CaseInsensitive) ||
                            contact->id().contains(mStringFilter, Qt::CaseInsensitive)) {
                        filtered = true;
                    } else {
                        filtered = false;
                    }
                } else {
                    filtered = true;
                }
            }
        }
    } else if (haveConnection()) {

        if (mConnection->isValid()
                && mConnection->actualFeatures().contains(Tp::Connection::FeatureSelfContact)) {
            QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

            // verify the index is valid before doing anything else
            if (index.isValid()) {
                Tpy::ContactModelItem *contactItem = qobject_cast<Tpy::ContactModelItem *>(
                            index.data(Tpy::AccountsModel::ItemRole).value<QObject *>());

                if (contactItem) {
                    Tp::ContactPtr contact = contactItem->contact();

                    // filter out the self contact first
                    if (contact == mConnection->selfContact()) {
                        return false;
                    }

                    // filter rejected subscription requests
                    if (contact->isSubscriptionRejected()
                            || (contact->subscriptionState() == Tp::Contact::PresenceStateNo
                                && contact->publishState() == Tp::Contact::PresenceStateNo)) {
                        return false;
                    }

                    // filter requests if set to show only contacts
                    if (mContactsOnly && contact->publishState() == Tp::Contact::PresenceStateAsk) {
                        return false;
                    }

                    //filter blocked contacts
                    if (!mBlockedOnly && contact->isBlocked()) {
                        return false;
                    }

                    if (mSkippedContacts.contains(contact->id())) {
                        return false;
                    }

                    // do not show contact if not upgraded yet
                    // check only for FeatureAlias, as some protocols may not support all features,
                    // but the contacts ARE upgraded nonetheless
                    if (!contact->actualFeatures().contains(Tp::Contact::FeatureAlias)) {
                        return false;
                    }

                    // filter offline and unknown state if set
                    // if requests only, do not filter
                    if (!mShowOffline && contact->publishState() != Tp::Contact::PresenceStateAsk) {
                        Tp::ConnectionPresenceType status = contact->presence().type();
                        if (status == Tp::ConnectionPresenceTypeError
                                || status == Tp::ConnectionPresenceTypeHidden
                                || status == Tp::ConnectionPresenceTypeOffline
                                || status == Tp::ConnectionPresenceTypeUnknown
                                || status == Tp::ConnectionPresenceTypeUnset) {
                            return false;
                        }
                    }

                    // whether the contacts matches the filtered connection
                    filtered = (!contact->manager().isNull() && mConnection == contact->manager()->connection());

                    // if the contact is accepted, it also has to match the string filter
                    // the string filter matches the id or alias to a user-entered string
                    if (filtered && !mStringFilter.isEmpty()) {
                        if (contact->alias().contains(mStringFilter, Qt::CaseInsensitive) ||
                                contact->id().contains(mStringFilter, Qt::CaseInsensitive)) {
                            filtered = true;
                        } else {
                            filtered = false;
                        }
                    }
                } else {
                    IMGroupChatModelItem *groupChatItem = qobject_cast<IMGroupChatModelItem *>(
                                index.data(Tpy::AccountsModel::ItemRole).value<QObject *>());

                    if(groupChatItem) {

                        // in some cases, only contacts or requests should be shown
                        if(mContactsOnly || mBlockedOnly) {
                            return false;
                        }

                        filtered = (groupChatItem->channel()->connection() == mConnection);

                        // check if the string filter matches any of the names in the group chat
                        if (filtered and !mStringFilter.isEmpty()) {
                            foreach(Tp::ContactPtr contact, groupChatItem->channel()->groupContacts().toList()) {
                                if (contact->alias().contains(mStringFilter, Qt::CaseInsensitive) ||
                                        contact->id().contains(mStringFilter, Qt::CaseInsensitive)) {
                                    filtered = true;
                                } else {
                                    filtered = false;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return filtered;
}

void ContactsSortFilterProxyModel::filterByConnection(const Tp::ConnectionPtr &connection)
{
    mConnection = connection;
    invalidate();
}

void ContactsSortFilterProxyModel::filterByAccountId(const QString &id)
{
    foreach (Tp::AccountPtr accountPtr, mManager->accounts()) {
        if (accountPtr->uniqueIdentifier() == id) {
            mAccountId = id;
            mServiceName = accountPtr->serviceName();
            filterByConnection(accountPtr->connection());
            emit accountIdChanged(mAccountId);
            // saved the account id in the settings
            QSettings settings("MeeGo", "meego-app-im");
            settings.setValue("LastUsed/Account", id);
            return;
        }
    }
    mConnection = Tp::ConnectionPtr();
    invalidate();
}

QString ContactsSortFilterProxyModel::serviceName() const
{
    return mServiceName;
}

bool ContactsSortFilterProxyModel::haveConnection() const
{
    return (mConnection && mConnection->isValid() && mConnection->status() == Tp::ConnectionStatusConnected);
}

void ContactsSortFilterProxyModel::filterByString(const QString &filter)
{
    // only set the filter if it's different to current filter
    if (filter != mStringFilter) {
        mStringFilter = filter;
        invalidateFilter();
    }
}

void ContactsSortFilterProxyModel::filterByLastUsedAccount(const QString &accountId)
{
    emit openLastUsedAccount(accountId);
}

void ContactsSortFilterProxyModel::setShowOffline()
{
    mShowOffline = SettingsHelper::self()->showOfflineContacts();
}

bool ContactsSortFilterProxyModel::isShowOffline() const
{
    return mShowOffline;
}

void ContactsSortFilterProxyModel::setContactsOnly(bool toggle)
{
    mContactsOnly = toggle;
    invalidateFilter();
    emit rowCountChanged();
}

bool ContactsSortFilterProxyModel::isContactsOnly() const
{
    return mContactsOnly;
}

void ContactsSortFilterProxyModel::setBlockedOnly(bool toggle)
{
    mBlockedOnly = toggle;
    invalidateFilter();
    emit rowCountChanged();
}

bool ContactsSortFilterProxyModel::isBlockedOnly() const
{
    return mBlockedOnly;
}

QString ContactsSortFilterProxyModel::accountId() const
{
    return mAccountId;
}

void ContactsSortFilterProxyModel::skipContacts(const QStringList &contactsList)
{
    mSkippedContacts = contactsList;
    invalidateFilter();
    emit rowCountChanged();
}

void ContactsSortFilterProxyModel::clearSkippedContacts()
{
    mSkippedContacts.clear();
    invalidateFilter();
    emit rowCountChanged();
}
