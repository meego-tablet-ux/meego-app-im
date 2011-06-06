/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "imchannelapprover.h"
#include "imapproveradaptor_p.h"

#include <TelepathyQt4/Account>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4Yell/ChannelClassSpec>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/AccountFactory>
#include <TelepathyQt4/ConnectionFactory>
#include <TelepathyQt4/ContactFactory>
#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4Yell/ChannelFactory>
#include <MGConfItem>

IMChannelApprover::IMChannelApprover(bool autoApproveCalls)
: Tp::AbstractClientApprover(channelFilters()),
  mApplicationRunning(false),
  mNotificationManager(this),
  mAutoApproveCalls(autoApproveCalls)
{
    mAdaptor = new IMApproverAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/com/meego/app/imapprover", this);
    QDBusConnection::sessionBus().registerService("com.meego.app.imapprover");

    // watch for the meego-app-im to be running
    mIMServiceWatcher.setConnection(QDBusConnection::sessionBus());
    mIMServiceWatcher.setWatchMode(QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration);
    mIMServiceWatcher.addWatchedService("org.freedesktop.Telepathy.Client.MeeGoIM");

    connect(&mIMServiceWatcher,
            SIGNAL(serviceRegistered(QString)),
            SLOT(onServiceRegistered()));
    connect(&mIMServiceWatcher,
            SIGNAL(serviceUnregistered(QString)),
            SLOT(onServiceUnregistered()));

    bool appRunning = QDBusConnection::sessionBus().interface()->isServiceRegistered("org.freedesktop.Telepathy.Client.MeeGoIM").value();
    setApplicationRunning(appRunning);
}

IMChannelApprover::~IMChannelApprover()
{
}

void IMChannelApprover::addDispatchOperation(const Tp::MethodInvocationContextPtr<> &context,
                                        const Tp::ChannelDispatchOperationPtr &dispatchOperation)
{
    QList<Tp::ChannelPtr> channels = dispatchOperation->channels();

    bool shouldApprove = true;

    foreach (Tp::ChannelPtr channel, channels) {

        channel->setProperty("channelDispatchOperation", QVariant(dispatchOperation->objectPath()));

        Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (!textChannel.isNull()) {
            // if the chat app is running, just approve the channel
            if (mApplicationRunning) {
                continue;
            }

            // todo fix hack: stick a property to have accountId handy when we get the stream
            textChannel->setProperty("accountId", QVariant(dispatchOperation->account()->uniqueIdentifier()));
            qDebug() << "IMChannelApprover::handleChannels: handling text channel - becomeReady "
                     << " immutableProperties=" << textChannel->immutableProperties();
            connect(textChannel->becomeReady(Tp::Features()
                                             << Tp::TextChannel::FeatureCore
                                             << Tp::TextChannel::FeatureChatState
                                             << Tp::TextChannel::FeatureMessageCapabilities
                                             << Tp::TextChannel::FeatureMessageQueue
                                             << Tp::TextChannel::FeatureMessageSentSignal),
                    SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onTextChannelReady(Tp::PendingOperation*)));
            shouldApprove = false;
            continue;
        }

        Tpy::CallChannelPtr callChannel = Tpy::CallChannelPtr::dynamicCast(channel);
        if (!callChannel.isNull()) {

            // todo fix hack: stick a property to have accountId handy when we get the stream
            callChannel->setProperty("accountId", QVariant(dispatchOperation->account()->uniqueIdentifier()));
            connect(callChannel->becomeReady(Tp::Features()
                                             << Tpy::CallChannel::FeatureCore
                                             << Tpy::CallChannel::FeatureContents),
                    SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onCallChannelReady(Tp::PendingOperation*)));

            shouldApprove = false || mAutoApproveCalls;
            continue;
        }

        Tp::IncomingFileTransferChannelPtr fileTransferChannel = Tp::IncomingFileTransferChannelPtr::dynamicCast(channel);
        if (!fileTransferChannel.isNull()) {
            // if the chat application is running, just approve the channel
            if (mApplicationRunning) {
                continue;
            }
            // todo fix hack: stick a property to have accountId handy when we get the stream
            fileTransferChannel->setProperty("accountId", QVariant(dispatchOperation->account()->uniqueIdentifier()));
            qDebug() << "IMChannelApprover::handleChannels: handling file transfer channel - becomeReady "
                     << " immutableProperties=" << fileTransferChannel->immutableProperties();
            connect(fileTransferChannel->becomeReady(Tp::Features()
                                                     << Tp::IncomingFileTransferChannel::FeatureCore),
                    SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onFileTransferChannelReady(Tp::PendingOperation*)));
            shouldApprove = false;
            continue;
        }
    }
    connect(dispatchOperation.data(), SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(onInvalidated()) );

    if (shouldApprove) {
        if (dispatchOperation->possibleHandlers().contains("org.freedesktop.Telepathy.Client.MeeGoIM")) {
            dispatchOperation->handleWith("org.freedesktop.Telepathy.Client.MeeGoIM");
        }

        // TODO: check what to do when the MeegoIM handler is not available
    } else {
        mDispatchOps.append(dispatchOperation);
    }
    context->setFinished();
}

