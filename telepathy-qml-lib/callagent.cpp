/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "farstreamchannel.h"
#include "callagent.h"
#include <gst/gst.h>
#include <glib-object.h>
#include <gst/farsight/fs-element-added-notifier.h>
#include <gst/farsight/fs-conference-iface.h>
#include <TelepathyQt4/Channel>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingChannelRequest>
#include <TelepathyQt4/ChannelRequest>
#include <TelepathyQt4/PendingChannel>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/ReferencedHandles>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/ReferencedHandles>
#include <TelepathyQt4Yell/Farstream/Channel>
#include <QDebug>
#include <QLatin1String>

CallAgent::CallAgent(
        const Tp::AccountPtr &account,
        const Tp::ContactPtr &contact,
        QObject *parent)
    : QObject(parent),
      mAccount(account),
      mContact(contact),
      mCallStatus(CallStatusNoCall),
      mError(false),
      mMissedVideoCalls(0),
      mMissedAudioCalls(0),
      mRemoteVideoRender(false),
      mFarstreamChannel(0),
      mIncomingVideo(0),
      mOutgoingVideo(0),
      mCurrentOrientation(1), // TopUp
      mPendingChannelRequest(0),
      mIsRequested(false),
      mResourceSetCall(0),
      mUseResourcePolicy(true)
{
    qDebug() << "CallAgent::CallAgent: created for contact " << mContact->id();

    connect(mAccount.data(), SIGNAL(connectionChanged(const Tp::ConnectionPtr&)),
        SLOT(onAccountConnectionChanged(const Tp::ConnectionPtr&)));

    onAccountConnectionChanged(mAccount->connection());
}

CallAgent::CallAgent(QObject *parent)
    : QObject(parent),
      mCallStatus(CallStatusNoCall),
      mError(false),
      mMissedVideoCalls(0),
      mMissedAudioCalls(0),
      mRemoteVideoRender(false),
      mFarstreamChannel(0),
      mIncomingVideo(0),
      mOutgoingVideo(0),
      mPendingChannelRequest(0)
{
    Q_UNUSED(parent);
    qDebug() << "CallAgent::CallAgent: default constructor";

    mChannelFactory = Tpy::FarstreamChannelFactory::create();
}

CallAgent::~CallAgent()
{
    qDebug() << "CallAgent::~CallAgent: destroyed for contact " << mContact->id();

    endCall();
    freeResourceSets();
}

void CallAgent::beginCall(bool withVideo)
{
    qDebug() << "CallAgent::beginCall: contact=" << mContact->id() << " status=" << callStatus();

    if (callStatus() == CallStatusIncomingCall) {
        qDebug() << "CallAgent::beginCall: accepting incoming call instead";
        acceptCall();
        return;
    }

    if (callStatus() != CallStatusNoCall && callStatus() != CallStatusResourcing) {
        qDebug() << "CallAgent::beginCall: returning, already in call";
        return;
    }

    mRemoteVideoRender = false;
    emit remoteVideoRenderChanged();

    // call is done through the Channel Dispatcher
    QVariantMap request;
    request.insert(TELEPATHY_INTERFACE_CHANNEL ".ChannelType",
                   TP_QT4_YELL_IFACE_CHANNEL_TYPE_CALL);
    request.insert(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType", Tp::HandleTypeContact);
    request.insert(TELEPATHY_INTERFACE_CHANNEL ".TargetHandle", mContact->handle()[0]);
    request.insert(TP_QT4_YELL_IFACE_CHANNEL_TYPE_CALL + QLatin1String(".InitialAudio"), true);
    emit audioSentChanged();
    if (withVideo) {
        request.insert(TP_QT4_YELL_IFACE_CHANNEL_TYPE_CALL + QLatin1String(".InitialVideo"), true);
        emit videoSentChanged();
        emit videoSentOrAboutToChanged();
    }
    mPendingChannelRequest = mAccount->ensureChannel(
                request,
                QDateTime::currentDateTime(),
                QLatin1String("org.freedesktop.Telepathy.Client.MeeGoIM"));
    if (!mPendingChannelRequest) {
        setErrorString(tr("Unable to create media call for contact %1")
                       .arg(mContact->id()));
        endCall();
        return;
    }

    // calling (call setup)
    clearError();
    setCallStatus(CallStatusConnecting);

    connect(mPendingChannelRequest,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onPendingChanelRequestFinished(Tp::PendingOperation*)));
    connect(mPendingChannelRequest,
            SIGNAL(channelRequestCreated(Tp::ChannelRequestPtr)),
            SLOT(onPendingChanelRequestCreated(Tp::ChannelRequestPtr)));
}

void CallAgent::ensureResourceSetConfiguration(bool video)
{
    qDebug() << "Ensuring resources are setup for " << (video ? "audio/video" : "audio");

    if (!mResourceSetCall) {
        qDebug() << "Resources not created, creating now";
        setupResourceSets();
        connectResourceSets();
    }

    if (mResourceSetCall) {
        if (video) {
            mResourceSetCall->addResource(ResourcePolicy::VideoPlaybackType);
            mResourceSetCall->addResource(ResourcePolicy::VideoRecorderType);
        } else {
            mResourceSetCall->deleteResource(ResourcePolicy::VideoPlaybackType);
            mResourceSetCall->deleteResource(ResourcePolicy::VideoRecorderType);
        }
        bool ret = mResourceSetCall->update();
        qDebug() << "mResourceSetCall->update() -> " << ret;
    }
    else qWarning() << "No resources could be created";
}

void CallAgent::startOperationWithResources(const QString &slot, bool video)
{
    ensureResourceSetConfiguration(video);
    if (mResourceSetCall) {
        mOnAcquireInvoke = slot;
        mWaitingForResources = true;
        /* only set status when we're not started anything yet */
        if (mCallStatus == CallStatusNoCall)
            setCallStatus(CallStatusResourcing);
        qDebug() << "Acquiring resources to call " << slot;
        bool ret = mResourceSetCall->acquire();
        qDebug() << "mResourceSetCall acquire " << ret;
    }
    else qWarning() << "No resources, cannot setup for calling " << slot;
}

void CallAgent::endCall()
{
    qDebug() << "CallAgent::endCall: ";

    if (mResourceSetCall) {
        qDebug() << "Releasing resources";
        ensureResourceSetConfiguration(true);
        bool ret = mResourceSetCall->release();
        qDebug() << "mResourceSetCall->release() -> " << ret;
    }

    if (mPendingChannelRequest != 0) {
        // we assume it will be canceled ok, no need to track the pending operation
        mPendingChannelRequest->cancel();
        mPendingChannelRequest = 0;
    }

    if (mFarstreamChannel != 0) {
        mFarstreamChannel->stop();
        mFarstreamChannel->deleteLater();
        mFarstreamChannel = 0;
    }

    if (mCallStatus == CallStatusNoCall || mCallStatus == CallStatusHangingUp) {
        return;
    }

    if (mCallChannel.isNull()) {
        setCallStatus(CallStatusNoCall);
        return;
    }

    Tp::PendingOperation *op = mCallChannel->hangup(Tpy::CallStateChangeReasonUserRequested, QString(), QString());
    if (op) {
        setCallStatus(CallStatusHangingUp);
        connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onRequestCloseFinished(Tp::PendingOperation*)));
    } else {
        setErrorString(tr("Error ending call"));
    }

    op = mCallChannel->requestClose();
    if (op) {
        setCallStatus(CallStatusHangingUp);
        connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onRequestCloseFinished(Tp::PendingOperation*)));
    } else {
        setErrorString(tr("Error ending call"));
    }

    disconnect(mCallChannel.data(),
               SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
               this,
               SLOT(onChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &)));
}

