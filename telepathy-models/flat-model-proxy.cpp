/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "flat-model-proxy.h"

//#include "TelepathyQt4Yell/Models/_gen/flat-model-proxy.moc.hpp"

#include "accounts-model.h"

struct TELEPATHY_MODELS_NO_EXPORT FlatModelProxy::Private
{
    int offsetOf(const FlatModelProxy *model, int index) const;
};

int FlatModelProxy::Private::offsetOf(const FlatModelProxy *model, int index) const
{
    int offset = 0;
    for (int i = 0; i < index; i++) {
        offset += model->sourceModel()->rowCount(model->sourceModel()->index(i, 0, QModelIndex()));
    }
    return offset;
}

/**
 * \class FlatModelProxy
 * \ingroup models
 * \headerfile TelepathyQt4Yell/flat-model-proxy.h <TelepathyQt4Yell/FlatModelProxy>
 *
 * \brief A proxy that transforms an AccountsModel object into a flat model of ContactModelItem objects
 *
 */

/**
  * Construct a FlatModelProxy object
  * \param source An AccountsModel object.
  */
FlatModelProxy::FlatModelProxy(QAbstractItemModel *source)
    : QAbstractProxyModel(source),
      mPriv(new Private())
{
    setSourceModel(source);

    connect(source,
            SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            SLOT(onRowsAboutToBeInserted(QModelIndex,int,int)));
    connect(source,
            SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(onRowsInserted(QModelIndex,int,int)));
    connect(source,
            SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            SLOT(onRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(source,
            SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(onRowsRemoved(QModelIndex,int,int)));
    connect(source,
            SIGNAL(rowsInserted(QModelIndex,int,int)),
            SIGNAL(rowCountChanged()));
    connect(source,
            SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SIGNAL(rowCountChanged()));
    connect(source,
            SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(onDataChanged(QModelIndex,QModelIndex)));

    AccountsModel *accountsModel = qobject_cast<AccountsModel *> (source);
    if (accountsModel) {
        connect(accountsModel,
                SIGNAL(hierarchicalDataChanged(QModelIndex,QModelIndex)),
                SLOT(onHierarchicalDataChanged(QModelIndex,QModelIndex)));
    }
}

FlatModelProxy::~FlatModelProxy()
{
    delete mPriv;
}

QModelIndex FlatModelProxy::mapFromSource(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    QModelIndex parent = index.parent();

    if (!parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(mPriv->offsetOf(this, parent.row()) + index.row(), index.column(), parent.row());
}

QModelIndex FlatModelProxy::mapToSource(const QModelIndex &index) const
{
    int parentRow = index.internalId();
    QModelIndex parent = sourceModel()->index(parentRow, 0, QModelIndex());
    int row = index.row() - mPriv->offsetOf(this, parent.row());
    return sourceModel()->index(row, index.column(), parent);
}

QModelIndex FlatModelProxy::index(int row, int column, const QModelIndex &parent) const
{
    int count = 0;
    for (int i = 0; i < sourceModel()->rowCount(QModelIndex()); i++) {
        QModelIndex sourceIndex = sourceModel()->index(i, 0, QModelIndex());
        count += sourceModel()->rowCount(sourceIndex);
        if (row < count) {
            return createIndex(row, column, i);
        }
    }

    return QModelIndex();
}

QModelIndex FlatModelProxy::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int FlatModelProxy::columnCount(const QModelIndex &parent) const
{
    return 1;
}

int FlatModelProxy::rowCount() const
{
    return rowCount(QModelIndex());
}

int FlatModelProxy::rowCount(const QModelIndex &parent) const
{
    return mPriv->offsetOf(this, sourceModel()->rowCount(QModelIndex()));
}

void FlatModelProxy::onRowsAboutToBeInserted(const QModelIndex &index, int first, int last)
{
    if (index.isValid()) {
        int offset = mPriv->offsetOf(this, index.row());
        int firstIndex = offset + first;
        int lastIndex = offset + last;

        beginInsertRows(QModelIndex(), firstIndex, lastIndex);
    }
}

void FlatModelProxy::onRowsAboutToBeRemoved(const QModelIndex &index, int first, int last)
{
    if (index.isValid()) {
        int offset = mPriv->offsetOf(this, index.row());
        int firstIndex = offset + first;
        int lastIndex = offset + last;

        beginRemoveRows(QModelIndex(), firstIndex, lastIndex);
    }
}

void FlatModelProxy::onRowsInserted(const QModelIndex &index, int first, int last)
{
    if (index.isValid()) {
        endInsertRows();
    }
}

void FlatModelProxy::onRowsRemoved(const QModelIndex &index, int first, int last)
{
    if (index.isValid()) {
        endRemoveRows();
    }
}

void FlatModelProxy::onDataChanged(const QModelIndex &first, const QModelIndex &last)
{
    if (first.parent().isValid() && last.parent().isValid() && first.parent() == last.parent()) {
        QModelIndex firstIndex = mapFromSource(first);
        QModelIndex lastIndex = mapFromSource(last);
        emit dataChanged(firstIndex, lastIndex);
    }
}

void FlatModelProxy::onHierarchicalDataChanged(const QModelIndex &first, const QModelIndex &last)
{
    if (!first.parent().isValid() && !last.parent().isValid()) {
        int firstOffset = mPriv->offsetOf(this, first.row());
        int lastOffset = mPriv->offsetOf(this, last.row() + 1) - 1;
        QModelIndex firstIndex = createIndex(firstOffset, 0, first.row());
        QModelIndex lastIndex = createIndex(lastOffset, 0, last.row());
        emit dataChanged(firstIndex, lastIndex);
    } else {
        // do not do normal dataChanged, since dataChanged it was already triggered separately
        //onDataChanged(first, last);
    }
}
