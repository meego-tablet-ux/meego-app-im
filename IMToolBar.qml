/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Labs.Components 0.1
import MeeGo.App.IM 0.1
import TelepathyQML 0.1

Item {
    id: toolBar

    signal chatTextEnterPressed
    signal smileyClicked(string sourceName)

    height: background.height

    Behavior on height {
        NumberAnimation {
            duration: 500
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 500
        }
    }

    Image {
        id: background
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        source: "image://meegotheme/widgets/common/action-bar/action-bar-background"
    }

    // Bottom row to initiate calls & chat
    Item {
        id: bottomRow
        anchors.fill: parent

        property int numButtons : (scene.callAgent != undefined && scene.callAgent.callStatus != CallAgent.CallStatusNoCall) ? 3 : 4

        function calculateButtonX (button) {
            var buttonAreaWidth = width / button.parent.numButtons;
            var x = buttonAreaWidth * button.numButton;
            x += (buttonAreaWidth - button.width) / 2;
            return x;
        }

        IconButton {
            id: videoCallButton
            property int numButton: 0
            x : parent.calculateButtonX(videoCallButton)
            active: scene.contactItem.data(AccountsModel.VideoCallWithAudioCapabilityRole) && (scene.callAgent != undefined && scene.callAgent.callStatus == CallAgent.CallStatusNoCall)
            anchors.verticalCenter: parent.verticalCenter
            icon: "image://meegotheme/icons/actionbar/turn-video-on"
            iconDown: icon + "-active"
            anchors.margins: 10
            onClicked: {
                videoWindow.opacity = 1;
                scene.callAgent.setOutgoingVideo(cameraWindowSmall ? videoOutgoing : videoIncoming);
                scene.callAgent.onOrientationChanged(scene.orientation);
                scene.callAgent.setIncomingVideo(cameraWindowSmall ? videoIncoming : videoOutgoing);
                scene.callAgent.videoCall();
            }

            // it shouldn't be necessary to re-declare that here, but
            // defining a behavior on opacity is breaking it
            opacity: active ? 1.0 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 500
                }
            }
            Behavior on x {
                NumberAnimation {
                    duration: 500
                }
            }
        }

        IconButton {
            id: audioCallButton
            property int numButton: 1
            x : parent.calculateButtonX(audioCallButton)
            active: scene.contactItem.data(AccountsModel.AudioCallCapabilityRole) && (scene.callAgent != undefined && scene.callAgent.callStatus == CallAgent.CallStatusNoCall)
            anchors.verticalCenter: parent.verticalCenter
            icon: "image://meegotheme/icons/actionbar/call-audio-start"
            iconDown: icon + "-active"
            anchors.margins: 10
            onClicked: {
                videoWindow.opacity = 1;
                scene.callAgent.setOutgoingVideo(cameraWindowSmall ? videoOutgoing : videoIncoming);
                scene.callAgent.onOrientationChanged(scene.orientation);
                scene.callAgent.setIncomingVideo(cameraWindowSmall ? videoIncoming : videoOutgoing);
                scene.callAgent.audioCall();
            }
            // it shouldn't be necessary to re-declare that here, but
            // defining a behavior on opacity is breaking it
            opacity: active ? 1.0 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 500
                }
            }
            Behavior on x {
                NumberAnimation {
                    duration: 500
                }
            }
        }

        IconButton {
            id: endCallButton
            property int numButton: 0
            x : parent.calculateButtonX(endCallButton)
            enabled: scene.callAgent != undefined && scene.contactItem.data(AccountsModel.AudioCallCapabilityRole) && scene.callAgent.callStatus != CallAgent.CallStatusNoCall
            anchors.verticalCenter: parent.verticalCenter
            icon: "image://meegotheme/icons/actionbar/call-audio-stop"
            iconDown: icon + "-active"
            anchors.margins: 10

            onClicked: {
                scene.callAgent.endCall();
            }

            opacity: enabled ? 1.0 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 500
                }
            }
            Behavior on x {
                NumberAnimation {
                    duration: 500
                }
            }
        }

        AbstractContext {
            id: smileyContextMenu

            content: SmileyGridView {
                id: smileyGrid

                onSmileyClicked: {
                    toolBar.smileyClicked(sourceName);
                    smileyContextMenu.visible = false;
                }
            }
            mouseX: insertSmileyButton.x + insertSmileyButton.width / 2
            mouseY: insertSmileyButton.y + insertSmileyButton.height + toolBar.y
            fingerMode: 3
            contentWidth: 200
            contentHeight: 200

            Component.onCompleted: {
                insertSmileyButton.visible = smileyContextMenu.content != null
            }
        }

        AbstractContext {
            id: sendFileContextMenu

            content: SendFileView {
                id: sendFileView
                anchors.bottom: sendFileButton.bottom
                anchors.horizontalCenter: sendFileButton.horizontalCenter
                anchors.horizontalCenterOffset: -(width / 4 + 30)

                onFileSelected: {
                    fileTransferAgent.sendFile(fileName);
                    sendFileContextMenu.visible = false;
                }
            }

            mouseX: sendFileButton.x + sendFileButton.width / 2
            mouseY: sendFileButton.y + sendFileButton.height + toolBar.y
            fingerMode: 3

            Component.onCompleted: {
                sendFileButton.visible = sendFileContextMenu.content != null
            }
        }

        IconButton {
            id: insertSmileyButton
            property int numButton: parent.numButtons - 2
            x : parent.calculateButtonX(insertSmileyButton)
            anchors.verticalCenter: parent.verticalCenter
            icon: "image://meegotheme/icons/actionbar/insert-emote"
            iconDown: icon + "-active"
            anchors.margins: 10

            onClicked: {
                smileyContextMenu.visible = true;
            }

            Behavior on x {
                NumberAnimation {
                    duration: 500
                }
            }
        }

        IconButton {
            id: sendFileButton
            property int numButton: parent.numButtons - 1
            x : parent.calculateButtonX(sendFileButton)
            anchors.verticalCenter: parent.verticalCenter
            icon: "image://meegotheme/icons/actionbar/document-attach"
            iconDown: icon + "-active"
            anchors.margins: 10
            onClicked: sendFileContextMenu.visible = !sendFileContextMenu.visible
            visible: (scene.contactItem != undefined && scene.contactItem.data(AccountsModel.FileTransferCapabilityRole))

            Behavior on x {
                NumberAnimation {
                    duration: 500
                }
            }
        }
    }

}
