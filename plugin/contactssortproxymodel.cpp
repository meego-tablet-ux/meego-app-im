/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "contactssortproxymodel.h"

#include "imaccountsmodel.h"
#include "imgroupchatmodel.h"
#include "../telepathy-qml-lib/chatagent.h"
#include "../telepathy-qml-lib/settingshelper.h"

#include <TelepathyQt4/AvatarData>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4Yell/Models/AccountsModel>
#include <TelepathyQt4Yell/Models/ContactModelItem>

#include <meegolocale.h>

#include <QtGui>

ContactsSortProxyModel::ContactsSortProxyModel(TelepathyManager *manager,
                                               QAbstractItemModel *model,
                                               QObject *parent)
    : QSortFilterProxyModel(parent),
      mManager(manager),
      mModel(model)
{
    QTimer::singleShot(0, this, SLOT(startup()));
}

ContactsSortProxyModel::~ContactsSortProxyModel()
{
}

void ContactsSortProxyModel::startup()
{
    setDynamicSortFilter(true);
    setSourceModel(mModel);
    sort(0, Qt::AscendingOrder);
}

bool ContactsSortProxyModel::filterAcceptsColumn(int, const QModelIndex &) const
{
    return true;
}

bool ContactsSortProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceRow);
    Q_UNUSED(sourceParent);
    return true;
}

bool ContactsSortProxyModel::lessThan(const QModelIndex &left,
                                      const QModelIndex &right) const
{
    // first evaluate parent display names
    QString leftParentName = sourceModel()->data(left, IMAccountsModel::ParentDisplayNameRole).toString();
    QString rightParentName = sourceModel()->data(right, IMAccountsModel::ParentDisplayNameRole).toString();

    if (leftParentName != rightParentName) {
        static meego::Locale locale;
        return locale.lessThan(leftParentName, rightParentName);
    }

    // evaluate friend requests
    int leftPublish = sourceModel()->data(left, IMAccountsModel::PublishStateRole).toInt();
    int rightPublish = sourceModel()->data(right, IMAccountsModel::PublishStateRole).toInt();

    if (leftPublish != rightPublish) {
        if (leftPublish ==  Tp::Contact::PresenceStateAsk) {
            return true;
        } else if (rightPublish ==  Tp::Contact::PresenceStateAsk) {
            return false;
        }
    }

    // evaluate missed video calls
    int leftMissedVideoCalls = sourceModel()->data(left, IMAccountsModel::MissedVideoCallsRole).toInt();
    int rightMissedVideoCalls = sourceModel()->data(right, IMAccountsModel::MissedVideoCallsRole).toInt();

    if (leftMissedVideoCalls != rightMissedVideoCalls) {
        return !(leftMissedVideoCalls < rightMissedVideoCalls);
    }

    // evaluate missed audio calls
    int leftMissedAudioCalls = sourceModel()->data(left, IMAccountsModel::MissedAudioCallsRole).toInt();
    int rightMissedAudioCalls = sourceModel()->data(right, IMAccountsModel::MissedAudioCallsRole).toInt();

    if (leftMissedAudioCalls != rightMissedAudioCalls) {
        return !(leftMissedAudioCalls < rightMissedAudioCalls);
    }

    // evaluate pending messages
    int leftPending = sourceModel()->data(left, IMAccountsModel::PendingMessagesRole).toInt();
    int rightPending = sourceModel()->data(right, IMAccountsModel::PendingMessagesRole).toInt();

    if (leftPending != rightPending) {
        return !(leftPending < rightPending);
    }

    // if any has an open chat
    bool leftOpenChat = sourceModel()->data(left, IMAccountsModel::ChatOpenedRole).toInt();
    bool rightOpenChat = sourceModel()->data(right, IMAccountsModel::ChatOpenedRole).toInt();

    // the logic is inverted here because 1 means a chat is open and that takes higher priority
    if (leftOpenChat != rightOpenChat) {
        return !(leftOpenChat < rightOpenChat);
    }

    // evaluate presence
    int leftType = sourceModel()->data(left, Tpy::AccountsModel::PresenceTypeRole).toInt();
    int rightType = sourceModel()->data(right, Tpy::AccountsModel::PresenceTypeRole).toInt();

    // order by presence type
    int leftOrderedType = presenceOrder(leftType);
    int rightOrderedType = presenceOrder(rightType);

    if (leftOrderedType != rightOrderedType) {
        return (leftOrderedType < rightOrderedType);
    }

    // compare the alias
    QString leftAlias = sourceModel()->data(left, Tpy::AccountsModel::AliasRole).toString();
    QString rightAlias = sourceModel()->data(right, Tpy::AccountsModel::AliasRole).toString();

    if (leftAlias != rightAlias) {
        static meego::Locale locale;
        return locale.lessThan(leftAlias, rightAlias);
    }

    QString leftId = sourceModel()->data(left, Tpy::AccountsModel::IdRole).toString();
    QString rightId = sourceModel()->data(right, Tpy::AccountsModel::IdRole).toString();
    return (leftId < rightId);
}

int ContactsSortProxyModel::presenceOrder(const int type) const
{
    switch(type) {
    case Tp::ConnectionPresenceTypeAvailable:
    case Tp::ConnectionPresenceTypeAway:
    case Tp::ConnectionPresenceTypeExtendedAway:
    case Tp::ConnectionPresenceTypeBusy:
        return type; // return as-is
    case Tp::ConnectionPresenceTypeOffline:
        return (Tp::ConnectionPresenceTypeBusy + 1); // offline should be right after busy
    default:
        // anything else should be last in no particular order
        return Tp::ConnectionPresenceTypeError;
    }
}
