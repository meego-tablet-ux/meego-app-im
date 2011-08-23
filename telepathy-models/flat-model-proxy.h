/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_flat_model_proxy_h_HEADER_GUARD_
#define _Telepathy_Models_flat_model_proxy_h_HEADER_GUARD_

#include "global.h"

#include <QAbstractProxyModel>

class TELEPATHY_MODELS_EXPORT FlatModelProxy : public QAbstractProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY(FlatModelProxy)
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    FlatModelProxy(QAbstractItemModel *source);
    virtual ~FlatModelProxy();

    virtual QModelIndex mapFromSource(const QModelIndex &index) const;
    virtual QModelIndex mapToSource(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;

    int rowCount() const;

Q_SIGNALS:
    void rowCountChanged();

private Q_SLOTS:
    void onRowsAboutToBeInserted(const QModelIndex &index, int first, int last);
    void onRowsInserted(const QModelIndex &index, int first, int last);
    void onRowsAboutToBeRemoved(const QModelIndex &index, int first, int last);
    void onRowsRemoved(const QModelIndex &index, int first, int last);
    void onDataChanged(const QModelIndex &first, const QModelIndex &last);
    void onHierarchicalDataChanged(const QModelIndex &fisrt, const QModelIndex &last);

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // _Telepathy_Models_flat_model_proxy_h_HEADER_GUARD_
