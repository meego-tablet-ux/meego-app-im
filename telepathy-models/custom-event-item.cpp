/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "custom-event-item.h"

//#include "TelepathyQt4Yell/Models/_gen/custom-event-item.moc.hpp"

struct TELEPATHY_MODELS_NO_EXPORT CustomEventItem::Private
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