void CallAgent::beginAcceptCall()
{
    qDebug() << "CallAgent::beginAcceptCall:";

    if (!incomingVideoCall() && !incomingAudioCall()) {
        qDebug() << "CallAgent::beginAcceptCall: no incoming call";
        return;
    }

    if (mCallChannel.isNull()) {
        qDebug() << "CallAgent::beginAcceptCall: no call channel available";
        return;
    }

    Tp::PendingOperation *op = mCallChannel->accept();
    if (!op) {
        setErrorString(tr("Call could not be accepted"));
        return;
    }

    clearError();
    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onAcceptCallFinished(Tp::PendingOperation*)));
}

void CallAgent::acceptCall()
{
    qDebug() << "CallAgent::acceptCall: useResourcePolicy=" << mUseResourcePolicy;

    if (!mUseResourcePolicy) {
        beginAcceptCall();
        return;
    }

    if (!incomingVideoCall() && !incomingAudioCall()) {
        qDebug() << "CallAgent::acceptCall: no incoming call";
        return;
    }

    if (mCallChannel.isNull()) {
        qDebug() << "CallAgent::acceptCall: no call channel available";
        return;
    }

    int initialVideo = mCallChannel->immutableProperties().value("InitialVideo",0).toInt();

    startOperationWithResources(SLOT(beginAcceptCall()), initialVideo!=0);
}

void CallAgent::requestSendingAudio(Tpy::CallContentPtr content, bool send)
{
    foreach (Tpy::CallStreamPtr stream, content->streams()) {
        qDebug() << "Requesting sending " << send << " on stream " << stream;
        Tp::PendingOperation *op = stream->requestSending(send);
        if (op) {
            qDebug() << "Waiting for pending call to finish";
            connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onRequestSendingAudioFinished(Tp::PendingOperation*)));
            emit audioSentChanged();
        }
        else {
            qDebug() << "Failed to requestSending audio";
        }
    }
}

void CallAgent::setAudioSend(bool send)
{
    qDebug() << "CallAgent::setAudioSend: " << send;

    if (mCallChannel.isNull()) {
        return;
    }

    if (!mCallChannel->hasMutableContents()) {
        return;
    }

    /* if we want to switch video on, but do not have that content yet, we need to add it first */
    Tpy::CallContents contents = mCallChannel->contentsForType(Tp::MediaStreamTypeAudio);
    if (send && contents.isEmpty()) {
        Tpy::PendingCallContent * pcc = mCallChannel->requestContent("audio", Tp::MediaStreamTypeAudio);
        if (pcc) {
            connect(pcc, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onRequestContentFinishedSwitchOn(Tp::PendingOperation*)));
        }
    }
    else {
        foreach (Tpy::CallContentPtr content, contents) {
            requestSendingAudio(content, send);
        }
    }
}

bool CallAgent::audioSent() const
{
    if (mCallChannel.isNull()) {
        return false;
    }

    foreach (Tpy::CallContentPtr content, mCallChannel->contentsForType(Tp::MediaStreamTypeAudio)) {
        foreach (Tpy::CallStreamPtr stream, content->streams()) {
            if (stream->localSendingState() == Tpy::SendingStateSending) {
                return true;
            }
        }
    }

    return false;
}

bool CallAgent::audioReceived() const
{
    if (mCallChannel.isNull()) {
        return false;
    }

    foreach (Tpy::CallContentPtr content, mCallChannel->contentsForType(Tp::MediaStreamTypeAudio)) {
        foreach (Tpy::CallStreamPtr stream, content->streams()) {
            foreach (Tp::ContactPtr contact, stream->members()) {
                if (stream->remoteSendingState(contact) == Tpy::SendingStateSending) {
                    return true;
                }
            }
        }
    }

    return false;
}

Tpy::SendingState CallAgent::getVideoSentState() const
{
    Tpy::SendingState state = Tpy::SendingStateNone;

    if (mCallChannel.isNull()) {
        return state;
    }

    foreach (Tpy::CallContentPtr content, mCallChannel->contentsForType(Tp::MediaStreamTypeVideo)) {
        foreach (Tpy::CallStreamPtr stream, content->streams()) {
            Tpy::SendingState s = stream->localSendingState();
            if (s == Tpy::SendingStateSending) {
                state = s;
            }
            else if (s == Tpy::SendingStatePendingSend && state != Tpy::SendingStateSending) {
                state = s;
            }
        }
    }

    return state;
}

bool CallAgent::videoSent() const
{
    Tpy::SendingState state = getVideoSentState();
    return state == Tpy::SendingStateSending;
}

bool CallAgent::videoSentOrAboutTo() const
{
    Tpy::SendingState state = getVideoSentState();
    return state == Tpy::SendingStateSending || state == Tpy::SendingStatePendingSend;
}

bool CallAgent::videoReceived() const
{
    if (mCallChannel.isNull()) {
        return false;
    }

    foreach (Tpy::CallContentPtr content, mCallChannel->contentsForType(Tp::MediaStreamTypeVideo)) {
        foreach (Tpy::CallStreamPtr stream, content->streams()) {
            foreach (Tp::ContactPtr contact, stream->members()) {
                if (stream->remoteSendingState(contact) == Tpy::SendingStateSending) {
                    return true;
                }
            }
        }
    }

    return false;
}

void CallAgent::requestSendingVideo(Tpy::CallContentPtr content, bool send)
{
    foreach (Tpy::CallStreamPtr stream, content->streams()) {
        qDebug() << "Requesting sending " << send << " on stream " << stream;
        Tp::PendingOperation *op = stream->requestSending(send);
        if (op) {
            qDebug() << "Waiting for pending call to finish";
            connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onRequestSendingVideoFinished(Tp::PendingOperation*)));
            emit videoSentOrAboutToChanged();
        }
        else {
            qDebug() << "Failed to requestSending video";
        }
    }
}

void CallAgent::beginSetVideoSend(bool send)
{
    qDebug() << "CallAgent::beginSetVideoSend: " << send;

    if (mCallChannel.isNull()) {
        return;
    }

    if (!mCallChannel->hasMutableContents()) {
        return;
    }

    /* if we want to switch video on, but do not have that content yet, we need to add it first */
    Tpy::CallContents contents = mCallChannel->contentsForType(Tp::MediaStreamTypeVideo);
    if (send && contents.isEmpty()) {
        Tpy::PendingCallContent * pcc = mCallChannel->requestContent("video", Tp::MediaStreamTypeVideo);
        if (pcc) {
            connect(pcc, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onRequestContentFinishedSwitchOn(Tp::PendingOperation*)));
        }
    }
    else {
        foreach (Tpy::CallContentPtr content, contents) {
            requestSendingVideo(content, send);
        }
    }
}

