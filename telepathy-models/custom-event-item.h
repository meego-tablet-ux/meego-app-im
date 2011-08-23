/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_custom_event_item_h_HEADER_GUARD_
#define _Telepathy_Models_custom_event_item_h_HEADER_GUARD_

#include "event-item.h"
#include "global.h"

#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Types>

class TELEPATHY_MODELS_EXPORT CustomEventItem : public EventItem
{
    Q_OBJECT
    Q_DISABLE_COPY(CustomEventItem)

public:

    enum CustomEventType {
        /**
          * The user has left the chat conversation
          */
        CustomEventUserLeftChat = 0,

        // User defined values
        CustomEventUserDefined = 10000
    };

    CustomEventItem(const Tp::ContactPtr &sender, const Tp::ContactPtr &receiver,
        const QDateTime &time, const QString &customEventText, CustomEventType customEventType,
        QObject *parent = 0);
    virtual ~CustomEventItem();

    QString customEventText() const;
    CustomEventType customEventType() const;

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // _Telepathy_Models_custom_event_item_h_HEADER_GUARD_
