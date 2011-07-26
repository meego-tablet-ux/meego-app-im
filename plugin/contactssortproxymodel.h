/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef CONTACTSSORTPROXYMODEL_H
#define CONTACTSSORTPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "../telepathy-qml-lib/telepathymanager.h"
#include <TelepathyQt4Yell/Models/ContactModelItem>

class IMGroupChatModelItem;

class ContactsSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ContactsSortProxyModel(TelepathyManager *manager, QAbstractItemModel *model, QObject *parent = 0);
    ~ContactsSortProxyModel();

protected:
    friend class ContactsSortFilterProxyModel;

    bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    /**
      * This method returns an order according to the presence type
      */
    int presenceOrder(const int type) const;

protected slots:
    void startup();

private:
    TelepathyManager *mManager;
    QAbstractItemModel *mModel;
};

#endif
