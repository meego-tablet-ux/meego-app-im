/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "imfeedmodel.h"
#include <actions.h>
#include <TelepathyQt4/AvatarData>
#include <TelepathyQt4/Channel>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/Feature>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Types>

#include <QDebug>

#include <QDateTime>
#include <QStringList>

IMFeedModel::IMFeedModel(PanelsChannelObserver *observer, Tp::AccountPtr account, QObject *parent)
    : McaFeedModel(parent),
      mObserver(observer),
      mAccount(account),
      mAccountId(account->uniqueIdentifier())
{
    if(observer) {
        connect(observer,
                SIGNAL(newTextChannel(QString,Tp::TextChannelPtr)),
                SLOT(onNewTextChannel(QString,Tp::TextChannelPtr)));
        connect(observer,
                SIGNAL(newCallChannel(QString,Tpy::CallChannelPtr)),
                SLOT(onNewCallChannel(QString,Tpy::CallChannelPtr)));
        connect(observer,
                SIGNAL(newFileTransferChannel(QString,Tp::IncomingFileTransferChannelPtr)),
                SLOT(onNewFileTransferChannel(QString,Tp::IncomingFileTransferChannelPtr)));
    } else {
        qDebug() << "IMFeedModel::IMFeedModel(): The PanelsChannelObserver is not valid. Signals for new channels will not be received.";
    }
    connect(mAccount.data(),
            SIGNAL(connectionChanged(Tp::ConnectionPtr)),
            SLOT(onConnectionChanged(Tp::ConnectionPtr)));
    if (!mAccount->connection().isNull()
            && mAccount->connection()->isValid()) {
        onConnectionChanged(mAccount->connection());
    }

    // get existing channels
    mObserver->emitExistingChannels(mAccountId);
}

IMFeedModel::~IMFeedModel()
{
}

int IMFeedModel::rowCount(const QModelIndex &parent) const
{
    // The current UI plans only show at most 30 items from your model in
    //   a panel, and more likely 10 or fewer. So if there are performance
    //   improvements you can make by pruning down the queries you make to
    //   your back end, that is a guideline. However, if setSearchText is
    //   called, then you should try to search as widely as you can for
    //   matching text.
    // If you are given limit hints with the setHints function, you can also
    //   use those to help tune how much information you bother exposing
    //   because that will tell you how much will really be used.
    // When there is no search text set, your model should also restrict its
    //   queries to items from the last 30 days, if that is helpful for
    //   performance. Older items may be ignored.
    Q_UNUSED(parent);
    return mItems.count();
}

QVariant IMFeedModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row >= mItems.count())
        return QVariant();

    IMFeedModelItem *item = mItems.at(row);
    switch (role) {
    case RequiredTypeRole: {
        // Types defined currently are "content", "picture", and "request";
        //   see emailmodel.h for more information.
        int type = item->itemType();
        if (type == MessageType ||
            type == GroupMessageType ||
            type == InformationType) {
            return QString("content");
        } else if (type == RequestType) {
            return QString("request");
        } else {
            return QVariant();
        }
        break;
    }

    case RequiredUniqueIdRole:
        // RequiredUniqueIdRole is required, and the id you provide must be
        //   unique across this service and remain consistent for this data
        //   item across reboots, etc.
        // It doesn't have to be a true Uuid, although that would be fine.
        return item->uniqueId();

    case RequiredTimestampRole:
        // Each item should have a timestamp associated, and the model
        //   should be sorted by timestamp, most recent first.
        return item->timestamp();

    case GenericTitleRole:
        // The basic view for a content item contains two text fields,
        //   this one for the item title, plus the content preview (below).
        return item->contactName();

    case GenericContentRole:
        // The basic view for a content item contains two text fields,
        //   the item title (above), plus this content preview. The string
        //   should be limited to a few lines of text, maybe 160 or 256
        //   characters. More will be ignored.
        return item->message();

    case GenericAvatarUrlRole:
        // This field is for a thumbnail image of the message sender.
        return item->avatarUrl();

    case CommonActionsRole:
        // This is required to respond to user actions on your data item.
        return QVariant::fromValue<McaActions*>(item->actions());

    case GenericPictureUrlRole:
        return QString();

    case GenericRelevanceRole:
        return item->relevance();

    default:
        // There are a few other optional roles... you can provide a
        //   float "relevance" of the item from 0.0 (low) to 1.0 (high);
        //   for future use. You can set custom text for accept/reject
        //   buttons (localized!) for a "request" type item. You can
        //   provide a "picture url" if there is picture associated with
        //   the item, such as a photo upload notification.
        return QVariant();
    };
}

