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

#include <TelepathyQt4Yell/Models/CustomEventItem>

#include "TelepathyQt4Yell/Models/_gen/custom-event-item.moc.hpp"

namespace Tpy
{

struct TELEPATHY_QT4_YELL_MODELS_NO_EXPORT CustomEventItem::Private
{
    Private(const QString &customEventText, CustomEventItem::CustomEventType customEventType)
        : mCustomEventText(customEventText),
          mCustomEventType(customEventType)
    {
    }

    QString mCustomEventText;
    CustomEventItem::CustomEventType mCustomEventType;
};

/**
 * \class CustomEventItem
 * \ingroup models
 * \headerfile TelepathyQt4Yell/custom-event-item.h <TelepathyQt4Yell/CustomEventItem>
 *
 * \brief A model item to hold non-message events in a conversation
 *
 */

/**
  * Construct a CustomEventItem object
  * \param sender The contact originating the event
  * \param receiver The intended recipient of the event
  * \param time The time at which the event took place
  * \param customEventText A description of the event
  * \param customEventType The type of the event
  * \param parent the parent object
  */
CustomEventItem::CustomEventItem(const Tp::ContactPtr &sender,
    const Tp::ContactPtr &receiver, const QDateTime &time,
    const QString &customEventText, CustomEventItem::CustomEventType customEventType,
    QObject *parent)
    : EventItem(sender, receiver, time, parent),
      mPriv(new Private(customEventText, customEventType))
{
}

CustomEventItem::~CustomEventItem()
{
    delete mPriv;
}

QString CustomEventItem::customEventText() const
{
    return mPriv->mCustomEventText;
}

CustomEventItem::CustomEventType CustomEventItem::customEventType() const
{
    return mPriv->mCustomEventType;
}

}