void CallAgent::beginSetVideoSendTrue()
{
    beginSetVideoSend(true);
}

void CallAgent::beginSetVideoSendFalse()
{
    beginSetVideoSend(false);
}

void CallAgent::setVideoSend(bool send)
{
    qDebug() << "CallAgent::setVideoSend: " << send;

    if (!mUseResourcePolicy) {
        beginSetVideoSend(send);
        return;
    }

    if (mCallChannel.isNull()) {
        return;
    }

    if (!mCallChannel->hasMutableContents()) {
        return;
    }

    startOperationWithResources(send ? SLOT(beginSetVideoSendTrue()) : SLOT(beginSetVideoSendFalse()), send);
}

void CallAgent::setHoldCall(bool hold)
{
    qDebug() << "CallAgent::setHoldCall(): hold=" << hold
             << " localHoldState=" << mCallChannel->localHoldState()
             << " localHoldStateReason=" << mCallChannel->localHoldStateReason();

    if (mCallChannel.isNull()) {
        qDebug() << "CallAgent::setHoldCall: no call channel available";
        return;
    }

    Tp::PendingOperation *op = mCallChannel->requestHold(hold);
    if (!op) {
        setErrorString(tr("Unable to set hold"));
        return;
    }

    connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onRequestHoldFinished(Tp::PendingOperation*)));
}

CallAgent::CallStatus CallAgent::callStatus() const
{
    return mCallStatus;
}

void CallAgent::setCallStatus(CallStatus callStatus)
{
    CallStatus oldCallStatus = mCallStatus;
    bool previousIncomingAudioCall = incomingAudioCall();
    bool previousIncomingVideoCall = incomingVideoCall();
    bool previousHeldCall = heldCall();
    bool previousExistingCall = existingCall();

    if (mCallStatus != callStatus) {
        qDebug() << "CallAgent::setCallStatus: changed status to " << callStatus;
        if (callStatus == CallStatusTalking && mCallStatus != CallStatusHeld) {
            setStartTime(QDateTime::currentDateTime());
        }
        // make sure the duration is updated when call finishes
        if (mCallStatus == CallStatusTalking) {
            updateCallDuration();
        }
        mCallStatus = callStatus;
        emit callStatusChanged(oldCallStatus, mCallStatus);
    }

    if (previousIncomingVideoCall != incomingVideoCall()) {
        emit incomingVideoCallChanged();
    }
    if (previousIncomingAudioCall != incomingAudioCall()) {
        emit incomingAudioCallChanged();
    }

    if (previousHeldCall != heldCall()) {
        emit heldCallChanged();
    }

    if (previousExistingCall != existingCall()) {
        emit existingCallChanged();
    }
}

QString CallAgent::errorString() const
{
    return mErrorString;
}

bool CallAgent::error() const
{
    return mError;
}

void CallAgent::clearError()
{
    mError = false;
    mErrorString.clear();
    emit errorChanged();
}

void CallAgent::setErrorString(const QString &errorString)
{
    qDebug() << "CallAgent::setErrorString: " << errorString << " currentStatus=" << mCallStatus;

    CallStatus oldCallStatus = mCallStatus;
    bool statusChanged = (mCallStatus != CallStatusNoCall);
    bool errorHasChanged = !mError;

    mCallStatus = CallStatusNoCall;
    mError = true;

    if (mErrorString != errorString) {
        mErrorString = errorString;
        emit errorStringChanged();
        emit error(errorString);
    }

    if (errorHasChanged) {
        emit errorChanged();
    }

    if (statusChanged) {
        emit callStatusChanged(oldCallStatus, mCallStatus);
    }
}

void CallAgent::onChannelAvailable(Tp::ChannelPtr channel)
{
    qDebug() << "CallAgent::onChannelAvailable: handling channel=" << channel.data();

    Tpy::CallChannelPtr callChannel = Tpy::CallChannelPtr::dynamicCast(channel);
    qDebug() << "  converted to callChannel=" << callChannel.data();
    if (callChannel.isNull()) {
        setErrorString(tr("Channel not available"));
        endCall();
        return;
    }

    mCallChannel = callChannel;
    qDebug() << " state=" << mCallChannel->state()
             << " flags=" << int(mCallChannel->flags())
             << " stateReason=" << mCallChannel->stateReason().reason
             << " stateDetails=" << mCallChannel->stateDetails();

    mIsRequested = mCallChannel->isRequested();

    // If we just cancelled the request, we might still get notification of the new channel.
    // If we do, we just close it as it is not needed anymore.
    if (mCallStatus == CallStatusNoCall && mCallChannel->isRequested()) {
        qDebug() << "CallAgent::onChannelAvailable: Requested channel available, but we cancelled it, ending";
        endCall();
        return;
    }

    if (!mCallChannel->isRequested() && mCallChannel->state() == Tpy::CallStatePendingReceiver) {
        QVariantMap props = channel->immutableProperties();
        //int initialVideo = props.value("InitialVideo",0).toInt();
        emit videoReceivedChanged();
        emit videoSentChanged();
        emit videoSentOrAboutToChanged();
        emit audioReceivedChanged();
        emit audioSentChanged();
        setCallStatus(CallStatusIncomingCall);
    } else if (mCallChannel->isRequested() && mCallChannel->state() == Tpy::CallStatePendingReceiver) {
        setCallStatus(CallStatusRinging);
    } else {
        setCallStatus(CallStatusConnecting);
    }

    Tp::Features features;
    features << Tpy::CallChannel::FeatureCore
             << Tpy::CallChannel::FeatureContents;
             //<< Tpy::CallChannel::FeatureLocalHoldState;

    connect(mCallChannel->becomeReady(features),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onCallChannelReady(Tp::PendingOperation*)));
}

void CallAgent::beginAudioCall()
{
    beginCall(false);
}

void CallAgent::audioCall()
{
    qDebug() << "CallAgent::audioCall";

    if (!mUseResourcePolicy) {
        beginAudioCall();
        return;
    }

    ensureResourceSetConfiguration(false);
    startOperationWithResources(SLOT(beginAudioCall()), false);
}

void CallAgent::beginVideoCall()
{
    qDebug() << "CallAgent::beginAudioCall";
    beginCall(true);
}

void CallAgent::videoCall()
{
    qDebug() << "CallAgent::videoCall";

    if (!mUseResourcePolicy) {
        beginVideoCall();
        return;
    }

    startOperationWithResources(SLOT(beginVideoCall()), true);
}

bool CallAgent::existingCall() const
{
    return (callStatus() == CallStatusTalking || callStatus() == CallStatusHeld);
}

bool CallAgent::incomingVideoCall() const
{
    return callStatus() == CallStatusIncomingCall && videoReceived();
}

bool CallAgent::incomingAudioCall() const
{
    return callStatus() == CallStatusIncomingCall && !videoReceived();
}

