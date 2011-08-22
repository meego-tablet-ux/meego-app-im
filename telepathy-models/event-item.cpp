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

#include <TelepathyQt4Yell/Models/EventItem>

#include "TelepathyQt4Yell/Models/_gen/event-item.moc.hpp"

#include <TelepathyQt4/Contact>

namespace Tpy
{

struct TELEPATHY_QT4_YELL_MODELS_NO_EXPORT EventItem::Private
{
    Private(const Tp::ContactPtr &sender,
            const Tp::ContactPtr &receiver,
            const QDateTime &dateTime)
        : mSender(sender),
          mReceiver(receiver),
          mDateTime(dateTime.isValid() ? dateTime : QDateTime::currentDateTime())
    {
    }

    Tp::ContactPtr mSender;
    Tp::ContactPtr mReceiver;
    QDateTime mDateTime;
};

/**
 * \class EventItem
 * \ingroup models
 * \headerfile TelepathyQt4Yell/eventitem.h <TelepathyQt4Yell/EventItem>
 *
 * \brief A model for a text chat conversation
 *
 */

/**
  * Construct a EventItem object
  * \param sender The originating contact of the event
  * \param receiver The recipient of the event
  * \param dateTime The time at which the event took place
  * \param parent The parent object
  */
EventItem::EventItem(const Tp::ContactPtr &sender, const Tp::ContactPtr &receiver,
    const QDateTime &dateTime, QObject *parent)
    : QObject(parent),
      mPriv(new Private(sender, receiver, dateTime))
{
}

EventItem::~EventItem()
{
    delete mPriv;
}

Tp::ContactPtr EventItem::sender() const
{
    return mPriv->mSender;
}

Tp::ContactPtr EventItem::receiver() const
{
    return mPriv->mReceiver;
}

QDateTime EventItem::dateTime() const
{
    return mPriv->mDateTime;
}

}