void IMFeedModel::performAction(QString action, QString uniqueid)
{
    // This is a slot connected to the signal for all items, so we need to
    //   figure out which one it applies to based on the uniqueid paramater.
    //   Alternately, you could connect to a slot on an individual item, if
    //   you make it a QObject, and you don't need that parameter.
    qDebug() << "Action" << action << "called for im" << uniqueid;

    //look for the model item
    foreach (IMFeedModelItem *item, mItems) {
        if (item->uniqueId() == uniqueid) {
            if (action == "accept") {
                acceptContact(item->contactId());
                removeItem(item);
                break;
            } else if (action == "reject") {
                rejectContact(item->contactId());
                removeItem(item);
                break;
            } else if (action == "default") {
                QString cmd;
                QString parameter;
                switch (item->itemType()) {
                   case MessageType: {
                       cmd = QString("show-chat");
                       parameter = QString(mAccountId + "&" + item->contactId());
                       break;
                   }
                   case GroupMessageType: {
                       cmd = QString("show-group-chat");
                       parameter = QString(mAccountId + "&" + item->channelPath());
                       break;
                   }
                   case RequestType:
                   case InformationType:
                   default:
                   {
                       cmd = QString("show-contacts");
                       parameter = mAccountId;
                       break;
                   }
                }
                QString executable("meego-qml-launcher");
                QStringList parameters;
                parameters << "--app" << "meego-app-im";
                parameters << "--opengl" << "--fullscreen";
                parameters << "--cmd" << cmd;
                parameters << "--cdata" << parameter;
                QProcess::startDetached(executable, parameters);
            }
        }
    }
}

void IMFeedModel::onMessageReceived(const Tp::ReceivedMessage &message)
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    onMessageReceived(message, textChannel);
}

void IMFeedModel::onMessageReceived(const Tp::ReceivedMessage &message, Tp::TextChannelPtr &textChannel)
{
    FeedType type = MessageType;
    QString channelPath;
    Tp::ContactPtr contact = message.sender();

    if (textChannel->targetHandleType() == Tp::HandleTypeGroup ||
        textChannel->targetHandleType() == Tp::HandleTypeRoom) {
        type = GroupMessageType;
        channelPath = textChannel->objectPath();
    }

    // do not log old messages
    if (message.isRescued() || message.isScrollback()) {
        return;
    }

    // do not log delivery reports
    if(message.messageType() == Tp::ChannelTextMessageTypeDeliveryReport) {
        return;
    }

    //this is to allow parsing later on
    QString token = QString(MessageType + "&" + mAccountId + "&" + message.sender()->id() + "&" + message.messageToken());

    if (!contact->actualFeatures().contains(Tp::Contact::FeatureAlias)
            || !contact->actualFeatures().contains(Tp::Contact::FeatureAvatarData)) {
        //upgrade contacts
        Tp::Features features;
        features << Tp::Contact::FeatureAlias
            << Tp::Contact::FeatureAvatarData;
        QList<Tp::ContactPtr>  contacts;
        contacts.append(contact);

        connect(contact->manager()->upgradeContacts(contacts, features),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onContactUpgraded(Tp::PendingOperation*)));
    }

    IMFeedModelItem *item = new IMFeedModelItem(mAccount, contact, message.text(), message.received(),
                                                new McaActions(), type, token, textChannel->objectPath());
    connect(item->actions(), SIGNAL(standardAction(QString,QString)),
            this, SLOT(performAction(QString,QString)));

    insertItem(item);
}

void IMFeedModel::onNewTextChannel(const QString &accountId, const Tp::TextChannelPtr &textChannel)
{
    if (accountId == mAccountId) {
        // enable the features we need to receive incoming messages
        connect(textChannel->becomeReady(Tp::Features()
                        << Tp::TextChannel::FeatureCore
                        << Tp::TextChannel::FeatureMessageQueue
                        << Tp::TextChannel::FeatureMessageCapabilities
                        << Tp::TextChannel::FeatureMessageSentSignal),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onTextChannelReady(Tp::PendingOperation*)));
    }
}