void CallAgent::setupFarstreamChannel(FarstreamChannel *channel)
{
    qDebug() << "CallAgent::createFarstreamChannel:";

    qDebug() << "created farstream channel: " << mFarstreamChannel;
    mFarstreamChannel = channel;
    if (!mFarstreamChannel) {
        qDebug() << "CallAgent::handleReadyChannel: Farstream channel could not be created";
        return;
    }
    connect(mFarstreamChannel, SIGNAL(stateChanged()), SLOT(onFarstreamStateChanged()));
    connect(mFarstreamChannel, SIGNAL(remoteVideoRender(bool)), SLOT(onRemoteVideoRender(bool)));
    qDebug() << "CallAgent::handleReadyChannel: Farstream channel state=" << mFarstreamChannel->state();

    onOrientationChanged(mCurrentOrientation);

    emit volumeChanged();
    emit mutedCallChanged();

    if (mFarstreamChannel) {
        if (mOutgoingVideo) {
            mFarstreamChannel->setOutgoingVideo(mOutgoingVideo);
        }
        if (mIncomingVideo) {
            mFarstreamChannel->setIncomingVideo(mIncomingVideo);
        }
        mFarstreamChannel->init();
    }
}

void CallAgent::onVideoItemDestroyed(QObject *obj)
{
    qDebug() << "CallAgent::onVideoItemDestroyed: " << obj;

    if (obj == mOutgoingVideo) {
        qDebug() << "CallAgent::onVideoItemDestroyed: reset outgoing video item";
        mOutgoingVideo = 0;
    }
    else if (obj == mIncomingVideo) {
        qDebug() << "CallAgent::onVideoItemDestroyed: reset incoming video item";
        mIncomingVideo = 0;
    }
}

void CallAgent::handleReadyChannel()
{
    qDebug() << "CallAgent::handleReadyChannel:";

    if (mCallChannel.isNull()) {
        setErrorString(tr("Unable to create call channel for contact %1")
                       .arg(mContact->id()));
        endCall();
        return;
    }

    connect(mCallChannel.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(onChannelInvalidated(Tp::DBusProxy*,QString,QString)));
    connect(mCallChannel.data(),
            SIGNAL(contentAdded(Tpy::CallContentPtr)),
            SLOT(onContentAdded(Tpy::CallContentPtr)));
    connect(mCallChannel.data(),
            SIGNAL(contentRemoved(Tpy::CallContentPtr)),
            SLOT(onContentRemoved(Tpy::CallContentPtr)));
    connect(mCallChannel.data(),
            SIGNAL(localHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)),
            SLOT(onLocalHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)));
    connect(mCallChannel.data(),
            SIGNAL(stateChanged(Tpy::CallState)),
            SLOT(onStateChanged(Tpy::CallState)));

    Tpy::CallContents contents = mCallChannel->contents();
    qDebug() << "CallAgent::handleReadyChannel: number of contents:" << contents.size();
    if (contents.size() > 0) {
        foreach (const Tpy::CallContentPtr &content, contents) {
            Q_ASSERT(!content.isNull());
            qDebug() << "  Call Content";
            Tpy::CallStreams streams = content->streams();
            foreach (const Tpy::CallStreamPtr &stream, streams) {
                qDebug() << "    Call stream: localSendingState=" << stream->localSendingState();
                qDebug() << "      members: " << stream->members().size() ;
                foreach(const Tp::ContactPtr contact, stream->members()) {
                    qDebug() << "        member " << contact->id() << " remoteSendingState=" << stream->remoteSendingState(contact);
                }
                onStreamAdded(stream);
            }
        }
        emit videoSentChanged();
        emit videoSentOrAboutToChanged();
        emit videoReceivedChanged();
        emit audioSentChanged();
        emit audioReceivedChanged();
    }

    // create the gstreamer handler
    Tpy::PendingTfChannel *po = mChannelFactory->createTfChannel(mCallChannel);
    if (!po || po->isError()) {
        setErrorString(po ? po->errorMessage() : "Unable to create farstream channel");
        endCall();
        return;
    }

    connect(po, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onFarstreamChannelCreated(Tp::PendingOperation*)));
}

void CallAgent::onFarstreamStateChanged()
{
    qDebug() << "CallAgent::onFarstreamStateChanged: mFarstreamChannel=" << mFarstreamChannel;

    if (mFarstreamChannel) {
        qDebug() << "CallAgent::onFarstreamStateChanged: state=" << mFarstreamChannel->state();
        if (mFarstreamChannel->state() == Tp::MediaStreamStateConnected) {
            setCallStatus(CallStatusTalking);
        }
    } else {
        setErrorString(tr("Channel not available"));
        endCall();
    }
}

void CallAgent::holdCall()
{
    setHoldCall(true);
}

void CallAgent::unholdCall()
{
    setHoldCall(false);
}

bool CallAgent::heldCall()
{
    return (mCallStatus == CallStatusHeld);
}

QDateTime CallAgent::startTime() const
{
    return mStartTime;
}

QDateTime CallAgent::elapsedTime() const
{
    // returns a QDateTime, since QML Qt.formatTime does not work with QTime
    if (existingCall()) {
        QDateTime ret = QDateTime::currentDateTime();
        qint64 ms = ret.toMSecsSinceEpoch() - mStartTime.toMSecsSinceEpoch();
        QTime time(ms / (1000 * 60 * 60), ms / (1000 * 60) % 60, ms / 1000 % 60, ms % 1000);
        ret.setTime(time);
        return ret;
    }
    return QDateTime();
}

void CallAgent::setStartTime(const QDateTime &startTime)
{
    if (mStartTime != startTime) {
        mStartTime = startTime;
        emit startTimeChanged();
    }
}

double CallAgent::volume() const
{
    if (mFarstreamChannel) {
        return mFarstreamChannel->volume();
    }

    return 1.0;
}

void CallAgent::setVolume(double val)
{
    if (mFarstreamChannel) {
        double currentVolume = mFarstreamChannel->volume();
        if (int(currentVolume*100) != int(val*100)) {
            mFarstreamChannel->setVolume(val);
            emit volumeChanged();
        }
    }
}

void CallAgent::setMuteCall(bool mute)
{
    if (mFarstreamChannel) {
        int mutedCall = mFarstreamChannel->mute();
        if (mutedCall != mute) {
            mFarstreamChannel->setMute(mute);
            emit mutedCallChanged();
        }
    }
}

void CallAgent::muteCall()
{
    setMuteCall(true);
}

void CallAgent::unmuteCall()
{
    setMuteCall(false);
}

bool CallAgent::mutedCall() const
{
    if (mFarstreamChannel) {
        return mFarstreamChannel->mute();
    }

    return false;
}

uint CallAgent::missedAudioCalls() const
{
    return mMissedAudioCalls;
}

uint CallAgent::missedVideoCalls() const
{
    return mMissedVideoCalls;
}

uint CallAgent::missedCalls() const
{
    return missedAudioCalls() + missedVideoCalls();
}

void CallAgent::resetMissedCalls()
{
    mMissedAudioCalls = 0;
    mMissedVideoCalls = 0;
}

QDateTime CallAgent::lastMissedAudioCall() const
{
    return mLastMissedAudioCall;
}

QDateTime CallAgent::lastMissedVideoCall() const
{
    return mLastMissedVideoCall;
}

void CallAgent::reportMissedAudioCall(const QDateTime &time)
{
    mLastMissedAudioCall = time;
    mMissedAudioCalls++;
    emit missedAudioCallsChanged();
}

void CallAgent::reportMissedVideoCall(const QDateTime &time)
{
    mLastMissedVideoCall = time;
    mMissedVideoCalls++;
    emit missedVideoCallsChanged();
}

