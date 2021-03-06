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
#include <TelepathyQt4/ReceivedMessage>
#include <TelepathyQt4Yell/ChannelFactory>
#include <MGConfItem>
#include <mremoteaction.h>
#include "../telepathy-qml-lib/improtocolsmodel.h"

IMChannelApprover::IMChannelApprover(bool autoApproveCalls)
: Tp::AbstractClientApprover(channelFilters()),
  mApplicationRunning(false),
  mAutoApproveCalls(autoApproveCalls),
  mPendingCall(false),
  mProtocolsModel(0),
  mNotificationManager(this)
{
    mProtocolsModel = new IMProtocolsModel(this);
    if (!mProtocolsModel) {
        qDebug() << "Error creating protocols model";
    }

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

    // make the notification manager always behave like the application was in background
    // meaning all events are going to be notified
    mNotificationManager.setApplicationActive(false);
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

            bool appBusy = false;

            // if the application is not running, there is no ongoing call
            // but if the app is running, check if the user is not in a call already
            if (mApplicationRunning) {
                QDBusInterface meegoAppIM("com.meego.app.im",
                                          "/com/meego/app/im",
                                          "com.meego.app.im");

                QDBusReply<bool> reply = meegoAppIM.call("userBusyCalling");
                if (reply.isValid()) {
                    appBusy = reply.value();
                }
            }

            // if there is already an incoming call, the second one should just be rejected.
            if (mPendingCall || appBusy) {
                callChannel->hangup(Tpy::CallStateChangeReasonNoAnswer, TELEPATHY_ERROR_BUSY, QString());
                callChannel->requestClose();
                context->setFinished();
                return;
            }

            mPendingCall = true && !mAutoApproveCalls;

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

    // if the application is now running, it is the one handling notifications
    if (mApplicationRunning) {
        mNotificationManager.clear();
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

    // this is will get just the default incoming sound
    // at this point the protocol is unknown yet
    // TODO: maybe it could be fixed by moving this code to onCallChannelReady
    QString incomingCallSound = mProtocolsModel->customizerForId("")->property("incomingCallSound").toString();

    // TODO: this is a little hackish but it is the way to get it done for now
    // set the notification sound for incoming calls
    MGConfItem longVideoCallSound("/meego/chat/long-videocall-sound", this);
    longVideoCallSound.set(incomingCallSound);

    MGConfItem shortVideoCallSound("/meego/chat/short-videocall-sound", this);
    shortVideoCallSound.set(incomingCallSound);
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

    foreach (Tp::ReceivedMessage message, textChannel->messageQueue()) {
        onMessageReceived(textChannel, message);
    }

    connect(textChannel.data(), SIGNAL(messageReceived(Tp::ReceivedMessage)),
            this, SLOT(onMessageReceived(Tp::ReceivedMessage)));

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

    connect(callChannel.data(),
            SIGNAL(stateChanged(Tpy::CallState)),
            SLOT(onCallChannelStateChanged(Tpy::CallState)));

    QString accountId = callChannel->property("accountId").toString();
    Tp::ContactPtr contact = callChannel->initiatorContact();

    // Call the meego-ux-alarm daemon to show the incoming call dialog
    QList<QVariant> args;
    args << accountId << contact->id();
    MRemoteAction acceptAction("com.meego.app.imapprover",
                               "/com/meego/app/imapprover",
                               "com.meego.app.imapprover",
                               "acceptCall",
                               args);
    MRemoteAction rejectAction("com.meego.app.imapprover",
                               "/com/meego/app/imapprover",
                               "com.meego.app.imapprover",
                               "rejectCall",
                               args);
    QString icon = contact->avatarData().fileName;
    if (icon.isNull()) {
        // set a default image
        icon = "image://themedimage/widgets/apps/chat/call-fullscreen-default";
    }

    QDBusInterface meegoAlarm("org.meego.alarms",
                              "/incomingCall",
                              "org.meego.alarms");

    // get the icon Id for the protocol of the channel
    QString iconId;
    QList<Tp::AccountPtr> accounts = mTelepathyManager->accounts();
    foreach(Tp::AccountPtr account, accounts) {
        if (!account.isNull() &&
            !callChannel.isNull() &&
            account->connection() == callChannel->connection() &&
            account->connection() != callChannel->connection() ) {
            iconId = account->iconName();
            break;
        }
    }
    QString incomingCallSound = mProtocolsModel->customizerForId(iconId)->property("incomingCallSound").toString();

    meegoAlarm.call("incomingCall",
                    tr("Incoming Call"),
                    tr("%1 is calling you").arg(contact->alias()),
                    acceptAction.toString(),
                    rejectAction.toString(),
                    incomingCallSound,
                    icon);
}

void IMChannelApprover::onCallChannelStateChanged(Tpy::CallState state)
{
    if (state != Tpy::CallStateEnded &&
        state != Tpy::CallStateUnknown) {
        return;
    }

    // if there is no pending call, we dont have anything to do here
    if (!mPendingCall) {
        return;
    }

    mPendingCall = false;

    // dismiss the alarm
    QDBusInterface meegoAlarm("org.meego.alarms",
                              "/incomingCall",
                              "org.meego.alarms");

    meegoAlarm.call("dropCall");

    // and remove the dispatch operation
    Tpy::CallChannel *callChannel = qobject_cast<Tpy::CallChannel*>(sender());
    Tpy::CallChannelPtr callChannelPtr(callChannel);

    foreach (Tp::ChannelDispatchOperationPtr dispatchOperation, mDispatchOps) {
        if (dispatchOperation->channels().indexOf(callChannelPtr) >= 0) {
            mDispatchOps.removeAll(dispatchOperation);
            break;
        }
    }

    QString accountId = callChannel->property("accountId").toString();
    Tp::ContactPtr contact = callChannel->initiatorContact();

    // check if it was a video call
    int initialVideo = callChannel->immutableProperties().value("org.freedesktop.Telepathy.Channel.Type.Call.DRAFT.InitialVideo",0).toInt();

    // report the missed call
    if (mApplicationRunning) {
        if (initialVideo) {
            reportMissedVideoCalls(accountId,
                                   QStringList() << contact->id(),
                                   QStringList() << QDateTime::currentDateTime().toString());
        } else {
            reportMissedAudioCalls(accountId,
                                   QStringList() << contact->id(),
                                   QStringList() << QDateTime::currentDateTime().toString());
        }
    } else {
        if (initialVideo) {
            mMissedVideoCalls[accountId].contacts.append(contact->id());
            mMissedVideoCalls[accountId].times.append(QDateTime::currentDateTime().toString());
            mNotificationManager.notifyMissedVideoCall(accountId,
                                                       contact->id(),
                                                       contact->alias(),
                                                       QDateTime::currentDateTime());
        } else {
            mMissedAudioCalls[accountId].contacts.append(contact->id());
            mMissedAudioCalls[accountId].times.append(QDateTime::currentDateTime().toString());
            mNotificationManager.notifyMissedCall(accountId,
                                                  contact->id(),
                                                  contact->alias(),
                                                  QDateTime::currentDateTime());
        }
    }
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

    // if the application is not running, notify the incoming file transfer
    if (!mApplicationRunning) {
        Tp::ContactPtr contact;
        foreach(Tp::ContactPtr channelContact, fileTransferChannel->groupContacts().toList()) {
            if(contact != fileTransferChannel->groupSelfContact()) {
                contact = channelContact;
                break;
            }
        }

        if(contact.isNull()) {
            return;
        }

        mNotificationManager.notifyIncomingFileTransfer(accountId,
                                                        contact->id(),
                                                        contact->alias(),
                                                        QDateTime::currentDateTime(),
                                                        fileTransferChannel->fileName());
    }

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
    mPendingCall = false;

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

void IMChannelApprover::rejectCall(const QString &accountId, const QString &contactId)
{
    mPendingCall = false;

    int initialVideo = 0;
    QString contactAlias;

    // look for the channel in the pending dispatch operations and approve it
    foreach (Tp::ChannelDispatchOperationPtr dispatchOperation, mDispatchOps) {
        if (dispatchOperation->account()->uniqueIdentifier() != accountId) {
            continue;
        }

        QList<Tp::ChannelPtr> channels = dispatchOperation->channels();
        foreach (Tp::ChannelPtr channel, channels) {
            Tpy::CallChannelPtr callChannel = Tpy::CallChannelPtr::dynamicCast(channel);
            if (callChannel.isNull()) {
                continue;
            }

            Tp::ContactPtr contact = channel->initiatorContact();
            if (contact->id() == contactId) {
                callChannel->hangup(Tpy::CallStateChangeReasonUserRequested, QString(), QString());
                callChannel->requestClose();
                contactAlias = callChannel->initiatorContact()->alias();
                mDispatchOps.removeAll(dispatchOperation);

                // check if it was a video call
                initialVideo = callChannel->immutableProperties().value("org.freedesktop.Telepathy.Channel.Type.Call.DRAFT.InitialVideo",0).toInt();
                break;
            }
        }
    }

    // report the missed call
    if (mApplicationRunning) {
        if (initialVideo) {
            reportMissedVideoCalls(accountId,
                                   QStringList() << contactId,
                                   QStringList() << QDateTime::currentDateTime().toString());
        } else {
            reportMissedAudioCalls(accountId,
                                   QStringList() << contactId,
                                   QStringList() << QDateTime::currentDateTime().toString());
        }
    } else {
        if (initialVideo) {
            mMissedVideoCalls[accountId].contacts.append(contactId);
            mMissedVideoCalls[accountId].times.append(QDateTime::currentDateTime().toString());
            mNotificationManager.notifyMissedVideoCall(accountId,
                                                       contactId,
                                                       contactAlias,
                                                       QDateTime::currentDateTime());
        } else {
            mMissedAudioCalls[accountId].contacts.append(contactId);
            mMissedAudioCalls[accountId].times.append(QDateTime::currentDateTime().toString());
            mNotificationManager.notifyMissedCall(accountId,
                                                  contactId,
                                                  contactAlias,
                                                  QDateTime::currentDateTime());
        }
    }
}

void IMChannelApprover::onServiceRegistered()
{
    setApplicationRunning(true);

    // report the missed calls
    QMap<QString, MissedCalls>::const_iterator it = mMissedAudioCalls.constBegin();
    while (it != mMissedAudioCalls.constEnd()) {
        reportMissedAudioCalls(it.key(), it.value().contacts, it.value().times);
        ++it;
    }

    it = mMissedVideoCalls.constBegin();
    while (it != mMissedVideoCalls.constEnd()) {
        reportMissedVideoCalls(it.key(), it.value().contacts, it.value().times);
        ++it;
    }

    // and clear the already reported missed calls
    mMissedAudioCalls.clear();
    mMissedVideoCalls.clear();
}

void IMChannelApprover::onServiceUnregistered()
{
    setApplicationRunning(false);
}

void IMChannelApprover::reportMissedAudioCalls(const QString &accountId, const QStringList &contacts, const QStringList &times)
{
    QDBusInterface meegoAppIM("com.meego.app.im",
                              "/com/meego/app/im",
                              "com.meego.app.im");

    meegoAppIM.call("reportMissedAudioCalls", accountId, contacts, times);
}

void IMChannelApprover::reportMissedVideoCalls(const QString &accountId, const QStringList &contacts, const QStringList &times)
{
    QDBusInterface meegoAppIM("com.meego.app.im",
                              "/com/meego/app/im",
                              "com.meego.app.im");

    meegoAppIM.call("reportMissedVideoCalls", accountId, contacts, times);
}

void IMChannelApprover::onMessageReceived(const Tp::ReceivedMessage &message)
{
    Tp::TextChannelPtr channel(qobject_cast<Tp::TextChannel*>(sender()));
    onMessageReceived(channel, message);
}

void IMChannelApprover::onMessageReceived(const Tp::TextChannelPtr &textChannel, const Tp::ReceivedMessage &message)
{
    if (mApplicationRunning) {
        return;
    }

    QString accountId = textChannel->property("accountId").toString();
    Tp::ContactPtr contact = message.sender();

    // do not log old messages
    if (message.isRescued() || message.isScrollback()) {
        return;
    }

    // do not log delivery reports
    if(message.messageType() == Tp::ChannelTextMessageTypeDeliveryReport) {
        return;
    }

    if (textChannel->targetHandleType() == Tp::HandleTypeGroup ||
        textChannel->targetHandleType() == Tp::HandleTypeRoom) {
        mNotificationManager.notifyPendingGroupMessage(accountId,
                                                       textChannel->objectPath(),
                                                       contact->alias(),
                                                       message.received(),
                                                       message.text());

    } else {
        mNotificationManager.notifyPendingMessage(accountId,
                                                  contact->id(),
                                                  contact->alias(),
                                                  message.received(),
                                                  message.text());
    }
}

