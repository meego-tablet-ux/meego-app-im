/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "call-event-item.h"

//#include "TelepathyQt4Yell/Models/_gen/call-event-item.moc.hpp"

#include <TelepathyQt4/Contact>

struct TELEPATHY_MODELS_NO_EXPORT CallEventItem::Private
{
    Private(QTime duration, const Tp::ContactPtr &endActor,
            Tpy::CallStateChangeReason endReason, const QString &detailedEndReason)
        : mDuration(duration),
          mEndActor(endActor),
          mEndReason(endReason),
          mDetailedEndReason(detailedEndReason)
    {
    }

    QTime mDuration;
    Tp::ContactPtr mEndActor;
    Tpy::CallStateChangeReason mEndReason;
    QString mDetailedEndReason;
};

/**
 * \class CallEventItem
 * \ingroup models
 * \headerfile TelepathyQt4Yell/call-event-item.h <TelepathyQt4Yell/CallEventItem>
 *
 * \brief A model item that represents a call-related events
 *
 */

/**
  * Construct a CallEventItem object
  * \param sender The originating contact of the event
  * \param receiver The recipient of the event
  * \param time The time at which the event took place
  * \param duration Duration of the event
  * \param endActor Contact that ended the event
  * \param endReason Reason to end the event
  * \param detailedEndReason Detailed description of the end reason
  * \param parent The parent object
  */
CallEventItem::CallEventItem(const Tp::ContactPtr &sender,
    const Tp::ContactPtr &receiver, const QDateTime &time,
    const QTime &duration, Tp::ContactPtr &endActor,
    Tpy::CallStateChangeReason &endReason, const QString &detailedEndReason,
    QObject *parent)
    : EventItem(sender, receiver, time, parent),
      mPriv(new Private(duration, endActor, endReason, detailedEndReason))
{
}

CallEventItem::~CallEventItem()
{
    delete mPriv;
}

QTime CallEventItem::duration() const
{
    return mPriv->mDuration;
}

Tp::ContactPtr CallEventItem::endActor() const
{
    return mPriv->mEndActor;
}

Tpy::CallStateChangeReason CallEventItem::endReason() const
{
    return mPriv->mEndReason;
}

QString CallEventItem::detailedEndReason() const
{
    return mPriv->mDetailedEndReason;
}

/**
  * Returns whether the event is a missed call
  */
bool CallEventItem::missedCall() const
{
    return (endActor().isNull() &&
            endReason() == Tpy::CallStateChangeReasonNoAnswer &&
            duration().elapsed() <= 0);
}

/**
  * Returns whether the event is a rejected call
  */
bool CallEventItem::rejectedCall() const
{
    return (!endActor().isNull() &&
            endReason() == Tpy::CallStateChangeReasonUserRequested &&
            duration().elapsed() <= 0 &&
            detailedEndReason() == TP_QT4_ERROR_REJECTED);
}
