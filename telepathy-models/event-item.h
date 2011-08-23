/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_event_item_h_HEADER_GUARD_
#define _Telepathy_Models_event_item_h_HEADER_GUARD_

#include "global.h"

#include <TelepathyQt4/Types>

#include <QDateTime>
#include <QString>

class TELEPATHY_MODELS_EXPORT EventItem : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(EventItem)

public:
    EventItem(const Tp::ContactPtr &sender, const Tp::ContactPtr &receiver,
        const QDateTime &dateTime, QObject *parent = 0);
    virtual ~EventItem();

    Tp::ContactPtr sender() const;
    Tp::ContactPtr receiver() const;
    QDateTime dateTime() const;

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // _Telepathy_Models_event_item_h_HEADER_GUARD_
