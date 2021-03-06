/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "imconversationmodel.h"
#include "callagent.h"
#include <TelepathyQt4/AvatarData>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/ReceivedMessage>
#include "custom-event-item.h"
#include "call-event-item.h"
#include "filetransferitem.h"

IMConversationModel::IMConversationModel(const Tp::AccountPtr &account,
    const Tp::ContactPtr &self,
    const Tp::ContactPtr &contact,
    const Tp::TextChannelPtr &channel,
    QObject *parent)
    : MergedModel(parent),
      mCallRunningItem(0),
      mLoggerConversationModel(0),
      mSelf(self),
      mAccount(account),
      mNumDuplicatedMessages(0),
      mCurrentMatch(0),
      mCurrentRowMatch(0),
      mSearching(false)
{
    // if the contact is null, it means we are in 1-to-1 chat
    if (!contact.isNull()) {
        mLoggerConversationModel = new LoggerConversationModel(account, contact, this);
    }

    if (mLoggerConversationModel) {
        addModel(mLoggerConversationModel);
        connect(mLoggerConversationModel,
                SIGNAL(backFetched(int)),
                SLOT(onBackFetched(int)));
        connect(mLoggerConversationModel,
                SIGNAL(backFetchable()),
                SIGNAL(backFetchable()));
        connect(mLoggerConversationModel,
                SIGNAL(backFetched(int)),
                SIGNAL(backFetched(int)));
        // messages in the queue will be reported from both conversation models (logger and session)
        // so basically we will delete the ones in the logger when loaded
        mNumDuplicatedMessages = channel->messageQueue().count();
    }

    mSessionConversationModel = new SessionConversationModel(self, channel, parent);
    if (mSessionConversationModel) {
        addModel(mSessionConversationModel);
        connect(channel.data(),
                SIGNAL(chatStateChanged(Tp::ContactPtr,Tp::ChannelChatState)),
                SLOT(onChatStateChanged(Tp::ContactPtr,Tp::ChannelChatState)));
        connect(mSessionConversationModel,
                SIGNAL(numPendingMessagesChanged()),
                SIGNAL(numPendingMessagesChanged()));
    }

    QHash<int, QByteArray> roles = roleNames();
    roles[StatusRole] = "status";
    roles[DateStringRole] = "dateString";
    roles[FromLoggerRole] = "fromLogger";
    roles[IncomingTransferRole] = "incomingTransfer";
    roles[FileNameRole] = "fileName";
    roles[FilePathRole] = "filePath";
    roles[FileSizeRole] = "fileSize";
    roles[TransferStateRole] = "transferState";
    roles[TransferStateReasonRole] = "transferStateReason";
    roles[PercentTransferredRole] = "percentTransferred";
    roles[BubbleColorRole] = "bubbleColor";
    roles[IncomingEventRole] = "incomingEvent";
    setRoleNames(roles);

    mBubbleColor.clear();
    mBubbleColor.append("blue");
    mBubbleColor.append("green");
    mBubbleColor.append("orange");
    mBubbleColor.append("white");
}

IMConversationModel::~IMConversationModel()
{
}

QString IMConversationModel::searchString() const
{
    return mSearchString;
}

void IMConversationModel::searchByString(const QString &search)
{
    qDebug() << "searchByString: " << search;
    mSearchString = search;
    continueSearch();
}

