/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <actions.h>

#include "imservmodel.h"

IMServiceModel::IMServiceModel(TelepathyManager *tpManager, IMProtocolsModel *protoModel, QObject *parent)
    : McaServiceModel(parent),
      CATEGORY("im"),
      m_tpManager(tpManager),
      m_protocolsModel(protoModel)
{
    m_actions = new McaActions;
}

IMServiceModel::~IMServiceModel()
{
    delete m_actions;
}

int IMServiceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_accounts.count();
}

QVariant IMServiceModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row >= m_accounts.count()) {
        return QVariant();
    }

    switch (role) {
    case CommonDisplayNameRole:
        // This display name is a localized name for your service -- if you
        //   provide more than one service, each should have a distinct name
        //   for example, identifying the account. But it should be a title,
        //   preferably under ~32 characters.
        return m_tpManager->accountDisplayName(m_accounts.at(row)->iconName(), m_accounts.at(row)->displayName());

    case CommonIconUrlRole:
        // Here you can provide a small icon that identifies the service.
        //   This icon would probably be the same for each account if you
        //   provide multiple accounts as separate "services".
        return m_accounts.at(row)->iconName();

    case RequiredCategoryRole:
        // Currently we define three categories: "social", "email", and "im"
        //   that will be pulled into the Friends panel. In the future we
        //   may extend the categories for other panels and purposes.
        return CATEGORY;

    case RequiredNameRole:
        // This field is a unique name for the service within this plugin
        //   If you provide multiple accounts each should have its own
        //   unique id. This is not user-visible.
        return m_accounts.at(row)->uniqueIdentifier();

    case CommonActionsRole:
        // This is required if you will ever return true for
        //   CommonConfigErrorRole.
        return QVariant::fromValue<McaActions*>(m_actions);

    case CommonCapFlagsRole:
        return McaServiceModel::ProvidesFeed;

    case CommonConfigErrorRole:
        return false;

    default:
        // There is also the CommonConfigErrorRole which is a bool and just
        //   requests the UI to alert the user to misconfiguration, and gives
        //   them an opportunity to configure your service. If you return
        //   true here, you must also provide the CommonActionsRole above
        //   with a handler watching for a "configure" action.
        qWarning() << "IM Plugin Service model Unhandled data role requested! Role: " << role ;
        return QVariant();
    }
}

void IMServiceModel::performAction(QString action, QString uniqueid)
{
    // The mtfcontent sample application provides Configure buttons for each
    //   service you report so you can test that you are receiving the
    //   configure signal properly. In the real application, we plan to only
    //   provide this option to the user if you report that there is a
    //   configuration error through CommonConfigErrorRole.
    if (action == "configure") {
        configure(uniqueid);
    }
}

void IMServiceModel::configure(QString serviceName)
{
    Q_UNUSED(serviceName);
    qDebug() << "Configure option not available";
}

void IMServiceModel::onAccountAvailable(Tp::AccountPtr account)
{
    // check if it already exists
    foreach (Tp::AccountPtr accountPtr, m_accounts) {
        if (accountPtr->uniqueIdentifier() == account->uniqueIdentifier()) {
            return;
        }
    }

    connect(account.data(), SIGNAL(removed()),
            SLOT(onAccountRemoved()));

    // add if new
    beginInsertRows(QModelIndex(), m_accounts.count(), m_accounts.count());
    m_accounts.append(account);
    endInsertRows();

    // check existing accounts
    // if there are accounts of the same service, the name has to be updated
    foreach (Tp::AccountPtr accountPtr, m_accounts) {
        if (accountPtr->iconName() == account->iconName()) {
            onItemChanged(m_accounts.indexOf(accountPtr));
        }
    }
}

void IMServiceModel::onItemChanged(int row)
{
    QModelIndex rowIndex = index(row, 0);
    emit dataChanged(rowIndex, rowIndex);
}

void IMServiceModel::onAccountRemoved()
{
    Tp::Account *account = qobject_cast<Tp::Account *>(sender());

    // make sure we are not acting on a null object
    if (account) {
        QString id = account->uniqueIdentifier();
        if (!id.isEmpty()) {
            foreach (Tp::AccountPtr accountPtr, m_accounts) {
                int index = m_accounts.indexOf(accountPtr);
                if (!accountPtr.isNull()) {
                    if (id == accountPtr->uniqueIdentifier()) {
                        //remove the item
                        beginRemoveRows(QModelIndex(), index, index);
                        m_accounts.removeAt(index);
                        endRemoveRows();
                        break;
                    }
                } else {
                    // if null, remove the item
                    beginRemoveRows(QModelIndex(), index, index);
                    m_accounts.removeAt(index);
                    endRemoveRows();
                }
            }
        }
    }
}