void IMFeedModel::onNewCallChannel(const QString &accountId, const Tpy::CallChannelPtr &callChannel)
{
    if (accountId == mAccountId) {
        // enable the features we need to receive incoming messages
        connect(callChannel->becomeReady(Tp::Features()
                        << Tpy::CallChannel::FeatureCore
                        << Tpy::CallChannel::FeatureConferenceInitialInviteeContacts),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onCallChannelReady(Tp::PendingOperation*)));
    }
}

void IMFeedModel::onNewFileTransferChannel(const QString &accountId, const Tp::IncomingFileTransferChannelPtr &fileTransferChannel)
{
    if (accountId == mAccountId) {
        // enable the features we need to receive incoming messages
        connect(fileTransferChannel->becomeReady(Tp::Features()
                        << Tp::IncomingFileTransferChannel::FeatureCore),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onFileTransferChannelReady(Tp::PendingOperation*)));
    }
}

void IMFeedModel::onContactUpgraded(Tp::PendingOperation *op)
{
    if (op->isError()) {
        qWarning() << "Contacts cannot be upgraded";
        return;
    }
}

void IMFeedModel::insertItem(IMFeedModelItem *item)
{
    qDebug("item inserted");
    beginInsertRows(QModelIndex(), 0, 0);
    mItems.prepend(item);
    connect(item, SIGNAL(itemChanged(IMFeedModelItem*)),
            SLOT(onModelItemChanged(IMFeedModelItem*)));
    endInsertRows();
}

void IMFeedModel::removeItem(IMFeedModelItem *item)
{
    int row = mItems.indexOf(item);
    beginRemoveRows(QModelIndex(), row, row);
    mItems.removeOne(item);
    endRemoveRows();
}

void IMFeedModel::onItemChanged(int row)
{
    QModelIndex index = createIndex(row, 0, mItems.at(row));
    emit dataChanged(index, index);
}

void IMFeedModel::onModelItemChanged(IMFeedModelItem *item)
{
    int index = mItems.indexOf(item);
    if( index >= 0) {
        onItemChanged(index);
    }
}

void IMFeedModel::onPresencePublicationRequested(const Tp::Contacts &contacts)
{
    // Add new items
    foreach (Tp::ContactPtr contact, contacts) {
        QString token = QString(InformationType + "&" + mAccountId + "&" + contact->id() + QDateTime::currentDateTime().toString(Qt::ISODate));
        contact->setProperty("feedtype", RequestType);
        contact->setProperty("messagetoken", token);
        IMFeedModelItem *item = new IMFeedModelItem(mAccount,
                                                    contact,
                                                    tr("Add as friend?"),
                                                    QDateTime::currentDateTime(),
                                                    new McaActions(),
                                                    RequestType,
                                                    token);
        connect(item->actions(), SIGNAL(standardAction(QString,QString)),
                this, SLOT(performAction(QString,QString)));
        insertItem(item);
    }
}

Tp::AccountPtr IMFeedModel::account(void) const
{
    return mAccount;
}

void IMFeedModel::onTextChannelReady(Tp::PendingOperation *op)
{
    qDebug("IMFeedModel::onTextChannelReady: channel ready");

    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);
    if (!pr || pr->isError()) {
        qDebug() << "IMFeedModel::onTextChannelReady: error "
                 << (op ? op->errorName() : "")
                 << (op ? op->errorMessage() : "");
        return;
    }

    Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(pr->object());
    if (textChannel.isNull()) {
        qDebug() << "IMFeedModel::onTextChannelReady: channel invalid";
        return;
    }

    //flush the queue and enter all messages into the model
    // display messages already in queue
    qDebug("message queue: %d", textChannel->messageQueue().count());
    foreach (Tp::ReceivedMessage message, textChannel->messageQueue()) {
        onMessageReceived(message, textChannel);
    }

    //connect to incoming messages
    connect(textChannel.data(),
                SIGNAL(messageReceived(Tp::ReceivedMessage)),
                SLOT(onMessageReceived(Tp::ReceivedMessage)));
}

void IMFeedModel::onCallChannelReady(Tp::PendingOperation *op)
{
    qDebug("IMFeedModel::onCallChannelReady: channel ready");

    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);
    if (!pr || pr->isError()) {
        qDebug() << "IMFeedModel::onCallChannelReady: error "
                 << (op ? op->errorName() : "")
                 << (op ? op->errorMessage() : "");
        return;
    }

    Tpy::CallChannelPtr callChannel = Tpy::CallChannelPtr::dynamicCast(pr->object());
    if (callChannel.isNull()) {
        qDebug() << "IMFeedModel::onCallChannelReady: channel invalid";
        return;
    }

    createNewChannelItem(callChannel, CallType);
}

