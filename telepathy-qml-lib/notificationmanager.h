/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QDateTime>

class MNotification;

class NotificationItem
{
public:
    enum NotificationType {
        PendingChatMessage,
        PendingGroupChatMessage,
        MissedCall,
        MissedVideoCall,
        IncomingFileTransfer,
        IncomingCall
    };

    ~NotificationItem();
    NotificationType type;
    QString accountId;
    // the chatId can be either a contact id or a group chat id
    QString chatId;

    // the message will only be used by the pending text message
    QString message;

    QSharedPointer<MNotification> item;
};

class NotificationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentAccount READ currentAccount WRITE setCurrentAccount)
    Q_PROPERTY(QString currentContact READ currentContact WRITE setCurrentContact)
    Q_PROPERTY(QString currentGroupChat READ currentGroupChat() WRITE setCurrentGroupChat)
    Q_PROPERTY(bool chatActive READ chatActive WRITE setChatActive)
    Q_PROPERTY(bool applicationActive READ applicationActive WRITE setApplicationActive)
public:
    explicit NotificationManager(QObject *parent = 0);

    QString currentAccount() const;
    QString currentContact() const;
    QString currentGroupChat() const;
    bool chatActive() const;
    bool applicationActive() const;

    Q_INVOKABLE void notifyPendingMessage(const QString &accountId,
                                          const QString &contactId,
                                          const QString &contactAlias,
                                          const QDateTime &time,
                                          const QString &message);

    Q_INVOKABLE void notifyPendingGroupMessage(const QString &accountId,
                                               const QString &groupChatId,
                                               const QString &contactAlias,
                                               const QDateTime &time,
                                               const QString &message);

    Q_INVOKABLE void notifyMissedCall(const QString &accountId,
                                      const QString &contactId,
                                      const QString &contactAlias,
                                      const QDateTime &time);

    Q_INVOKABLE void notifyMissedVideoCall(const QString &accountId,
                                           const QString &contactId,
                                           const QString &contactAlias,
                                           const QDateTime &time);

    Q_INVOKABLE void notifyIncomingFileTransfer(const QString &accountId,
                                                const QString &contactId,
                                                const QString &contactAlias,
                                                const QDateTime &time,
                                                const QString &fileName);

    Q_INVOKABLE void clear();

public slots:
    void setCurrentAccount(const QString &account);
    void setCurrentContact(const QString &contact);
    void setCurrentGroupChat(const QString &groupChat);
    void setChatActive(bool active);
    void setApplicationActive(bool active);

protected:
    void processNotifications();
    // chatId can be either a contact id or a group chat id
    void placeNotification(NotificationItem::NotificationType type,
                           const QString &accountId,
                           const QString &chatId,
                           const QString &contactAlias,
                           const QDateTime &time,
                           const QString &message = QString::null);

private:
    QString mCurrentAccount;
    QString mCurrentContact;
    QString mCurrentGroupChat;
    int mChatActive;
    bool mApplicationActive;
    QList<NotificationItem> mNotifications;
};

#endif // NOTIFICATIONMANAGER_H