QVariant IMConversationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const EventItem *eventItem = qobject_cast<const EventItem*>(
                MergedModel::data(index, SessionConversationModel::ItemRole).value<QObject*>());
    const FileTransferItem *fileTransferItem = qobject_cast<const FileTransferItem*>(eventItem);

    switch (role) {
    case AbstractConversationModel::MessageTextRole: {
        QString text = MergedModel::data(index, role).toString();

        text = text.replace(QString("&"), QString("&amp;"));
        text = text.replace(QString("<"), QString("&lt;"));
        text = text.replace(QString(">"), QString("&gt;"));
        text = text.replace(QChar('\n'), QString("<br>"));

        if (!mSearchString.isEmpty() && mMatchesFound.count()) {
            // add underscore if current
            if (currentRowMatch() == index.row()) {
                int index = mMatchesFound.at(mCurrentMatch).column();
                text = text.insert(index + mSearchString.size(), "</u>");
                text = text.insert(index, "<u>");
            }

            // change color to all matches in the line
            QRegExp regExp("(" + QRegExp::escape(mSearchString) + ")", Qt::CaseInsensitive);
            text = text.replace(regExp, "<font color=\"#ff0000\">\\1</font>");
        }
        return text;
    }
    case StatusRole: {
        if (eventItem) {
            return eventItem->sender()->presence().type();
        }

        return QVariant();
    }
    case DateStringRole: {
        if (eventItem) {
            return eventItem->dateTime().date().toString(Qt::DefaultLocaleLongDate);
        }
        return QDate::currentDate().toString(Qt::DefaultLocaleLongDate);
    }
    case FromLoggerRole: {
        if (mLoggerConversationModel && index.row() < mLoggerConversationModel->rowCount()) {
            return true;
        }
        return false;
    }
    case BubbleColorRole: {
        if (!eventItem->sender().isNull()) {
            if (eventItem->sender() == mSelf) {
                return mBubbleColor[mBubbleColor.count() - 1];
            } else {
                return contactColor(eventItem->sender()->id());
            }
        }
        return mBubbleColor[mBubbleColor.count() - 1];
    }
    case IncomingEventRole: {
        if (!eventItem->sender().isNull()) {
            return (eventItem->sender() != mSelf);
        }
        return false;
    }
    // override the type role, so that we can return a custom type for file transfer items
    case IncomingTransferRole: {
        if (fileTransferItem) {
            return fileTransferItem->incomingTransfer();
        }
        return false;
    }
    case FileNameRole: {
        if (fileTransferItem) {
            return fileTransferItem->fileName();
        }
        return QVariant("");
    }
    case FilePathRole: {
        if (fileTransferItem) {
            return fileTransferItem->filePath();
        }
        return QVariant("");
    }
    case FileSizeRole: {
        if (fileTransferItem) {
            return friendlyFileSize(fileTransferItem->fileSize());
        }
        return QVariant();
    }
    case TransferStateRole: {
        if (fileTransferItem) {
            return fileTransferItem->transferState();
        }
        return QVariant();
    }
    case TransferStateReasonRole: {
        if (fileTransferItem) {
            return fileTransferItem->transferStateReason();
        }
        return QVariant();
    }
    case PercentTransferredRole: {
        if (fileTransferItem) {
            return fileTransferItem->percentTransferred();
        }
        return QVariant();
    }
    default:
        return MergedModel::data(index, role);
    }
}

void IMConversationModel::slotResetModel()
{
    beginResetModel();
    endResetModel();
}

void IMConversationModel::onChatStateChanged(const Tp::ContactPtr &contact, Tp::ChannelChatState state)
{
    qDebug() << "IMConversationModel::onChatStateChanged state=" << state;

    // ignore events originating from self
    if (!mSessionConversationModel || contact == mSessionConversationModel->selfContact()) {
        return;
    }

    // if it is a new contact, add it to the list
    if(!mContactsList.contains(contact->id())) {
        mContactsList.append(contact->id());
        QString joinMessage = tr("%1 joined the chat").arg(contact->alias());
        CustomEventItem *item = new CustomEventItem(contact, mSelf,
            QDateTime::currentDateTime(), joinMessage, CustomEventItem::CustomEventUserDefined, this);
        mSessionConversationModel->addItem(item);
    }

    // build the message
    QString message;
    bool running = true;

    mContactsTyping.removeOne(contact);
    switch(state) {
    case Tp::ChannelChatStateComposing: {
        mContactsTyping.append(contact);
        if (mContactsTyping.size() == 1) {
            message = tr("%1 is typing").arg(mContactsTyping.at(0)->alias());
        } else if (mContactsTyping.size() == 2) {
            message = tr("%1 and %2 are typing").arg(mContactsTyping.at(0)->alias()).arg(mContactsTyping.at(1)->alias());
        } else {
            message = tr("Lots of people are typing");
        }
        break;
    }
    case Tp::ChannelChatStateGone: {
        QString goneMessage = tr("%1 left the chat").arg(contact->alias());
        CustomEventItem *item = new CustomEventItem(contact, mSelf,
            QDateTime::currentDateTime(), goneMessage, CustomEventItem::CustomEventUserLeftChat, this);
        mSessionConversationModel->addItem(item);
        mContactsList.removeOne(contact->id());
        break;
    }
    case Tp::ChannelChatStatePaused: {
        message = tr("%1 has paused typing").arg(contact->alias());
        break;
    }
    case Tp::ChannelChatStateActive: {
        //message = tr("%1 is now active").arg(contact->alias());
        break;
    }
    case Tp::ChannelChatStateInactive: {
        message = tr("%1 is now idle").arg(contact->alias());
        break;
    }
    default:
        break;
    }

    // if we have a previous running chat item from the contact, delete it
    foreach(EventItem *item, mChatRunningItems) {
        if(item->sender() == contact) {
            qDebug("previous running item found, deleting");
            mChatRunningItems.removeOne(item);
            mSessionConversationModel->deleteItem(item);
            delete item;
        }
    }

    // add the event message
    if (!message.isEmpty()) {
        CustomEventItem *item = new CustomEventItem(contact, mSelf,
            QDateTime::currentDateTime(), message, CustomEventItem::CustomEventUserDefined, this);
        // remember running messages
        if (running) {
            mChatRunningItems.append(item);
        }
        mSessionConversationModel->addItem(item);
    }
}

