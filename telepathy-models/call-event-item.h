/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_call_event_item_h_HEADER_GUARD_
#define _Telepathy_Models_call_event_item_h_HEADER_GUARD_

#include "event-item.h"
#include "global.h"

#include <TelepathyQt4Yell/Constants>
#include <TelepathyQt4/Types>

#include <QDateTime>
#include <QString>

class TELEPATHY_MODELS_EXPORT CallEventItem : public EventItem
{
    Q_OBJECT
    Q_DISABLE_COPY(CallEventItem)

public:
    CallEventItem(const Tp::ContactPtr &sender,
        const Tp::ContactPtr &receiver, const QDateTime &time,
        const QTime &duration, Tp::ContactPtr &endActor,
        Tpy::CallStateChangeReason &endReason, const QString &detailedEndReason,
        QObject *parent = 0);
    virtual ~CallEventItem();

    QTime duration() const;
    Tp::ContactPtr endActor() const;
    Tpy::CallStateChangeReason endReason() const;
    QString detailedEndReason() const;

    bool missedCall() const;
    bool rejectedCall() const;

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // _Telepathy_Models_call_event_item_h_HEADER_GUARD_
