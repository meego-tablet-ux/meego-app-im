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
#include <TelepathyQt4Yell/Models/ConversationItem>
#include "filetransferitem.h"

IMConversationModel::IMConversationModel(const Tp::AccountPtr &account,
    const Tp::ContactPtr &self,
    const Tp::ContactPtr &contact,
    const Tp::TextChannelPtr &channel,
    QObject *parent)
    : MergedModel(parent),
      mCallRunningItem(0)
{
    mLoggerConversationModel = new Tpl::LoggerConversationModel(account, contact, this);
    if (mLoggerConversationModel) {
        addModel(mLoggerConversationModel);
        connect(mLoggerConversationModel,
                SIGNAL(backFetchable()),
                SIGNAL(backFetchable()));
        connect(mLoggerConversationModel,
                SIGNAL(backFetched(int)),
                SIGNAL(backFetched(int)));
    }

    mSessionConversationModel = new Tpy::SessionConversationModel(self, channel, parent);
    if (mSessionConversationModel) {
        addModel(mSessionConversationModel);
    }

    QHash<int, QByteArray> roles = roleNames();
    roles[IncludeSearchRole] = "includeSearch";
    roles[StatusRole] = "status";
    roles[IncomingTransferRole] = "incomingTransfer";
    roles[FileNameRole] = "fileName";
    roles[FileSizeRole] = "fileSize";
    roles[TransferStateRole] = "transferState";
    roles[TransferStateReasonRole] = "transferStateReason";
    roles[PercentTransferredRole] = "percentTransferred";
    roles[BubbleColorRole] = "bubbleColor";
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

void IMConversationModel::onSearchByString(const QString &search)
{
    mSearchString = search;
    slotResetModel();
}

QVariant IMConversationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const FileTransferItem *item = qobject_cast<const FileTransferItem*>(
                MergedModel::data(index, Tpy::SessionConversationModel::ItemRole).value<QObject*>());
    const Tpy::ConversationItem *conversationItem = qobject_cast<const Tpy::ConversationItem*>(
                MergedModel::data(index, Tpy::SessionConversationModel::ItemRole).value<QObject*>());

    switch (role) {
    case Tpy::SessionConversationModel::TextRole: {
        QString text = MergedModel::data(index, role).toString();

        if (!mSearchString.isEmpty()) {
            text = text.replace(QString("<"), QString("&lt;"));
            text = text.replace(QString(">"), QString("&gt;"));
            text = text.replace(QChar('\n'), QString("<br>"));

            int index = text.indexOf(mSearchString, Qt::CaseInsensitive);
            QString prefix = text.left(index);
            QString middle = text.mid(index, mSearchString.length());
            QString suffix = text.mid(index + mSearchString.length());
            QString newText = QString(prefix + "<font color=\"#ff0000\">" + middle + "</font>" + suffix);
            newText = newText.replace(QString("&"), QString("&amp;"));
            return QVariant(newText);
        } else {
            text = text.replace(QString("&"), QString("&amp;"));
            text = text.replace(QString("<"), QString("&lt;"));
            text = text.replace(QString(">"), QString("&gt;"));
            text = text.replace(QChar('\n'), QString("<br>"));
            return text;
        }
    }
    case IncludeSearchRole: {
        if (!mSearchString.isEmpty()) {
            return QVariant(MergedModel::data(index, Qt::UserRole).toString().contains(mSearchString, Qt::CaseInsensitive));
        } else {
            return QVariant(true);
        }
    }
    case StatusRole: {
        if (conversationItem) {
            return conversationItem->contact()->presence().type();
        }

        return QVariant();
    }
    case BubbleColorRole: {
        if (item) {
            if(item->incomingTransfer()) {
                return contactColor(item->contact()->id());
            } else {
                return mBubbleColor[mBubbleColor.count()-1];
            }
        }
        else if (conversationItem) {
            if(conversationItem->type() == Tpy::ConversationItem::OUTGOING_MESSAGE) {
                return mBubbleColor[mBubbleColor.count()-1];
            } else {
                QString id = conversationItem->contact()->id();
                return contactColor(id);
            }
        }
        return mBubbleColor[mBubbleColor.count()-1];
    }
    // override the type role, so that we can return a custom type for file transfer items
    case Tpy::SessionConversationModel::TypeRole: {
        if (item) {
            return (item->incomingTransfer() ? "incoming_file_transfer" :
                                               "outgoing_file_transfer");
        }
        return MergedModel::data(index, role);
    }
    case IncomingTransferRole: {
        if (item) {
            return item->incomingTransfer();
        }
        return false;
    }
    case FileNameRole: {
        if (item) {
            return item->fileName();
        }
        return QVariant("");
    }
    case FileSizeRole: {
        if (item) {
            return friendlyFileSize(item->fileSize());
        }
        return QVariant();
    }
    case TransferStateRole: {
        if (item) {
            return item->transferState();
        }
        return QVariant();
    }
    case TransferStateReasonRole: {
        if (item) {
            return item->transferStateReason();
        }
        return QVariant();
    }
    case PercentTransferredRole: {
        if (item) {
            return item->percentTransferred();
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
        message = tr("%1 has left the conversation").arg(contact->alias());
        running = false;
        break;
    }
    case Tp::ChannelChatStatePaused: {
        message = tr("%1 has paused typing").arg(contact->alias());
        break;
    }
    case Tp::ChannelChatStateActive: {
        message = tr("%1 is now active").arg(contact->alias());
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
    foreach(Tpy::ConversationItem* item, mChatRunningItems) {
        if(item->contact() == contact) {
            qDebug("previous running item found, deleting");
            mChatRunningItems.removeOne(item);
            mSessionConversationModel->deleteItem(item);
            item = 0;
        }
    }

    // add the event message
    if (!message.isEmpty()) {
        Tpy::ConversationItem *item = new Tpy::ConversationItem(contact,
            QDateTime::currentDateTime(), message, Tpy::ConversationItem::EVENT, this);
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

    Tpy::ConversationItem *item = qobject_cast<Tpy::ConversationItem*>(sender());
    if(!item) {
        return;
    }

    QModelIndex idx = mSessionConversationModel->index(item);
    if (idx.isValid()) {
        emit dataChanged(idx, idx);
    }

    // if it is a new contact, add it to the list
    if(!mContactsList.contains(item->contact()->id())) {
        mContactsList.append(item->contact()->id());
    }
}

void IMConversationModel::notifyCallStatusChanged(Tp::ContactPtr contact, CallAgent::CallStatus oldCallStatus, CallAgent::CallStatus newCallStatus)
{
    qDebug() << "IMConversationModel::notifyCallStatusChanged: oldCallStatus=" << oldCallStatus << " newCallStatus=" << newCallStatus;

    if (!mLoggerConversationModel) {
        return;
    }

    // build the message
    QString message;
    bool running = true;

    // if it is a new contact, add it to the list
    if(!mContactsList.contains(contact->id())) {
        mContactsList.append(contact->id());
    }

    switch (newCallStatus) {
    case CallAgent::CallStatusNoCall:
        message = tr("Error in call with %1").arg(contact->alias());
        // check if previous was call
        if (oldCallStatus == CallAgent::CallStatusTalking ||
            oldCallStatus == CallAgent::CallStatusHeld ||
            oldCallStatus == CallAgent::CallStatusHangingUp) {
            message = tr("Call with %1 ended").arg(contact->alias());
        } else if (oldCallStatus == CallAgent::CallStatusIncomingCall) {
            message = tr("Missed call from %1").arg(contact->alias());
        }
        running = false;
        break;
    case CallAgent::CallStatusIncomingCall:
        message = tr("%1 is calling you").arg(contact->alias());
        break;
    case CallAgent::CallStatusConnecting:
        message = tr("Setting up call to %1").arg(contact->alias());
        break;
    case CallAgent::CallStatusRinging:
        message = tr("Calling %1").arg(contact->alias());
        break;
    case CallAgent::CallStatusTalking:
        message = tr("Call with %1 started").arg(contact->alias());
        break;
    case CallAgent::CallStatusHeld:
        message = tr("Call with %1 on hold").arg(contact->alias());
        break;
    case CallAgent::CallStatusHangingUp:
        break;
    }

    // if we have a previous running call item, delete it
    if (mCallRunningItem) {
        mLoggerConversationModel->deleteItem(mCallRunningItem);
        mCallRunningItem = 0;
    }

    // add the event message
    if (!message.isEmpty()) {
        Tpy::ConversationItem *item = new Tpy::ConversationItem(contact,
            QDateTime::currentDateTime(), message, Tpy::ConversationItem::EVENT, this);
        // remember running messages
        if (running) {
            mCallRunningItem = item;
        }
        mLoggerConversationModel->addItem(item);
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

    Tpy::ConversationItem *item = new Tpy::ConversationItem(contact,
        QDateTime::currentDateTime(), message, Tpy::ConversationItem::EVENT, this);
    mLoggerConversationModel->addItem(item);
}

void IMConversationModel::notifyFileTransfer(Tp::ContactPtr contact, FileTransferAgent *agent, Tp::FileTransferChannelPtr channel)
{
    FileTransferItem *item = new FileTransferItem(contact, agent, channel, this);
    connect(item, SIGNAL(itemChanged()), SLOT(onItemChanged()));

    if (mLoggerConversationModel) {
        mLoggerConversationModel->addItem(item);
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
    qDebug("bubble index: %d", index);
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