void IMConversationModel::onItemChanged()
{
    if (!mSessionConversationModel) {
        return;
    }

    EventItem *item = qobject_cast<EventItem*>(sender());
    if(!item) {
        return;
    }

    QModelIndex idx = mapFromSource(mSessionConversationModel->index(item));
    if (idx.isValid()) {
        emit dataChanged(idx, idx);
    }

    // if it is a new contact, add it to the list
    if(!mContactsList.contains(item->sender()->id())) {
        mContactsList.append(item->sender()->id());
    }
}

void IMConversationModel::notifyCallStatusChanged(CallAgent *callAgent, CallAgent::CallStatus oldCallStatus, CallAgent::CallStatus newCallStatus)
{
    qDebug() << "IMConversationModel::notifyCallStatusChanged: oldCallStatus=" << oldCallStatus << " newCallStatus=" << newCallStatus;

    if (!mSessionConversationModel) {
        return;
    }

    // build the message
    QString message;
    bool running = true;

    // if it is a new contact, add it to the list
    if(!mContactsList.contains(callAgent->contact()->id())) {
        mContactsList.append(callAgent->contact()->id());
    }

    switch (newCallStatus) {
    case CallAgent::CallStatusNoCall:
        /*
        message = tr("Error in call with %1").arg(callAgent->contact()->alias());
        // check if previous was call
        if (oldCallStatus == CallAgent::CallStatusTalking ||
            oldCallStatus == CallAgent::CallStatusHeld ||
            oldCallStatus == CallAgent::CallStatusHangingUp) {
            message = tr("Call with %1 ended").arg(callAgent->contact()->alias());
        } else if (oldCallStatus == CallAgent::CallStatusIncomingCall) {
            message = tr("Missed call from %1").arg(callAgent->contact()->alias());
        }
        */
        running = false;
        break;
    case CallAgent::CallStatusIncomingCall:
        message = tr("%1 is calling you").arg(callAgent->contact()->alias());
        break;
    case CallAgent::CallStatusResourcing:
    case CallAgent::CallStatusConnecting:
        message = tr("Setting up call to %1").arg(callAgent->contact()->alias());
        break;
    case CallAgent::CallStatusRinging:
        message = tr("Calling %1").arg(callAgent->contact()->alias());
        break;
    case CallAgent::CallStatusTalking:
        message = tr("Call with %1 started").arg(callAgent->contact()->alias());
        break;
    case CallAgent::CallStatusHeld:
        message = tr("Call with %1 on hold").arg(callAgent->contact()->alias());
        break;
    case CallAgent::CallStatusHangingUp:
        break;
    }

    // if we have a previous running call item, delete it
    if (mCallRunningItem) {
        mSessionConversationModel->deleteItem(mCallRunningItem);
        mCallRunningItem = 0;
    }

    // add the event message
    if (!running || !message.isEmpty()) {
        Tp::ContactPtr sender;
        Tp::ContactPtr receiver;
        if (callAgent->isRequested()) {
            sender = mSelf;
            receiver = callAgent->contact();
        } else {
            sender = callAgent->contact();
            receiver = mSelf;
        }

        if (running) {
            CustomEventItem *item = new CustomEventItem(sender, receiver,
                QDateTime::currentDateTime(), message, CustomEventItem::CustomEventUserDefined, this);
            mCallRunningItem = item;
            mSessionConversationModel->addItem(item);
       } else {
            Tp::ContactPtr endActor;
            if (callAgent &&
                callAgent->stateReason().actor != 0 &&
                !mAccount.isNull() &&
                !mAccount->connection().isNull() &&
                !mAccount->connection()->contactManager().isNull()) {
                endActor = mAccount->connection()->contactManager()->lookupContactByHandle(callAgent->stateReason().actor);
            }
            Tpy::CallStateChangeReason endReason = (Tpy::CallStateChangeReason) callAgent->stateReason().reason;
            QString detailedEndReason = callAgent->stateReason().DBusReason;
            CallEventItem *item = new CallEventItem(sender, receiver,
                callAgent->startTime(), callAgent->updateCallDuration(),
                endActor, endReason, detailedEndReason, this);
            mSessionConversationModel->addItem(item);
        }
    }
}

