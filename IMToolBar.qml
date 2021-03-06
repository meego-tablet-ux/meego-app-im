/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Ux.Components.Common 0.1
import MeeGo.App.IM 0.1
import MeeGo.Media 0.1
import TelepathyQML 0.1
import "utils.js" as Utils

BottomToolBar {
    id: toolBar

    signal chatTextEnterPressed
    signal smileyClicked(string sourceName)

    Connections {
        target: window.contactItem
        onCapabilitiesChanged: {
            //console.log("AudioCallRole " + window.contactItem.data(AccountsModel.AudioCallCapabilityRole) + " " + audioCallButtonOpacity());
            //console.log("VideoCallRole" + window.contactItem.data(AccountsModel.VideoCallWithAudioCapabilityRole) + " " + videoCallButtonOpacity());
            //console.log("FileTransferRole " + window.contactItem.data(AccountsModel.FileTransferCapabilityRole) + " " + sendFileButtonOpacity());
            videoCallButton.opacity = videoCallButtonOpacity();
            videoOnOffButton.opacity = videoOnOffButtonOpacity();
            audioCallButton.opacity = audioCallButtonOpacity();
            sendFileButton.opacity = sendFileButtonOpacity();
        }
    }

    function videoCallButtonOpacity() {
        return (window.callAgent == undefined && !window.chatAgent.isConference) ||
                (!window.callAgent.existingCall &&
                window.contactItem.data(AccountsModel.VideoCallWithAudioCapabilityRole) &&
                window.callAgent != undefined &&
                window.callAgent.callStatus == CallAgent.CallStatusNoCall &&
                window.chatAgent != undefined &&
                !window.chatAgent.isConference) ? 1 : 0
    }

    function videoOnOffButtonOpacity() {
        return (window.contactItem !== undefined &&
                window.contactItem.data(AccountsModel.VideoCallWithAudioCapabilityRole) &&
                window.callAgent != undefined && window.callAgent.existingCall &&
                window.callAgent.callStatus != CallAgent.CallStatusNoCall &&
                !(window.chatAgent != undefined && window.chatAgent.isConference)) ? 1 : 0;
    }

    function audioCallButtonOpacity() {
        return (window.contactItem !== undefined &&
                window.contactItem.data(AccountsModel.AudioCallCapabilityRole) &&
                window.callAgent != undefined &&
                window.callAgent.callStatus == CallAgent.CallStatusNoCall &&
                window.chatAgent != undefined &&
                !window.chatAgent.isConference) ? 1 : 0;
    }

    function sendFileButtonOpacity() {
        return (!window.fullScreen &&
                window.contactItem != undefined &&
                window.contactItem.data(AccountsModel.FileTransferCapabilityRole)) ? 1 : 0;
    }


    // Bottom row to initiate calls & chat
    content: BottomToolBarRow {
        id: bottomRow
        contentVerticalMargins: 0

        leftContent: [
            IconButton {
                id: endCallButton
                width: 120
                active: window.callAgent != undefined &&
                        window.contactItem != undefined &&
                        //window.contactItem.data(AccountsModel.AudioCallCapabilityRole) &&
                        window.callAgent.callStatus != CallAgent.CallStatusNoCall
                opacity: window.callAgent != undefined &&
                         window.callAgent.callStatus != CallAgent.CallStatusNoCall ? 1 : 0
                visible: opacity > 0

                icon: "image://themedimage/icons/actionbar/call-audio-stop"
                iconDown: icon + "-active"
                hasBackground: true
                bgSourceUp: "image://themedimage/widgets/common/button/button-negative"
                bgSourceDn: "image://themedimage/widgets/common/button/button-negative-pressed"

                onClicked: {
                    window.callAgent.endCall();
                }
                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }

                Image
                {
                    anchors.right: parent.right
                    source: "image://themedimage/widgets/common/action-bar/action-bar-separator"
                    height: parent.height
                }
            },


            IconButton {
                id: videoCallButton
                width: 60
                opacity: videoCallButtonOpacity()
                visible: opacity > 0

                icon: "image://themedimage/icons/actionbar/turn-video-on"
                iconDown: icon + "-active"
                hasBackground: false
                onClicked: {
                    messageScreenPage.loadVideoWindow(videoCallButton);
                }
                function onVideoWindowLoaded(videoWindow) {
                    videoWindow.opacity = 1;
                    window.callAgent.setOutgoingVideo(videoWindow.cameraWindowSmall ? videoWindow.videoOutgoing : videoWindow.videoIncoming);
                    window.callAgent.onOrientationChanged(window.orientation);
                    window.callAgent.setIncomingVideo(videoWindow.cameraWindowSmall ? videoWindow.videoIncoming : videoWindow.videoOutgoing);
                    //window.callAgent.useResourcePolicy = false;
                    window.callAgent.videoCall();
                }
                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
                Image
                {
                    anchors.right: parent.right
                    source: "image://themedimage/widgets/common/action-bar/action-bar-separator"
                    height: parent.height
                }
            },

            IconButton {
                id: videoOnOffButton
                width: 60
                opacity: videoOnOffButtonOpacity()
                visible: opacity > 0

                icon: window.callAgent.videoSentOrAboutTo ?
                           "image://themedimage/icons/actionbar/turn-video-off" :
                           "image://themedimage/icons/actionbar/turn-video-on"
                iconDown: icon + "-active"
                hasBackground: false
                onClicked: {
                    window.callAgent.videoSent = !window.callAgent.videoSent;
                }
                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
                Image
                {
                    anchors.right: parent.right
                    source: "image://themedimage/widgets/common/action-bar/action-bar-separator"
                    height: parent.height
                }
            },

            IconButton {
                id: audioCallButton
                width: 60
                opacity: audioCallButtonOpacity()
                visible: opacity > 0

                icon: "image://themedimage/icons/actionbar/call-audio-start"
                iconDown: icon + "-active"
                hasBackground: false
                onClicked: {
                    messageScreenPage.loadVideoWindow(audioCallButton);
                }
                function onVideoWindowLoaded(videoWindow) {
                    videoWindow.opacity = 1;
                    window.callAgent.setOutgoingVideo(videoWindow.cameraWindowSmall ? videoWindow.videoOutgoing : videoWindow.videoIncoming);
                    window.callAgent.onOrientationChanged(window.orientation);
                    window.callAgent.setIncomingVideo(videoWindow.cameraWindowSmall ? videoWindow.videoIncoming : videoWindow.videoOutgoing);
                    //window.callAgent.useResourcePolicy = false;
                    window.callAgent.audioCall();
                }
                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
                Image
                {
                    anchors.right: parent.right
                    source: "image://themedimage/widgets/common/action-bar/action-bar-separator"
                    height: parent.height
                }
            },

            IconButton {
                id: volumeOnButton
                width: 60
                icon: "image://themedimage/icons/actionbar/turn-audio-off"
                iconDown: icon + "-active"
                hasBackground: false
                onClicked: {
                    if (volumeLoader.sourceComponent != null ) {
                        volumeLoader.sourceComponent = null;
                    } else {
                        volumeLoader.sourceComponent = volumeControlComponent;
                        volumeLoader.item.parent = toolBar.parent;
                        volumeLoader.item.z = 1000
                        volumeLoader.item.volumeControl = volumeControl;
                        volumeLoader.item.volumeControlX = volumeOnButton.x + (volumeOnButton.width - volumeLoader.item.volumeWidth) / 2;
                        volumeLoader.item.volumeControlY = toolBar.y - volumeLoader.item.volumeHeight;
                        volumeLoader.item.closeTimer.interval = 3000;
                        volumeLoader.item.closeTimer.restart();
                    }
                }
                opacity: window.callAgent != undefined && window.callAgent.existingCall ? 1 : 0
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
                Image
                {
                    anchors.right: parent.right
                    source: "image://themedimage/widgets/common/action-bar/action-bar-separator"
                    height: parent.height
                }
            },

            IconButton {
                id: muteButton
                width: 60
                icon: window.callAgent.mutedCall ?
                        "image://themedimage/icons/actionbar/microphone-unmute" :
                        "image://themedimage/icons/actionbar/microphone-mute"
                iconDown: icon + "-active"
                hasBackground: false
                onClicked: {
                    window.callAgent.setMuteCall(!window.callAgent.mutedCall);
                }
                opacity: window.callAgent != undefined && window.callAgent.existingCall ? 1 : 0
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
                Image
                {
                    anchors.right: parent.right
                    source: "image://themedimage/widgets/common/action-bar/action-bar-separator"
                    height: parent.height
                }
            },

            IconButton {
                id: fullscreenButton
                width: 60
                icon: "image://themedimage/icons/actionbar/view-" +
                       (window.fullScreen ? "smallscreen" : "fullscreen")
                iconDown: icon + "-active"
                hasBackground: false
                onClicked: {
                    window.fullScreen = !window.fullScreen
                }
                opacity: (window.callAgent != undefined && window.callAgent.existingCall) ? 1 : 0
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
                Image
                {
                    anchors.right: parent.right
                    source: "image://themedimage/widgets/common/action-bar/action-bar-separator"
                    height: parent.height
                }
            }
        ]

        rightContent: [
            Text {
                id: callInfoText
                elide: Text.ElideLeft
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignRight
                text: Utils.getCallStatusText(window.callAgent)
                color: theme_buttonFontColor

                Timer {
                    //running: window.callAgent.callStatus == CallAgent.CallStatusTalking
                    running: true
                    interval: 1000
                    repeat: true
                    onTriggered: {
                        callInfoText.text = Utils.getCallStatusText(window.callAgent);
                    }
                }
                opacity: window.callAgent != undefined && window.callAgent.existingCall ? 1 : 0
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
                Image
                {
                    visible: insertSmileyButton.visible || sendFileButton.visible
                    anchors.right: parent.right
                    source: "image://themedimage/widgets/common/action-bar/action-bar-separator"
                    height: parent.height
                }
            },

            IconButton {
                id: insertSmileyButton
                width: 60
                icon: "image://themedimage/icons/actionbar/insert-emote"
                iconDown: icon + "-active"
                hasBackground: false

                onClicked: {
                    if (smileyContextMenuLoader.item == null) {
                        smileyContextMenuLoader.sourceComponent = smileyContextMenuComponent;
                    }

                    var map = insertSmileyButton.mapToItem(messageTopItem, insertSmileyButton.x, insertSmileyButton.y)

                    smileyContextMenuLoader.item.setPosition(map.x + insertSmileyButton.width / 2,
                                                  map.y + insertSmileyButton.height * 2);
                    smileyContextMenuLoader.item.show();
                }
                opacity: !window.fullScreen ? 1 : 0
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
                Image
                {
                    visible: sendFileButton.visible
                    anchors.right: parent.right
                    source: "image://themedimage/widgets/common/action-bar/action-bar-separator"
                    height: parent.height
                }
            },

            IconButton {
                id: sendFileButton
                width: 60
                icon: "image://themedimage/icons/actionbar/document-attach"
                iconDown: icon + "-active"
                onClicked: {
                    if (sendFileContextMenuLoader.item == null) {
                        sendFileContextMenuLoader.sourceComponent = sendFileContextMenuComponent;
                    }

                    var map = sendFileButton.mapToItem(messageTopItem, sendFileButton.x, sendFileButton.y)
                    sendFileContextMenuLoader.item.setPosition(map.x + sendFileButton.width / 2,
                                                    map.y + sendFileButton.height * 2);
                    sendFileContextMenuLoader.item.show();
                }
                opacity: sendFileButtonOpacity()
                visible: opacity > 0

                hasBackground: false
                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
                Image
                {
                    anchors.right: parent.right
                    source: "image://themedimage/widgets/common/action-bar/action-bar-separator"
                    height: parent.height
                }
            }
        ]

        Loader {
            id: smileyContextMenuLoader
        }

        Component {
            id:  smileyContextMenuComponent
            ContextMenu {
                id: smileyContextMenu

                content: SmileyGridView {
                    id: smileyGrid
                    height: 200
                    width: 200

                    onSmileyClicked: {
                        toolBar.smileyClicked(sourceName);
                        smileyContextMenu.hide();
                    }
                }

                forceFingerMode: 3
                width: 200
                height: 200
            }
        }


        Loader {
            id: sendFileContextMenuLoader
        }

        Component {
            id: sendFileContextMenuComponent
            ContextMenu {
                id: sendFileContextMenu

                content: SendFileView {
                    id: sendFileView

                    onFileSelected: {
                        fileTransferAgent.sendFile(fileName);
                        sendFileContextMenu.hide();
                    }
                    onCancelled: {
                        sendFileContextMenu.hide();
                    }
                }

                forceFingerMode: 3

                Component.onCompleted: {
                    if (sendFileContextMenu.content == null)
                        sendFileButton.visible = false
                }
            }
        }

        Component {
            id: volumeControlComponent
            VolumeSlider {
                onClose: {
                    volumeLoader.sourceComponent = undefined;
                }
            }
        }

        Item {
            id: volumeControl

            property int volume: 100
            property bool mute: false

            onVolumeChanged: {
                window.callAgent.volume = volumeControl.volume / 100;
            }
            onMuteChanged: {
                window.callAgent.mutedCall = volumeControl.mute;
            }

            Component.onCompleted: {
                volumeControl.volume = window.callAgent.volume * 100;
                volumeControl.mute = window.callAgent.mutedCall;
            }
        }

        Loader {
            id: volumeLoader
        }
    }

    Component.onCompleted: {
        toolBar.show();
    }

}