Tp::AccountPtr CallAgent::account()
{
    return mAccount;
}

Tp::ContactPtr CallAgent::contact()
{
    return mContact;
}

QObject *CallAgent::incomingVideo() const
{
    qDebug() << "CallAgent::incomingVideo: this=" << (void *) this << " item=" << (void *) mIncomingVideo;
    return mIncomingVideo;
}

void CallAgent::setIncomingVideo(QObject *item)
{
    qDebug() << "CallAgent::setIncomingVideo: this=" << (void *) this << " item=" << (void *) item;

    mIncomingVideo = qobject_cast<QmlGstVideoItem *>(item);
    if (mIncomingVideo) {
        connect(item, SIGNAL(destroyed(QObject*)), SLOT(onVideoItemDestroyed(QObject*)));
    }
    if (mFarstreamChannel) {
        mFarstreamChannel->setIncomingVideo(mIncomingVideo);
    }
}

QObject *CallAgent::outgoingVideo() const
{
    qDebug() << "CallAgent::outgoingVideo: this=" << (void *) this << " item=" << (void *) mOutgoingVideo;
    return mOutgoingVideo;
}

void CallAgent::setOutgoingVideo(QObject *item)
{
    qDebug() << "CallAgent::setOutgoingVideo: this=" << (void *) this << " item=" << (void *) item;

    mOutgoingVideo = qobject_cast<QmlGstVideoItem*>(item);
    if (mOutgoingVideo) {
        connect(item, SIGNAL(destroyed(QObject*)), SLOT(onVideoItemDestroyed(QObject*)));
    }
    if (mFarstreamChannel) {
        mFarstreamChannel->setOutgoingVideo(mOutgoingVideo);
    }
}

bool CallAgent::remoteVideoRender() const
{
    return mRemoteVideoRender;
}

void CallAgent::onRemoteVideoRender(bool enabled)
{
    mRemoteVideoRender = enabled;
    emit remoteVideoRenderChanged();
}

void CallAgent::swapCamera()
{
    if (mFarstreamChannel) {
        mFarstreamChannel->swapCamera();
    }
}

bool CallAgent::cameraSwappable() const
{
    if (mFarstreamChannel) {
        return mFarstreamChannel->cameraSwappable();
    }
    return false;
}

bool CallAgent::canSwapVideos() const
{
    if (mFarstreamChannel) {
        return mFarstreamChannel->canSwapVideos();
    }
    return false;
}

const char *CallAgent::streamType(Tp::MediaStreamType streamType)
{
    if (streamType == Tp::MediaStreamTypeAudio) {
        return "audio";
    } else if (streamType == Tp::MediaStreamTypeVideo) {
        return "video";
    } else {
        return "unknown";
    }
}

void CallAgent::onOrientationChanged(uint orientation)
{
    mCurrentOrientation = orientation;
    if (mFarstreamChannel) {
        mFarstreamChannel->onOrientationChanged(orientation);
    }
}

void CallAgent::onAccountConnectionChanged(const Tp::ConnectionPtr &conn)
{
    if (conn == NULL) {
        qDebug() << "NULL connection";
        endCall();
    }
    else {
        connect(conn.data(),
            SIGNAL(statusChanged(Tp::ConnectionStatus)),
            SLOT(onConnectionStatusChanged(Tp::ConnectionStatus)));
        connect(conn.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(onConnectionInvalidated(Tp::DBusProxy*)));
        onConnectionStatusChanged(conn->status());
    }
}

void CallAgent::onConnectionStatusChanged(Tp::ConnectionStatus status)
{
    if(status == Tp::ConnectionStatusDisconnected) {
        endCall();
    }
}

void CallAgent::onConnectionInvalidated(Tp::DBusProxy *proxy)
{
    Tp::Connection *conn = qobject_cast<Tp::Connection *>(proxy);
    if (conn) {
        disconnect(conn, 0, this, 0);
    }
}


void CallAgent::onPendingChanelRequestFinished(Tp::PendingOperation *op)
{
    Q_ASSERT(op);

    qDebug() << "CallAgent::onPendingChanelRequestFinished:";

    if (op->isError()) {
        //: %1 error name - %2 error message
        setErrorString(tr("Unable to create channel: %1 - %2")
                       .arg(op->errorName())
                       .arg(op->errorMessage()));
        endCall();
        return;
    }

    mPendingChannelRequest = 0;
}

void CallAgent::onPendingChanelRequestCreated(const Tp::ChannelRequestPtr &channelRequest)
{
    qDebug() << "CallAgent::onPendingChanelRequestCreated:";

    if (channelRequest.isNull()) {
        setErrorString(tr("Unable to create channel"));
        endCall();
        return;
    }

    mPendingChannelRequest = 0;
}

void CallAgent::onCallChannelReady(Tp::PendingOperation *op)
{
    qDebug() << "CallAgent::onCallChannelReady";

    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);
    if (!pr || pr->isError()) {
        //: %1 contact id - %2 error name - %3 error message
        setErrorString(tr("Unable to create call channel for contact %1 - %2 - %3")
                       .arg(mContact->id())
                       .arg(op->errorName())
                       .arg(op->errorMessage()));
        endCall();
        return;
    }

    if (mCallChannel->isRequested()) {
        // call channel must be accepted always on requested calls
        Tp::PendingOperation *opa = mCallChannel->accept();
        if (!opa) {
            setErrorString(tr("Call could not be accepted"));
            return;
        }

        connect(opa, SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onAcceptCallFinished(Tp::PendingOperation*)));
    }

    handleReadyChannel();
}

void CallAgent::onChannelInvalidated(Tp::DBusProxy *, const QString &errorName, const QString &errorMessage)
{
    qDebug() << "CallAgent::onChannelInvalidated: channel became invalid:" <<
        errorName << "-" << errorMessage;

    if (errorName == TELEPATHY_ERROR_TERMINATED) {
        setCallStatus(CallStatusNoCall);
        endCall();
    } else if (errorName == TELEPATHY_ERROR_NO_ANSWER) {
        setCallStatus(CallStatusNoCall);
        endCall();
    } else if (errorName == TELEPATHY_ERROR_BUSY) {
        setCallStatus(CallStatusNoCall);
        endCall();
    } else if (errorName == TELEPATHY_ERROR_CANCELLED) {
        if (callStatus() == CallStatusIncomingCall) {
            int initialVideo = mCallChannel->immutableProperties().value("InitialVideo",0).toInt();
            if (initialVideo) {
                qDebug() << "CallAgent::onChannelInvalidated: missed video call registered";
                mMissedVideoCalls++;
                mLastMissedVideoCall = QDateTime::currentDateTime();
                emit missedVideoCallsChanged();
            } else {
                // we simplify assuming if no video is coming then it should be audio (not checking even InitialAudio)
                qDebug() << "CallAgent::onChannelInvalidated: missed audio call registered";
                mMissedAudioCalls++;
                mLastMissedAudioCall = QDateTime::currentDateTime();
                emit missedAudioCallsChanged();
            }
        }
        setCallStatus(CallStatusNoCall);
        endCall();
    } else {
        //: %1 contact id - %2 error name - %3 error message
        setErrorString(tr("Invalidated call channel for contact %1 - %2 - %3")
                       .arg(mContact->id())
                       .arg(errorName)
                       .arg(errorMessage));
        endCall();
    }
}