Tp::ChannelClassSpecList IMChannelApprover::channelFilters() const
{
    Tp::ChannelClassSpecList specList;
    specList << Tp::ChannelClassSpec::textChat();
    specList << Tp::ChannelClassSpec::textChatroom();
    specList << Tpy::ChannelClassSpec::mediaCall();
    specList << Tpy::ChannelClassSpec::audioCall();
    specList << Tpy::ChannelClassSpec::videoCall();
    specList << Tpy::ChannelClassSpec::videoCallWithAudio();
    specList << Tp::ChannelClassSpec::incomingFileTransfer();
    specList << Tp::ChannelClassSpec::outgoingFileTransfer();

    QMap<QString, QDBusVariant> filter;
    filter.insert(QString::fromLatin1(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
                  QDBusVariant(QString::fromLatin1(TELEPATHY_INTERFACE_CHANNEL_TYPE_SERVER_AUTHENTICATION)));
    filter.insert(QString::fromLatin1(TELEPATHY_INTERFACE_CHANNEL_TYPE_SERVER_AUTHENTICATION ".AuthenticationMethod"),
                  QDBusVariant(QString::fromLatin1(TELEPATHY_INTERFACE_CHANNEL_INTERFACE_SASL_AUTHENTICATION)));
    filter.insert(QString::fromLatin1(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
                  QDBusVariant(Tp::HandleTypeNone));
    specList << Tp::ChannelClassSpec(Tp::ChannelClass(filter));

    return specList;
}

void IMChannelApprover::setApplicationRunning(bool running)
{
    mApplicationRunning = running;
    mNotificationManager.setApplicationActive(running);

    // FIXME: check how to properly handle that
    // for now just approve everything
    foreach(Tp::ChannelDispatchOperationPtr dispatchOperation, mDispatchOps) {
        // approve only filetransfers and text chats
        // incoming calls will be approved when the notification is accepted
        QList<Tp::ChannelPtr> channels = dispatchOperation->channels();
        foreach (Tp::ChannelPtr channel, channels) {
            Tpy::CallChannelPtr callChannel = Tpy::CallChannelPtr::dynamicCast(channel);
            if (!callChannel.isNull()) {
                break;
            }

            if (dispatchOperation->possibleHandlers().contains("org.freedesktop.Telepathy.Client.MeeGoIM")) {
                dispatchOperation->handleWith("org.freedesktop.Telepathy.Client.MeeGoIM");
                mDispatchOps.removeAll(dispatchOperation);
            }
            // TODO: check what to do when the MeegoIM handler is not available
        }
    }
}

void IMChannelApprover::registerApprover()
{
    mTelepathyManager = TelepathyManager::instance();
    mClientRegistrar = Tp::ClientRegistrar::create(mTelepathyManager->accountManager());

    qDebug() << "Registering the approver";
    Tp::AbstractClientPtr approver(this);

    // register the approver
    mClientRegistrar->registerClient(approver, "MeeGoIMApprover");

    // TODO: this is a little hackish but it is the way to get it done for now
    // set the notification sound for incoming calls
    MGConfItem longVideoCallSound("/meego/chat/long-videocall-sound", this);
    longVideoCallSound.set("/usr/share/sounds/meego/stereo/ring-1.wav");

    MGConfItem shortVideoCallSound("/meego/chat/short-videocall-sound", this);
    shortVideoCallSound.set("/usr/share/sounds/meego/stereo/ring-1.wav");
}

void IMChannelApprover::onTextChannelReady(Tp::PendingOperation *op)
{
    qDebug() << "IMChannelApprover::onTextChannelReady: channel ready";

    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);
    if (!pr || pr->isError()) {
        qDebug() << "IMChannelApprover::onTextChannelReady: error "
                 << (op ? op->errorName() : "")
                 << (op ? op->errorMessage() : "");
        return;
    }

    Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(pr->object());
    if (textChannel.isNull()) {
        qDebug() << "IMChannelApprover::onTextChannelReady: stream invalid";
        return;
    }
    QString accountId = textChannel->property("accountId").toString();

    emit textChannelAvailable(accountId, textChannel);
}

void IMChannelApprover::onCallChannelReady(Tp::PendingOperation *op)
{
    qDebug() << "IMChannelApprover::onCallChannelReady: channel ready";

    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);
    if (!pr || pr->isError()) {
        qDebug() << "IMChannelApprover::onCallChannelReady: error "
                 << (op ? op->errorName() : "")
                 << (op ? op->errorMessage() : "");
        return;
    }

    Tpy::CallChannelPtr callChannel = Tpy::CallChannelPtr::dynamicCast(pr->object());
    if (callChannel.isNull()) {
        qDebug() << "IMChannelApprover::onCallChannelReady: channel invalid";
        return;
    }
    QString accountId = callChannel->property("accountId").toString();
    QString operationPath = callChannel->property("channelDispatchOperation").toString();

    Tp::ContactPtr contact = callChannel->initiatorContact();
    qDebug() << "Contact avatar:" << contact->avatarData().fileName;
    mNotificationManager.notifyIncomingCall(accountId,
                                            contact->id(),
                                            contact->alias(),
                                            contact->avatarData().fileName);
}

