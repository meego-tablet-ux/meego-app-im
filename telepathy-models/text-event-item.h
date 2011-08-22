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

#ifndef _TelepathyQt4Yell_Models_text_event_item_h_HEADER_GUARD_
#define _TelepathyQt4Yell_Models_text_event_item_h_HEADER_GUARD_

#ifndef IN_TELEPATHY_QT4_YELL_MODELS_HEADER
#error IN_TELEPATHY_QT4_YELL_MODELS_HEADER
#endif

#include <TelepathyQt4Yell/Models/EventItem>
#include <TelepathyQt4Yell/Models/Global>

#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Types>

#include <QDateTime>
#include <QString>

namespace Tpy
{

class TELEPATHY_QT4_YELL_MODELS_EXPORT TextEventItem : public EventItem
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

}

#endif // _TelepathyQt4Yell_Models_text_event_item_h_HEADER_GUARD_
