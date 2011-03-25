/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef IMCONVERSATIONMODEL_H
#define IMCONVERSATIONMODEL_H

#include <TelepathyQt4/Contact>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Types>
#include <TelepathyQt4Yell/Models/ConversationModel>
#include <TelepathyQt4Yell/Models/ConversationItem>
#include "callagent.h"

class FileTransferAgent;

class IMConversationModel : public Tpy::ConversationModel
{
    Q_OBJECT

public:
    enum Role {
        IncludeSearchRole = Qt::UserRole + 100,
        StatusRole,

        // file transfer related roles
        IncomingTransferRole,
        FileNameRole,
        FileSizeRole,
        TransferStateRole,
        TransferStateReasonRole,
        PercentTransferredRole,
        BubbleColorRole
    };

    explicit IMConversationModel(const Tp::ContactPtr &self, const Tp::TextChannelPtr &channel, QObject *parent = 0);
    virtual ~IMConversationModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    QString searchString(void) const;
    void notifyCallStatusChanged(Tp::ContactPtr contact, CallAgent::CallStatus oldCallStatus, CallAgent::CallStatus newCallStatus);
    void notifyCallError(Tp::ContactPtr contact, const QString & errorString);
    void notifyFileTransfer(Tp::ContactPtr contact, FileTransferAgent *agent, Tp::FileTransferChannelPtr channel);

    static QString friendlyFileSize(qulonglong size);

public Q_SLOTS:
    void onSearchByString(const QString &search);
    void slotResetModel(void);

protected Q_SLOTS:
    virtual void onChatStateChanged(const Tp::ContactPtr &contact, Tp::ChannelChatState state);
    void onItemChanged();

protected:
    QString contactColor(const QString &id) const;

private:
    QString mSearchString;
    QList<Tp::ContactPtr> mContactsTyping;
    QDateTime mContactsTypingTime;
    QList<Tpy::ConversationItem*> mChatRunningItems;
    Tpy::ConversationItem *mCallRunningItem;
    QStringList mContactsList;
    QStringList mBubbleColor;
};

#endif // IMCONVERSATIONMODEL_H