void IMConversationModel::notifyCallError(Tp::ContactPtr contact, const QString & errorString)
{
    qDebug() << "IMConversationModel::notifyError: errorString=" << errorString;

    // build the message
    QString message;

    // if it is a new contact, add it to the list
    if(!mContactsList.contains(contact->id())) {
        mContactsList.append(contact->id());
    }

    message = tr("Error in call with %1").arg(contact->alias());

    // FIXME sender ?
    Tp::ContactPtr receiver;
    CustomEventItem *item = new CustomEventItem(contact, receiver,
        QDateTime::currentDateTime(), message, CustomEventItem::CustomEventUserDefined, this);
    mLoggerConversationModel->addItem(item);
}

void IMConversationModel::notifyFileTransfer(Tp::ContactPtr contact, FileTransferAgent *agent,
                                             Tp::FileTransferChannelPtr channel, const QDateTime & dateTime)
{
    Tp::ContactPtr sender = channel->initiatorContact();
    Tp::ContactPtr receiver = (contact != sender) ? contact : channel->connection()->selfContact();
    FileTransferItem *item = new FileTransferItem(sender, receiver, dateTime, agent, channel, this);
    connect(item, SIGNAL(itemChanged()), SLOT(onItemChanged()));

    if (mSessionConversationModel) {
        mSessionConversationModel->addItem(item);
    }
}

QString IMConversationModel::friendlyFileSize(qulonglong size)
{
    QString text;

    // size in bytes
    if (size < 1024) {
        text = tr("%1 bytes").arg(size);
    }
    // size in kbytes
    else if (size < 1048576) {
        text = tr("%1 KB").arg(size / 1024., 0, 'f', 2);
    }
    // size in mbytes
    else if (size < (1048576 * 1024)) {
        text = tr("%1 MB").arg(size / 1048576., 0, 'f', 2);
    }
    // size in gbytes
    else {
        text = tr("%1 GB").arg(size / (1048576. * 1024.), 0, 'f', 2);
    }

    return text;
}

QString IMConversationModel::contactColor(const QString &id) const
{
    // we want the modulo, to iterate among the available bubble colors
    // the last one, white, is reserved for the user
    int index = mContactsList.indexOf(id) % (mBubbleColor.count()-1);
    if(index < 0 || index >= mBubbleColor.count()) {
        index = 0;
    }
    return mBubbleColor[index];
}

bool IMConversationModel::canFetchMoreBack() const
{
    if (mLoggerConversationModel) {
        return mLoggerConversationModel->canFetchMoreBack();
    }

    return false;
}

void IMConversationModel::fetchMoreBack()
{
    if (mLoggerConversationModel) {
        mLoggerConversationModel->fetchMoreBack();
    }
}

void IMConversationModel::clearLog()
{
    mLoggerConversationModel->removeRows(0, mLoggerConversationModel->rowCount());
    mSessionConversationModel->removeRows(0, mSessionConversationModel->rowCount());
}

void IMConversationModel::sendMessage(const QString &text)
{
    if (mSessionConversationModel) {
        mSessionConversationModel->sendMessage(text);
    }
}

void IMConversationModel::disconnectChannelQueue()
{
    if (mSessionConversationModel) {
        mSessionConversationModel->disconnectChannelQueue();
    }
}

void IMConversationModel::connectChannelQueue()
{
    if (mSessionConversationModel) {
        mSessionConversationModel->connectChannelQueue();
    }
}

bool IMConversationModel::channelQueueConnected() const
{
    if (mSessionConversationModel) {
        return mSessionConversationModel->channelQueueConnected();
    }

    return false;
}

int IMConversationModel::numMatchesFound() const
{
    return mMatchesFound.count();
}

bool IMConversationModel::olderActive() const
{
    return mCurrentMatch < (mMatchesFound.count() - 1);
}

bool IMConversationModel::newerActive() const
{
    return mCurrentMatch > 0;
}

bool IMConversationModel::searching() const
{
    return mSearching;
}

int IMConversationModel::currentRowMatch() const
{
    if (mCurrentMatch < (uint) mMatchesFound.count()) {
        return mMatchesFound.at(mCurrentMatch).row();
    }

    return rowCount(QModelIndex()) - 1;
}

void IMConversationModel::newerMatch(void)
{
    if (newerActive()) {
        bool previousOlderActive = olderActive();
        mCurrentMatch--;
        int oldRowMatch = mCurrentRowMatch;
        mCurrentRowMatch = mMatchesFound.at(mCurrentMatch).row();
        emit dataChanged(this->index(oldRowMatch), this->index(oldRowMatch));
        emit dataChanged(this->index(mCurrentRowMatch), this->index(mCurrentRowMatch));
        emit currentRowMatchChanged();
        if (!newerActive()) {
            emit newerActiveChanged();
        }
        if (previousOlderActive != olderActive()) {
            emit olderActiveChanged();
        }
    }
}