void IMFeedModel::onFileTransferChannelReady(Tp::PendingOperation *op)
{
    qDebug("IMFeedModel::onFileTransferChannelReady: channel ready");

    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);
    if (!pr || pr->isError()) {
        qDebug() << "IMFeedModel::onFileTransferChannelReady: error "
                 << (op ? op->errorName() : "")
                 << (op ? op->errorMessage() : "");
        return;
    }

    Tp::IncomingFileTransferChannelPtr fileTransferChannel = Tp::IncomingFileTransferChannelPtr::dynamicCast(pr->object());
    if (fileTransferChannel.isNull()) {
        qDebug() << "IMFeedModel::onFileTransferChannelReady: channel invalid";
        return;
    }

    createNewChannelItem(fileTransferChannel, FileTransferType);

}

void IMFeedModel::createNewChannelItem(const Tp::ChannelPtr &channel, const FeedType &type)
{

    Tp::ContactPtr contact;
    foreach(Tp::ContactPtr channelContact, channel->groupContacts().toList()) {
        if(contact != channel->groupSelfContact()) {
            contact = channelContact;
            break;
        }
    }

    if(contact.isNull()) {
        return;
    }

    QString token = QString(type + "&" + mAccountId + "&" + contact->id() + QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!contact->actualFeatures().contains(Tp::Contact::FeatureAlias)
            || !contact->actualFeatures().contains(Tp::Contact::FeatureAvatarData)) {
        //upgrade contacts
        Tp::Features features;
        features << Tp::Contact::FeatureAlias
            << Tp::Contact::FeatureAvatarData;
        QList<Tp::ContactPtr>  contacts;
        contacts.append(contact);

        connect(contact->manager()->upgradeContacts(contacts, features),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onContactUpgraded(Tp::PendingOperation*)));
    }
    QString alias = contact->alias();
    QString message;

    switch(type) {
    case CallType:
        message = tr("Incoming call from %1").arg(alias);
        break;
    case FileTransferType:
        message = tr("Incoming file transfer from %1").arg(contact->alias());
        break;
    default:
        message.clear();
        break;
    }

    IMFeedModelItem *item = new IMFeedModelItem(mAccount, contact, message, QDateTime::currentDateTime(),
                                                new McaActions(), MessageType, token);
    connect(item->actions(), SIGNAL(standardAction(QString,QString)),
            this, SLOT(performAction(QString,QString)));

    insertItem(item);
}

void IMFeedModel::onAllKnownContactsChanged(const Tp::Contacts &contactsAdded,
                                            const Tp::Contacts &contactsRemoved,
                                            const Tp::Channel::GroupMemberChangeDetails &details)
{
    Q_UNUSED(contactsRemoved);
    Q_UNUSED(details);

    //go over each new contact and insert the feedtype property
    //that way, when its publish state changes to yes, we can detect them and add only those to the model
    foreach (Tp::ContactPtr contact, contactsAdded) {
        contact->setProperty("feedtype", InformationType);

        connect(contact.data(),
                SIGNAL(publishStateChanged(Tp::Contact::PresenceState, QString)),
                SLOT(onPublishStateChanged(Tp::Contact::PresenceState)));
    }
}

void IMFeedModel::onPublishStateChanged(Tp::Contact::PresenceState state)
{
    //only process if it has been added
    if (state == Tp::Contact::PresenceStateYes) {
        Tp::Contact *contact = qobject_cast<Tp::Contact *>(sender());
        Tp::ContactPtr contactPtr(contact);
        if (!contact->actualFeatures().contains(Tp::Contact::FeatureAlias)
                || !contact->actualFeatures().contains(Tp::Contact::FeatureAvatarData)) {
            Tp::Features features;
            features << Tp::Contact::FeatureAlias
                << Tp::Contact::FeatureAvatarData;
            QList<Tp::ContactPtr>  contacts;
            contacts.append(contactPtr);

            //upgrade the contact
            connect(contact->manager()->upgradeContacts(contacts, features),
                    SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onContactUpgraded(Tp::PendingOperation*)));
        }

        // remove the request if it is still there
        removeExistingRequest(contact->id());

        // prepare the item to insert
        //: Message indicating the contact has been added
        QString messageText = tr("has been added as contact");
        QString token = QString(InformationType + "&" + mAccountId + "&" + contact->id() + QDateTime::currentDateTime().toString(Qt::ISODate));
        IMFeedModelItem *item = new IMFeedModelItem(mAccount,
                                                    contactPtr,
                                                    messageText,
                                                    QDateTime(),
                                                    new McaActions(),
                                                    InformationType,
                                                    token);
        connect(item->actions(), SIGNAL(standardAction(QString,QString)),
                this, SLOT(performAction(QString,QString)));
        insertItem(item);
    }
}

