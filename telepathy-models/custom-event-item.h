/*
 * This file is part of TelepathyQt4Yell Models
 *
 * Copyright (C) 2010 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _Telepathy_Models_custom_event_item_h_HEADER_GUARD_
#define _Telepathy_Models_custom_event_item_h_HEADER_GUARD_

#ifndef IN_TELEPATHY_MODELS_HEADER
#error IN_TELEPATHY_MODELS_HEADER
#endif

#include "EventItem"
#include "Global"

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