void IMConversationModel::olderMatch(void)
{
    if (olderActive()) {
        bool previousNewerActive =  newerActive();
        mCurrentMatch++;
        int oldRowMatch = mCurrentRowMatch;
        mCurrentRowMatch = mMatchesFound.at(mCurrentMatch).row();
        emit dataChanged(this->index(oldRowMatch), this->index(oldRowMatch));
        emit dataChanged(this->index(mCurrentRowMatch), this->index(mCurrentRowMatch));
        emit currentRowMatchChanged();
        if (!olderActive()) {
            emit olderActiveChanged();
        }
        if (previousNewerActive != newerActive()) {
            emit newerActiveChanged();
        }
    }
}

void IMConversationModel::calculateMatches(void)
{
    // get the current matches to clear them later
    // if cleared now, they could be refreshed before the new matches are found
    QList<QModelIndex> oldMatches;
    int lastIndex = -1;
    foreach(QModelIndex index, mMatchesFound) {
        if (lastIndex != index.row()) {
            oldMatches.append(index);
        }
        lastIndex = index.row();
    }

    mMatchesFound.clear();
    for(int i = rowCount(QModelIndex()) - 1;i >= 0;i--) {
        QModelIndex rowIndex = index(i,0,QModelIndex());
        QString text = MergedModel::data(rowIndex, AbstractConversationModel::MessageTextRole).toString();
        int fromIndex = -1;
        // NOTE matched are counted backwards in the string
        do {
            fromIndex = text.lastIndexOf(mSearchString, fromIndex, Qt::CaseInsensitive);
            if (fromIndex != -1) {
                mMatchesFound.append(this->index(i, fromIndex));
                fromIndex--;
            }
        } while(fromIndex != -1);
    }

    if (mMatchesFound.count()) {
        mCurrentRowMatch = mMatchesFound.at(0).row();
    } else {
        mCurrentRowMatch = rowCount(QModelIndex()) - 1;
    }

    // clear new matches (otherwise they will not get selected)
    foreach(QModelIndex index, mMatchesFound) {
        if (lastIndex != index.row()) {
            emit dataChanged(index, index);
        }
        lastIndex = index.row();
    }

    // refresh old matches
    foreach(QModelIndex index, oldMatches) {
        bool emitted = false;
        foreach(QModelIndex newIndex, mMatchesFound) {
            if (index.row() == newIndex.row()) {
                emitted = true;
            }
        }

        // only refresh those rows that haven't been already refreshed
        if (!emitted) {
            emit dataChanged(index, index);
        }
    }

    emit numMatchesFoundChanged();
    emit currentRowMatchChanged();
    emit olderActiveChanged();
    emit newerActiveChanged();
}

void IMConversationModel::continueSearch()
{
    if (mSearchString.isEmpty()) {
        mMatchesFound.clear();
        mSearching = false;
        emit searchingChanged();
        reset();
        return;
    }

    calculateMatches();

    if (canFetchMoreBack()) {
        if (!mSearching) {
            mSearching = true;
            emit searchingChanged();
        }
        if (!mLoggerConversationModel->backFetching()) {
            mLoggerConversationModel->fetchMoreBack();
        }
    } else {
        mSearching = false;
        emit searchingChanged();
    }
}

void IMConversationModel::onBackFetched(int numItems)
{
    qDebug() << "IMConversationModel::onBackFetched" << mNumDuplicatedMessages;
    if (mLoggerConversationModel && mNumDuplicatedMessages > 0) {
        int numToDelete = mNumDuplicatedMessages;
        if (numToDelete > mLoggerConversationModel->rowCount()) {
            numToDelete = mLoggerConversationModel->rowCount();
        }
        int numRow = mLoggerConversationModel->rowCount() - numToDelete;
        qDebug() << "before " << mLoggerConversationModel->rowCount();
        mLoggerConversationModel->removeRows(numRow, numToDelete);
        qDebug() << "after " << mLoggerConversationModel->rowCount();
        mNumDuplicatedMessages -= numToDelete;
    }

    // if there is a search going on then update search
    if (!mSearchString.isEmpty() && numItems > 0) {
        continueSearch();
    }
}

int IMConversationModel::numPendingMessages() const
{
    if (mSessionConversationModel) {
        return mSessionConversationModel->numPendingMessages();
    }

    return 0;
}
