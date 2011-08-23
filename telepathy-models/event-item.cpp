/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "event-item.h"

//#include "TelepathyQt4Yell/Models/_gen/event-item.moc.hpp"

#include <TelepathyQt4/Contact>

struct TELEPATHY_MODELS_NO_EXPORT EventItem::Private
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
