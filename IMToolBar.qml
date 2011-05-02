/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Components 0.1
import MeeGo.Labs.Components 0.1 as Labs
import MeeGo.App.IM 0.1
import TelepathyQML 0.1
import "utils.js" as Utils

BottomToolBar {
    id: toolBar

    signal chatTextEnterPressed
    signal smileyClicked(string sourceName)

    // Bottom row to initiate calls & chat
    content: BottomToolBarRow {
        id: bottomRow
        visible: true

        leftContent: [
            IconButton {
                id: endCallButton
                width: 120
                active: scene.callAgent != undefined && scene.contactItem.data(AccountsModel.AudioCallCapabilityRole) &&
                        scene.callAgent.callStatus != CallAgent.CallStatusNoCall
                opacity: (scene.callAgent != undefined && scene.callAgent.existingCall) ? 1 : 0
                visible: opacity > 0
                icon: "image://meegotheme/icons/actionbar/call-audio-stop"
                iconDown: icon + "-active"
                hasBackground: true
                bgSourceUp: "image://meegotheme/images/btn_red_up"
                bgSourceDn: "image://meegotheme/images/btn_red_dn"

                onClicked: {
                    scene.callAgent.endCall();
                }
                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
            },

            IconButton {
                id: videoCallButton
                width: 60
                active: scene.contactItem !== undefined &&
                        scene.contactItem.data(AccountsModel.VideoCallWithAudioCapabilityRole) &&
                        scene.callAgent != undefined &&
                        scene.callAgent.callStatus == CallAgent.CallStatusNoCall &&
                        !scene.chatAgent.isConference
                opacity: (scene.callAgent != undefined && scene.callAgent.existingCall) ? 0 : 1
                visible: opacity > 0

                icon: "image://meegotheme/icons/actionbar/turn-video-on"
                iconDown: icon + "-active"
                hasBackground: false
                onClicked: {
                    videoWindow.opacity = 1;
                    scene.callAgent.setOutgoingVideo(cameraWindowSmall ? videoOutgoing : videoIncoming);
                    scene.callAgent.onOrientationChanged(scene.orientation);
                    scene.callAgent.setIncomingVideo(cameraWindowSmall ? videoIncoming : videoOutgoing);
                    scene.callAgent.videoCall();
                }
                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
            },

            IconButton {
                id: videoOnOffButton
                opacity: (scene.callAgent != undefined && scene.callAgent.existingCall) ? 1 : 0
                visible: opacity > 0
                icon: scene.callAgent.videoSent ?
                           "image://meegotheme/icons/actionbar/turn-video-off" :
                           "image://meegotheme/icons/actionbar/turn-video-on"
                iconDown: icon + "-active"
                hasBackground: false
                onClicked: {
                    scene.callAgent.videoSent = !scene.callAgent.videoSent;
                }
                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
            },

            IconButton {
                id: audioCallButton
                width: 60
                active: scene.contactItem !== undefined &&
                        scene.contactItem.data(AccountsModel.AudioCallCapabilityRole) &&
                        scene.callAgent != undefined &&
                        scene.callAgent.callStatus == CallAgent.CallStatusNoCall &&
                        !scene.chatAgent.isConference
                opacity: (scene.callAgent != undefined && scene.callAgent.existingCall) ? 0 : 1
                visible: opacity > 0

                icon: "image://meegotheme/icons/actionbar/call-audio-start"
                iconDown: icon + "-active"
                hasBackground: false
                onClicked: {
                    videoWindow.opacity = 1;
                    scene.callAgent.setOutgoingVideo(cameraWindowSmall ? videoOutgoing : videoIncoming);
                    scene.callAgent.onOrientationChanged(scene.orientation);
                    scene.callAgent.setIncomingVideo(cameraWindowSmall ? videoIncoming : videoOutgoing);
                    scene.callAgent.audioCall();
                }
                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
            },

            IconButton {
                id: volumeOnButton
                width: 60
                active: true
                icon: "image://meegotheme/icons/actionbar/turn-audio-off"
                iconDown: icon + "-active"
                hasBackground: false
                onClicked: {
                    if (volumeLoader.sourceComponent != null ) {
                        volumeLoader.sourceComponent = null;
                    } else {
                        volumeLoader.sourceComponent = volumeControlComponent;
                        volumeLoader.item.parent = volumeOnButton.parent;
                        volumeLoader.item.controllerWidth = volumeOnButton.width;
                        volumeLoader.item.volumeControlX = volumeOnButton.x;
                        volumeLoader.item.volumeControl = volumeControl;
                    }
                }
                opacity: (scene.callAgent != undefined && scene.callAgent.existingCall) ? 1 : 0
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
            },
            
            IconButton {
                id: muteButton
                icon: scene.callAgent.mutedCall ?
                        "image://meegotheme/icons/actionbar/microphone-unmute" :
                        "image://meegotheme/icons/actionbar/microphone-mute"
                iconDown: icon + "-active"
                hasBackground: false
                onClicked: {
                    scene.callAgent.setMuteCall(!scene.callAgent.mutedCall);
                }
                opacity: (scene.callAgent != undefined && scene.callAgent.existingCall) ? 1 : 0
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
            }
            
        ]

        centerContent: [
            Text {
                id: callInfoText
                elide: Text.ElideLeft
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignRight
                text: Utils.getCallStatusText(scene.callAgent)
                color: theme_buttonFontColor

                Timer {
                    //running: scene.callAgent.callStatus == CallAgent.CallStatusTalking
                    running: true
                    interval: 1000
                    repeat: true
                    onTriggered: {
                        callInfoText.text = Utils.getCallStatusText(scene.callAgent);
                    }
                }
                opacity: (scene.callAgent != undefined && scene.callAgent.existingCall) ? 1 : 0
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 500
                    }
                }
            }
        ]
        
        rightContent: [
            IconButton {
                id: insertSmileyButton
                width: 60
                icon: "image://meegotheme/icons/actionbar/insert-emote"
                iconDown: icon + "-active"
                hasBackground: false

                onClicked: {
                    smileyContextMenu.setPosition(insertSmileyButton.mapToItem(toolBar, insertSmileyButton.x, insertSmileyButton.y).x + insertSmileyButton.width / 2,
                                                  insertSmileyButton.y + insertSmileyButton.height + toolBar.y);

                    smileyContextMenu.show();
                }
            },

            IconButton {
                id: sendFileButton
                width: 60
                icon: "image://meegotheme/icons/actionbar/document-attach"
                iconDown: icon + "-active"
                onClicked: {
                    sendFileContextMenu.setPosition(sendFileButton.mapToItem(toolBar, sendFileButton.x, sendFileButton.y).x + sendFileButton.width / 2,
                                                    sendFileButton.y + sendFileButton.height + toolBar.y);
                    sendFileContextMenu.show();
                }
                visible: (scene.contactItem != undefined && scene.contactItem.data(AccountsModel.FileTransferCapabilityRole))
                hasBackground: false
            }
        ]

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

            Component.onCompleted: {
                insertSmileyButton.visible = smileyContextMenu.content != null
            }
        }

        ContextMenu {
            id: sendFileContextMenu

            content: SendFileView {
                id: sendFileView

                onFileSelected: {
                    fileTransferAgent.sendFile(fileName);
                    sendFileContextMenu.hide();
                }
            }

            forceFingerMode: 3

            Component.onCompleted: {
                if (sendFileContextMenu.content == null)
                    sendFileButton.visible = false
            }
        }

        Component {
            id: volumeControlComponent
            Labs.VolumeController {
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
                scene.callAgent.volume = volumeControl.volume / 100;
            }
            onMuteChanged: {
                scene.callAgent.mutedCall = volumeControl.mute;
            }

            Component.onCompleted: {
                volumeControl.volume = scene.callAgent.volume * 100;
                volumeControl.mute = scene.callAgent.mutedCall;
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
