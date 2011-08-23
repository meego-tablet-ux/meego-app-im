/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "text-event-item.h"

struct TELEPATHY_MODELS_NO_EXPORT TextEventItem::Private
{
    Private(const QString &message, Tp::ChannelTextMessageType messageType)
        : mMessageText(message),
          mMessageType(messageType)
    {
    }

    QString mMessageText;
    Tp::ChannelTextMessageType mMessageType;
};

/**
 * \class TextEventItem
 * \ingroup models
 * \headerfile TelepathyQt4Yell/text-event-item.h <TelepathyQt4Yell/TextEventItem>
 *
 * \brief A model item that represents a text message
 *
 */

/**
  * Construct a TextEventItem object
  * \param sender The originating contact of the event
  * \param receiver The recipient of the event
  * \param time The time at which the event took place
  * \param message The text message
  * \param messageType The type of message
  * \param parent The parent object
  */
TextEventItem::TextEventItem(const Tp::ContactPtr &sender,
    const Tp::ContactPtr &receiver, const QDateTime &time,
    const QString &message, Tp::ChannelTextMessageType messageType, QObject *parent)
    : EventItem(sender, receiver, time, parent),
      mPriv(new Private(message, messageType))
{
}

TextEventItem::~TextEventItem()
{
    delete mPriv;
}

QString TextEventItem::messageText() const
{
    return mPriv->mMessageText;
}

Tp::ChannelTextMessageType TextEventItem::messageType() const
{
    return mPriv->mMessageType;
}