void IMChannelApprover::onFileTransferChannelReady(Tp::PendingOperation *op)
{
    qDebug() << "IMChannelApprover::onFileTransferChannelReady: channel ready";

    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);
    if (!pr || pr->isError()) {
        qDebug() << "IMChannelApprover::onTextChannelReady: error "
                 << (op ? op->errorName() : "")
                 << (op ? op->errorMessage() : "");
        return;
    }

    Tp::IncomingFileTransferChannelPtr fileTransferChannel = Tp::IncomingFileTransferChannelPtr::dynamicCast(pr->object());
    if (fileTransferChannel.isNull()) {
        qDebug() << "IMChannelApprover::onFileTransferChannelReady: stream invalid";
        return;
    }
    QString accountId = fileTransferChannel->property("accountId").toString();

    emit fileTransferChannelAvailable(accountId, fileTransferChannel);
}

void IMChannelApprover::onCloseOperation(QString operationObjectPath)
{
    foreach (Tp::ChannelDispatchOperationPtr dispatchOperation, mDispatchOps) {
        if (dispatchOperation->objectPath() == operationObjectPath) {
            QList<Tp::ChannelPtr> channels = dispatchOperation->channels();
            foreach (Tp::ChannelPtr channel, channels) {
                channel->requestClose();
            }
        }
    }
}

void IMChannelApprover::onInvalidated()
{
    emit invalidated();
}

void IMChannelApprover::acceptCall(const QString &accountId, const QString &contactId)
{
    // look for the channel in the pending dispatch operations and approve it
    foreach (Tp::ChannelDispatchOperationPtr dispatchOperation, mDispatchOps) {
        if (dispatchOperation->account()->uniqueIdentifier() != accountId) {
            continue;
        }

        bool approve = false;
        QList<Tp::ChannelPtr> channels = dispatchOperation->channels();
        foreach (Tp::ChannelPtr channel, channels) {
            Tpy::CallChannelPtr callChannel = Tpy::CallChannelPtr::dynamicCast(channel);
            if (callChannel.isNull()) {
                continue;
            }

            Tp::ContactPtr contact = channel->initiatorContact();
            if (contact->id() == contactId) {
                approve = true;
                break;
            }
        }

        if (approve) {
            if (dispatchOperation->possibleHandlers().contains("org.freedesktop.Telepathy.Client.MeeGoIM")) {
                qDebug() << "Good! we can now approve the call";
                dispatchOperation->handleWith("org.freedesktop.Telepathy.Client.MeeGoIM");
            }

            // TODO: check what to do when the MeegoIM handler is not available

            mDispatchOps.removeAll(dispatchOperation);
        }
    }
}

void IMChannelApprover::onServiceRegistered()
{
    setApplicationRunning(true);
}

void IMChannelApprover::onServiceUnregistered()
{
    setApplicationRunning(false);
}