void CallAgent::onContentAdded(const Tpy::CallContentPtr &content)
{
    qDebug() << "CallAgent::onContentAdded: name=" << content->name() << " type=" << streamType(content->type());

    Tpy::CallStreams streams = content->streams();
    foreach (const Tpy::CallStreamPtr &stream, streams) {
        qDebug() << "    Call stream: localSendingState=" << stream->localSendingState();
        qDebug() << "      members: " << stream->members().size() ;
        foreach(const Tp::ContactPtr contact, stream->members()) {
            qDebug() << "        member " << contact->id() << " remoteSendingState=" << stream->remoteSendingState(contact);
        }
        onStreamAdded(stream);
    }

    if (content->type() == Tp::MediaStreamTypeAudio) {
        emit audioSentChanged();
        emit audioReceivedChanged();
    } else if (content->type() == Tp::MediaStreamTypeVideo) {
        emit videoSentChanged();
        emit videoSentOrAboutToChanged();
        emit videoReceivedChanged();
    }

    connect(content.data(),
            SIGNAL(streamAdded(Tpy::CallStreamPtr)),
            SLOT(onStreamAdded(Tpy::CallStreamPtr)));
    connect(content.data(),
            SIGNAL(streamRemoved(Tpy::CallStreamPtr)),
            SLOT(onStreamRemoved(Tpy::CallStreamPtr)));
}

void CallAgent::onContentRemoved(const Tpy::CallContentPtr &content)
{
    qDebug() << "CallAgent::onContentRemoved: name=" << content->name() << " type=" << streamType(content->type());

    Tpy::CallStreams streams = content->streams();
    foreach (const Tpy::CallStreamPtr &stream, streams) {
        qDebug() << "    Call stream: localSendingState=" << stream->localSendingState();
        qDebug() << "      members: " << stream->members().size() ;
        foreach(const Tp::ContactPtr contact, stream->members()) {
            qDebug() << "        member " << contact->id() << " remoteSendingState=" << stream->remoteSendingState(contact);
        }
    }

    if (content->type() == Tp::MediaStreamTypeAudio) {
        emit audioSentChanged();
        emit audioReceivedChanged();
    } else if (content->type() == Tp::MediaStreamTypeVideo) {
        emit videoSentChanged();
        emit videoSentOrAboutToChanged();
        emit videoReceivedChanged();
    }

    disconnect(content.data(), SIGNAL(streamAdded(Tpy::CallStreamPtr)),
               this, SLOT(onStreamAdded(Tpy::CallStreamPtr)));
    disconnect(content.data(), SIGNAL(streamRemoved(Tpy::CallStreamPtr)),
               this, SLOT(onStreamRemoved(Tpy::CallStreamPtr)));
}

void CallAgent::onLocalHoldStateChanged(Tp::LocalHoldState state,
        Tp::LocalHoldStateReason reason)
{
    qDebug() << "CallAgent::onLocalHoldStateChanged: state=" << state << " reason=" << reason;

    if (state == Tp::LocalHoldStateHeld) {
        setCallStatus(CallStatusHeld);
    } else {
        setCallStatus(CallStatusTalking);
    }
}