void IMFeedModel::removeExistingRequest(const QString &contactId)
{
    foreach (IMFeedModelItem *item, mItems) {
        // if it is a request belonging to the contact, remove it
        if (item->contactId() == contactId
                && item->itemType() == RequestType) {
            removeItem(item);
        }
    }
}

void IMFeedModel::onConnectionChanged(const Tp::ConnectionPtr &conn)
{
    if (!conn.isNull() && conn->isValid()) {
        if (conn->status() == Tp::ConnectionStatusConnected) {
            onConnectionAvailable();
        } else {
            connect(conn.data(), SIGNAL(statusChanged(Tp::ConnectionStatus)),
                    SLOT(onConnectionStatusChanged(Tp::ConnectionStatus)));
        }
    }
}

void IMFeedModel::onConnectionStatusChanged(const Tp::ConnectionStatus status)
{
    if (status == Tp::ConnectionStatusConnected) {
        onConnectionAvailable();
    }
}

void IMFeedModel::onConnectionAvailable()
{
    qDebug() << "IMFeedModel::onConnectionAvailable: " << mAccount->uniqueIdentifier();
    Tp::ContactManagerPtr manager = mAccount->connection()->contactManager();
    connect(manager.data(), SIGNAL(presencePublicationRequested(Tp::Contacts)),
            SLOT(onPresencePublicationRequested(Tp::Contacts)));
    connect(manager.data(), SIGNAL(allKnownContactsChanged(Tp::Contacts,Tp::Contacts,Tp::Channel::GroupMemberChangeDetails)),
            SLOT(onAllKnownContactsChanged(Tp::Contacts,Tp::Contacts,Tp::Channel::GroupMemberChangeDetails)));

    // Look for friend requests and listen for publish changes
    Tp::Contacts contacts = manager->allKnownContacts();
    QList<Tp::ContactPtr> friendRequests;
    foreach (Tp::ContactPtr contact, contacts) {
        // if a friend request
        if (contact->publishState() == Tp::Contact::PresenceStateAsk) {
            friendRequests.append(contact);
        }

        // connect to publish changes
        connect(contact.data(), SIGNAL(publishStateChanged(Tp::Contact::PresenceState,QString)),
                SLOT(onPublishStateChanged(Tp::Contact::PresenceState)));
    }
    // Add the friend requests to the model
    if (friendRequests.count() > 0) {
        onPresencePublicationRequested(Tp::Contacts().fromList(friendRequests));
    }
}


void IMFeedModel::acceptContact(QString contactId)
{
    if (mAccount->connection().isNull()) {
        Tp::ContactManagerPtr manager = mAccount->connection()->contactManager();

        Tp::Contacts contacts = manager->allKnownContacts();
        QList<Tp::ContactPtr> contactsToAppend;
        QList<Tp::ContactPtr>  contactsList = contacts.toList();
        foreach (Tp::ContactPtr contact, contactsList) {
            if (contact->id() == contactId) {
                contactsToAppend.append(contact);
                break;
            }
        }
        manager->authorizePresencePublication(contactsToAppend);
    }
}

void IMFeedModel::rejectContact(QString contactId)
{
    if (mAccount->connection().isNull()) {
        Tp::ContactManagerPtr manager = mAccount->connection()->contactManager();

        Tp::Contacts contacts = manager->allKnownContacts();
        QList<Tp::ContactPtr> contactsToAppend;
        QList<Tp::ContactPtr>  contactsList = contacts.toList();
        foreach (Tp::ContactPtr contact, contactsList) {
            if (contact->id() == contactId) {
                contactsToAppend.append(contact);
                break;
            }
        }
        manager->removePresencePublication(contactsToAppend);
    }
}
