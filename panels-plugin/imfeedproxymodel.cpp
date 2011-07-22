/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "imfeedproxymodel.h"

IMFeedProxyModel::IMFeedProxyModel(QAbstractItemModel *model, QObject *parent) :
    QSortFilterProxyModel(parent)
{
    setSourceModel(model);
    invalidate();
}