void CallAgent::onStateChanged(Tpy::CallState state)
{
    qDebug() << "CallAgent::onStateChanged: state=" << state
             << " flags=" << int(mCallChannel->flags())
             << " stateReasonActor=" << mCallChannel->stateReason().actor
             << " stateReason=" << mCallChannel->stateReason().reason
             << " stateDBusReason=" << mCallChannel->stateReason().DBusReason
             << " stateDetails=" << mCallChannel->stateDetails();

    switch(state) {
    case Tpy::CallStateUnknown:
        setCallStatus(CallStatusNoCall);
        break;
    case Tpy::CallStatePendingInitiator:
        setCallStatus(CallStatusConnecting);
        break;
    case Tpy::CallStatePendingReceiver:
        if (mCallChannel->flags() & Tpy::CallFlagLocallyRinging) {
            setCallStatus(CallStatusIncomingCall);
        } else if (mCallChannel->flags() & Tpy::CallFlagQueued) {
            setCallStatus(CallStatusConnecting);
        } else if (mCallChannel->flags() & Tpy::CallFlagForwarded) {
            setCallStatus(CallStatusConnecting);
        } else if (mCallChannel->flags() & Tpy::CallFlagInProgress) {
            setCallStatus(CallStatusConnecting);
        } else if (mCallChannel->flags() & Tpy::CallFlagLocallyHeld) {
            setCallStatus(CallStatusHeld);
        } else {
            setCallStatus(CallStatusRinging);
        }
        break;
    case Tpy::CallStateAccepted:
        if (mCallChannel->flags() & Tpy::CallFlagLocallyHeld) {
            setCallStatus(CallStatusHeld);
        } else if (mCallChannel->flags() & Tpy::CallFlagForwarded) {
            setCallStatus(CallStatusConnecting);
        } else {
            setCallStatus(CallStatusTalking);
        }
        break;
    case Tpy::CallStateEnded:
        if (callStatus() == CallStatusIncomingCall) {
            int initialVideo = mCallChannel->immutableProperties().value("InitialVideo",0).toInt();
            if (initialVideo) {
                qDebug() << "CallAgent::onStateChanged: missed video call registered";
                mMissedVideoCalls++;
                mLastMissedVideoCall = QDateTime::currentDateTime();
                emit missedVideoCallsChanged();
            } else {
                // we simplify assuming if no video is coming then it should be audio (not checking even InitialAudio)
                qDebug() << "CallAgent::onStateChanged: missed audio call registered";
                mMissedAudioCalls++;
                mLastMissedAudioCall = QDateTime::currentDateTime();
                emit missedAudioCallsChanged();
            }
        }
        if (callStatus() != CallStatusHangingUp) {
            endCall();
        }
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

void CallAgent::onStreamAdded(const Tpy::CallStreamPtr &stream)
{
    qDebug() << "CallAgent::onStreamAdded: stream created type:" << streamType(stream->content()->type())
             << " localSendingState:" << stream->localSendingState();

    qDebug() << "  members " << stream->members().size();
    foreach(const Tp::ContactPtr contact, stream->members()) {
        qDebug() << "    member " << contact->id() << " remoteSendingState=" << stream->remoteSendingState(contact);
    }

    connect(stream.data(),
            SIGNAL(localSendingStateChanged(Tpy::SendingState)),
            SLOT(onLocalSendingStateChanged(Tpy::SendingState)));
    connect(stream.data(),
            SIGNAL(remoteSendingStateChanged(QHash<Tp::ContactPtr,Tpy::SendingState>)),
            SLOT(onRemoteSendingStateChanged(QHash<Tp::ContactPtr,Tpy::SendingState>)));
    connect(stream.data(),
            SIGNAL(remoteMembersRemoved(Tp::Contacts)),
            SLOT(onRemoteMembersRemoved(Tp::Contacts)));
}

void CallAgent::onStreamRemoved(const Tpy::CallStreamPtr &stream)
{
    qDebug() << "CallAgent::onStreamRemoved: stream removed type=" << streamType(stream->content()->type())
             << " localSendingState:" << stream->localSendingState();

    qDebug() << "  members " << stream->members().size();
    foreach(const Tp::ContactPtr contact, stream->members()) {
        qDebug() << "    member " << contact->id() << " remoteSendingState=" << stream->remoteSendingState(contact);
    }

    disconnect(stream.data(), SIGNAL(localSendingStateChanged(Tpy::SendingState)),
               this, SLOT(onLocalSendingStateChanged(Tpy::SendingState)));
    disconnect(stream.data(), SIGNAL(remoteSendingStateChanged(QHash<Tp::ContactPtr,Tpy::SendingState>)),
               this, SLOT(onRemoteSendingStateChanged(QHash<Tp::ContactPtr,Tpy::SendingState>)));
    disconnect(stream.data(), SIGNAL(remoteMembersRemoved(Tp::Contacts)),
               this, SLOT(onRemoteMembersRemoved(Tp::Contacts)));
}

void CallAgent::onLocalSendingStateChanged(Tpy::SendingState localSendingState)
{
    qDebug() << "CallAgent::onLocalSendingStateChanged: localSendingState=" << localSendingState;
}

void CallAgent::onRemoteSendingStateChanged(const QHash< Tp::ContactPtr, Tpy::SendingState > &remoteSendingStates)
{
    qDebug() << "CallAgent::onRemoteSendingStateChanged: size=" << remoteSendingStates.size();

    bool hasIncomingVideo = false;
    QHash<Tp::ContactPtr, Tpy::SendingState>::const_iterator i = remoteSendingStates.begin();
    while (i != remoteSendingStates.end()) {
        Tpy::SendingState remoteSendingState =  i.value();
        qDebug() << "  contact " << i.key()->id() << " sendingState=" << remoteSendingState;
        if (remoteSendingState ==Tpy::SendingStateSending)
            hasIncomingVideo = true;
        ++i;
    }

    if (mFarstreamChannel) {
        mFarstreamChannel->onIncomingVideo(hasIncomingVideo);
        onRemoteVideoRender(hasIncomingVideo);
    }
}

void CallAgent::onRemoteMembersRemoved(const Tp::Contacts &remoteMembers)
{
    qDebug() << "CallAgent::onRemoteSendingStateChanged: size=" << remoteMembers.size();

    foreach(Tp::ContactPtr contact, remoteMembers) {
        qDebug() << "  contact " << contact->id();
    }
}

void CallAgent::onRequestSendingVideoFinished(Tp::PendingOperation *op)
{
    qDebug() << "CallAgent::onRequestSendingVideoFinished";

    if (!op || op->isError()) {
        qDebug() << "CallAgent::onRequestSendingVideoFinished: error";
        return;
    }

    emit videoSentChanged();
    emit videoSentOrAboutToChanged();
}

void CallAgent::onRequestSendingAudioFinished(Tp::PendingOperation *op)
{
    qDebug() << "CallAgent::onRequestSendingAudioFinished";

    if (!op || op->isError()) {
        qDebug() << "CallAgent::onRequestSendingAudioFinished: error";
        return;
    }

    emit audioSentChanged();
}

void CallAgent::handleRequestContentFinished(Tp::PendingOperation *op, bool switchOn)
{
    qDebug() << "CallAgent::onRequestContentFinished";
    Q_UNUSED(op);

    Tpy::PendingCallContent * pcc = qobject_cast<Tpy::PendingCallContent*>(op);
    if (!pcc || pcc->isError()) {
        return;
    }

    Tpy::CallContentPtr content = pcc->content();
    if (content.isNull()) {
        return;
    }

    if (content->type() == Tp::MediaStreamTypeAudio) {
        qDebug() << "Just got audio added";
        if (switchOn) {
            requestSendingAudio(content, true);
            return;
        }
        emit audioSentChanged();
        emit audioReceivedChanged();
    } else if (content->type() == Tp::MediaStreamTypeVideo) {
        qDebug() << "Just got video added";
        if (switchOn) {
            requestSendingVideo(content, true);
            return;
        }
        emit videoSentChanged();
        emit videoSentOrAboutToChanged();
        emit videoReceivedChanged();
    }
}

void CallAgent::onRequestContentFinished(Tp::PendingOperation *op)
{
    handleRequestContentFinished(op, false);
}

void CallAgent::onRequestContentFinishedSwitchOn(Tp::PendingOperation *op)
{
    handleRequestContentFinished(op, true);
}

void CallAgent::onRemoveContentFinished(Tp::PendingOperation *op)
{
    qDebug() << "CallAgent::onRemoveContentFinished";
    Q_UNUSED(op);
    enum { type_audio = 1, type_video = 2, type_unknown = 3 } type = type_unknown;

    Tpy::PendingCallContent * pcc = qobject_cast<Tpy::PendingCallContent*>(op);
    if (pcc && !pcc->isError()) {
        Tpy::CallContentPtr content = pcc->content();
        if (!content.isNull()) {
            if (content->type() == Tp::MediaStreamTypeAudio) {
                qDebug() << "Just got audio removed";
                type = type_audio;
            }
            else if (content->type() == Tp::MediaStreamTypeVideo) {
                qDebug() << "Just got video removed";
                type = type_video;
            }
        }
    }

    if (type & type_audio) {
        emit audioSentChanged();
        emit audioReceivedChanged();
    }
    if (type & type_video) {
        emit videoSentChanged();
        emit videoSentOrAboutToChanged();
        emit videoReceivedChanged();
    }
}

void CallAgent::onRequestCloseFinished(Tp::PendingOperation *op)
{
    qDebug() << "CallAgent::onRequestCloseFinished:";

    if (op && op->isError()) {
        //: %1 error name - %2 error message
        setErrorString(tr("Error ending call: %1 - %2")
                       .arg(op->errorName())
                       .arg(op->errorMessage()));
        return;
    }

    setCallStatus(CallStatusNoCall);
}

void CallAgent::onAcceptCallFinished(Tp::PendingOperation *op)
{
    qDebug() << "CallAgent::onAcceptCallFinished:";

    if (op->isError()) {
        //: %1 error name - %2 error message
        setErrorString(tr("Call could not be accepted %1 - %2")
                       .arg(op->errorName())
                       .arg(op->errorMessage()));
        endCall();
        return;
    }
    emit acceptCallFinished(this);
}

void CallAgent::onRequestHoldFinished(Tp::PendingOperation *op)
{
    qDebug() << "CallAgent::onRequestHoldFinished: "
             << " localHoldState=" << mCallChannel->localHoldState()
             << " localHoldStateReason=" << mCallChannel->localHoldStateReason();

    if (!op || op->isError()) {
        //: %1 error name - %2 error message
        setErrorString(tr("Hold request could not be done %1 - %2")
                       .arg(op ? op->errorName() : "")
                       .arg(op ? op->errorMessage(): ""));
        return;
    }

    if (mCallChannel->localHoldState() == Tp::LocalHoldStateHeld) {
        setCallStatus(CallStatusHeld);
    } else {
        setCallStatus(CallStatusTalking);
    }
}

void CallAgent::onFarstreamChannelCreated(Tp::PendingOperation *po)
{
    qDebug() << "CallAgent::onFarstreamChannelCreated: po=" << po;

    Tpy::PendingTfChannel *pendingTfChannel = static_cast<Tpy::PendingTfChannel*> (po);
    if (!pendingTfChannel || pendingTfChannel->isError() || !pendingTfChannel->tfChannel()) {
        setErrorString(pendingTfChannel ? pendingTfChannel->errorMessage() : "Unable to create farstream channel");
        endCall();
        return;
    }

    // create the gstreamer handler
    mFarstreamChannel = new FarstreamChannel(pendingTfChannel->tfChannel(), this);
    setupFarstreamChannel(mFarstreamChannel);
}

Tpy::CallStateReason CallAgent::stateReason() const
{
    Tpy::CallStateReason ret;
    if (mCallChannel) {
        ret = mCallChannel->stateReason();
    } else {
        ret.actor = 0;
        ret.reason = Tpy::CallStateChangeReasonUnknown;
    }
    return ret;
}

bool CallAgent::isRequested() const
{
    return mIsRequested;
}

QTime CallAgent::updateCallDuration()
{
    if (mCallStatus == CallStatusTalking) {
        QDateTime currentTime = QDateTime::currentDateTime();
        quint64 ms = mStartTime.msecsTo(currentTime);
        int hh = ms / 3600000;
        ms = ms % 3600000;
        int mm = ms / 60000;
        ms = ms % 60000;
        int ss = ms / 1000;
        //ms = ms % 1000;
        mCallDuration.setHMS(hh, mm, ss, 0);
    }
    return mCallDuration;
}

void CallAgent::setupResourceSets()
{
    qDebug() << "CallAgent::setupResourceSets";

    if (!mResourceSetCall) {
        mResourceSetCall = new ResourcePolicy::ResourceSet("call", 0, true, true);
        if (mResourceSetCall) {
            bool ret = mResourceSetCall->setAutoRelease();
            qDebug() << "setAutoRelease returned " << ret;
            ret = mResourceSetCall->setAlwaysReply();
            qDebug() << "setAlwaysReply returned " << ret;

            ResourcePolicy::AudioResource *audioResource = new ResourcePolicy::AudioResource("call");
            if (audioResource) {
                audioResource->setProcessID(QCoreApplication::applicationPid());
                audioResource->setStreamTag("media.name", "*");
                audioResource->setOptional(false);
                mResourceSetCall->addResourceObject(audioResource);
            }

            ret = mResourceSetCall->addResource(ResourcePolicy::AudioRecorderType);
            qDebug() << "addResource returned " << ret;

            ret = mResourceSetCall->initAndConnect();
            qDebug() << "initAndConnect returned " << ret;
        }
    }
}

void CallAgent::freeResourceSets()
{
    qDebug() << "CallAgent::freeResourceSets";

    if (mResourceSetCall) {
        ensureResourceSetConfiguration(true);
        mResourceSetCall->release();
        delete mResourceSetCall;
        mResourceSetCall = 0;
    }
}

void CallAgent::connectResourceSets()
{
    if (mResourceSetCall) {
        connect(mResourceSetCall,
                SIGNAL(resourcesGranted(QList<ResourcePolicy::ResourceType>)),
                SLOT(onResourceSetCallGranted()));
        connect(mResourceSetCall,
                SIGNAL(lostResources()),
                SLOT(onResourceSetCallLost()));
        connect(mResourceSetCall,
                SIGNAL(resourcesDenied()),
                SLOT(onResourceSetCallDenied()));
        connect(mResourceSetCall,
                SIGNAL(errorCallback(quint32,const char*)),
                SLOT(onResourceSetCallError(quint32,const char *)));
        // next signals used for debugging resource life cycle
        connect(mResourceSetCall,
                SIGNAL(resourcesBecameAvailable(QList<ResourcePolicy::ResourceType>)),
                SLOT(onResourcesBecameAvailable(QList<ResourcePolicy::ResourceType>)));
        connect(mResourceSetCall,
                SIGNAL(resourcesReleased()),
                SLOT(onResourcesReleased()));
        connect(mResourceSetCall,
                SIGNAL(resourcesReleasedByManager()),
                SLOT(onResourcesReleasedByManager()));
    }
}

void CallAgent::disconnectResourceSets()
{
    if (mResourceSetCall) {
        disconnect(mResourceSetCall, SIGNAL(resourcesGranted(QList<ResourcePolicy::ResourceType>)),
                   this, SLOT(onResourceSetCallGranted()));
        disconnect(mResourceSetCall, SIGNAL(lostResources()),
                this, SLOT(onResourceSetCallLost()));
        disconnect(mResourceSetCall, SIGNAL(resourcesDenied()),
                this, SLOT(onResourceSetCallDenied()));
        disconnect(mResourceSetCall, SIGNAL(errorCallback(quint32,const char*)),
                this, SLOT(onResourceSetCallError(quint32,const char *)));
        disconnect(mResourceSetCall,
                SIGNAL(resourcesBecameAvailable(QList<ResourcePolicy::ResourceType>)),
                this, SLOT(onResourcesBecameAvailable(QList<ResourcePolicy::ResourceType>)));
        disconnect(mResourceSetCall,
                SIGNAL(resourcesReleased()),
                this, SLOT(onResourcesReleased()));
        disconnect(mResourceSetCall,
                SIGNAL(resourcesReleasedByManager()),
                this, SLOT(onResourcesReleasedByManager()));
    }
}

void CallAgent::onResourceSetCallGranted()
{
    qDebug() << "CallAgent::onResourceSetCallGranted: callStatus=" << callStatus();

    mWaitingForResources = false;
    if (!mOnAcquireInvoke.isEmpty()) {
        QTimer::singleShot(0, this, qPrintable(mOnAcquireInvoke));
        mOnAcquireInvoke = QString();
    }
}

void CallAgent::onResourceSetCallLost()
{
    qDebug() << "CallAgent::onResourceSetCallLost";

    mWaitingForResources = false;
    mOnAcquireInvoke = QString();
    setErrorString(tr("Call resources were lost"));
    endCall();
}

void CallAgent::onResourceSetCallDenied()
{
    qDebug() << "CallAgent::onResourceSetCallDenied";

    mWaitingForResources = false;
    mOnAcquireInvoke = QString();
    setErrorString(tr("Call resources not granted"));
    endCall();
}

void CallAgent::onResourceSetCallError(quint32 error,const char *message)
{
    qDebug() << "CallAgent::onResourceSetCallError: error=" << error << " msg=" << message;

    mWaitingForResources = false;
    mOnAcquireInvoke = QString();
    setErrorString(tr("Error acquiring call resources"));
    endCall();
}

void CallAgent::onResourcesBecameAvailable(QList<ResourcePolicy::ResourceType> resources)
{
    qDebug() << "CallAgent::onResourcesBecameAvailable" << resources;
}

void CallAgent::onResourcesReleased()
{
    qDebug() << "CallAgent::onResourcesReleased";
}

void CallAgent::onResourcesReleasedByManager()
{
    qDebug() << "CallAgent::onResourcesReleasedByManager";
}

bool CallAgent::useResourcePolicy() const
{
    return mUseResourcePolicy;
}

void CallAgent::setUseResourcePolicy(bool urp)
{
    mUseResourcePolicy = urp;
    emit useResourcePolicyChanged();
}
