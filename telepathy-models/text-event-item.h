/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_text_event_item_h_HEADER_GUARD_
#define _Telepathy_Models_text_event_item_h_HEADER_GUARD_

#include "event-item.h"
#include "global.h"

#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Types>

#include <QDateTime>
#include <QString>

class TELEPATHY_MODELS_EXPORT TextEventItem : public EventItem
{
    Q_OBJECT
    Q_DISABLE_COPY(TextEventItem)

public:
    TextEventItem(const Tp::ContactPtr &sender, const Tp::ContactPtr &receiver,
        const QDateTime &time, const QString &message,
        Tp::ChannelTextMessageType messageType, QObject *parent = 0);
    virtual ~TextEventItem();

    QString messageText() const;
    Tp::ChannelTextMessageType messageType() const;

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // _Telepathy_Models_text_event_item_h_HEADER_GUARD_
