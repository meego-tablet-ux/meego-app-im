/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "imfeedmodelitem.h"
#include "imfeedmodel.h"

IMFeedModelItem::IMFeedModelItem(Tp::AccountPtr account,
                                 Tp::ContactPtr contact,
                                 QString message,
                                 QDateTime time,
                                 McaActions *actions,
                                 int type,
                                 QString token,
                                 QString channelPath)
    : mItemType(type),
      mAccount(account),
      mContact(contact),
      mMessage(message),
      mTimestamp(time),
      mActions(actions),
      mUniqueId(token),
      mRelevance(0),
      mChannelPath(channelPath)
{
    // if it is a request, assign more relevance to it
    if (type == IMFeedModel::RequestType) {
        mRelevance = 1.0;
    }

    // if timestamp is null, use current time
    if (mTimestamp.isNull()) {
        mTimestamp = QDateTime::currentDateTime();
    }

    // if token is empty, use contact id and current time
    if (mUniqueId.isEmpty()) {
        mUniqueId = QString(mItemType + "&&" + mContact->id() + mTimestamp.toString(Qt::ISODate));
    }

    connect(mContact.data(), SIGNAL(avatarDataChanged(Tp::AvatarData)),
            SLOT(onContactChanged()));
    connect(mContact.data(), SIGNAL(aliasChanged(QString)),
            SLOT(onContactChanged()));
}

IMFeedModelItem::~IMFeedModelItem()
{
}

QString IMFeedModelItem::avatarUrl() const
{
    if (!mContact->avatarData().fileName.isEmpty()) {
        return mContact->avatarData().fileName;
    } else {
        return QString("image://themedimage/widgets/common/avatar/avatar-default");
    }
}

QString IMFeedModelItem::contactName() const
{
    return mContact->alias();
}

QString IMFeedModelItem::contactId() const
{
    return mContact->id();
}

QString IMFeedModelItem::message() const
{
    return mMessage;
}

QDateTime IMFeedModelItem::timestamp() const
{
    return mTimestamp;
}

int IMFeedModelItem::itemType() const
{
    return mItemType;
}

QString IMFeedModelItem::uniqueId() const
{
    return mUniqueId;
}

McaActions *IMFeedModelItem::actions()
{
    return mActions;
}

qreal IMFeedModelItem::relevance(void) const
{
    return mRelevance;
}

QString IMFeedModelItem::channelPath() const
{
    return mChannelPath;
}

void IMFeedModelItem::onContactChanged()
{
    emit itemChanged(this);
}
