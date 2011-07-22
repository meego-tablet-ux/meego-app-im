/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __imfeedproxymodel_h
#define __imfeedproxymodel_h

#include <QSortFilterProxyModel>

class IMFeedProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    IMFeedProxyModel(QAbstractItemModel *model, QObject *parent = 0);

};

#endif // __imfeedproxymodel_h
