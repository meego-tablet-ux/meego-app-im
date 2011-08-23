/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_logger_conversation_model_h_HEADER_GUARD_
#define _Telepathy_Models_logger_conversation_model_h_HEADER_GUARD_

#include "global.h"
#include "abstract-conversation-model.h"
#include <TelepathyQt4/Types>
#include <TelepathyLoggerQt4/PendingOperation>

class TELEPATHY_MODELS_EXPORT LoggerConversationModel
    : public AbstractConversationModel
{
    Q_OBJECT
    Q_PROPERTY(bool backFetching READ backFetching NOTIFY backFetchingChanged)

public:
    LoggerConversationModel(const Tp::AccountPtr &account, const Tp::ContactPtr &contact, QObject *parent = 0);
    virtual ~LoggerConversationModel();

    /*
        Next methods are very similar to canFetchMore / fetchMore already present in Qt models.
        The difference is that already existing methods work just for scrolling forwards.
        For scrolling backwards, neither the views nor the models have a way to support
        on-demand feeding of information, since it is not a normal use case.
        However for chat history, normally you see the last portion (newest information) and
        you might want to scroll backwards to feed more data in in reverse chronological order.

        backFetching is a new method that checks if there is an already exiting request being
        executed. We prevent to execute multiple requests concurrently.
     */
    Q_INVOKABLE virtual bool canFetchMoreBack() const;
    Q_INVOKABLE virtual void fetchMoreBack();
    virtual bool backFetching() const;

    virtual bool canFetchMore(const QModelIndex & index) const;
    virtual void fetchMore(const QModelIndex & index);

Q_SIGNALS:
    void backFetchable();
    void backFetched(int numItems);
    void backFetchingChanged() const;

private Q_SLOTS:
    void onPendingDatesFinished(Tpl::PendingOperation *op);
    void onPendingEventsFinished(Tpl::PendingOperation *op);

 private:
    void setup() const;
    void fetchDate(const QDate &date) const;

    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif
