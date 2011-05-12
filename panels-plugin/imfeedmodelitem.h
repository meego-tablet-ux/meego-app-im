/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef IMFEEDMODELITEM_H
#define IMFEEDMODELITEM_H

#include <actions.h>

#include <QObject>
#include <QDateTime>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/Contact>

class IMFeedModelItem : public QObject
{
    Q_OBJECT

public:
    IMFeedModelItem(Tp::AccountPtr account, Tp::ContactPtr contact, QString message, QDateTime time, McaActions *actions, int type, QString token);
    ~IMFeedModelItem();

    QString contactName(void) const;
    QString contactId(void) const;
    QString message(void) const;
    QDateTime timestamp(void) const;
    QString avatarUrl(void) const;
    int itemType(void) const;
    McaActions *actions(void);
    QString uniqueId(void) const;
    qreal relevance(void) const;

Q_SIGNALS:
    void itemChanged(IMFeedModelItem* item);

protected Q_SLOTS:
    void onContactChanged();

private:
    int mItemType;
    Tp::AccountPtr mAccount;
    Tp::ContactPtr mContact;
    QString mMessage;
    QDateTime mTimestamp;
    McaActions *mActions;
    QString mUniqueId;
    qreal mRelevance;
};

#endif // IMFEEDMODELITEM_H
