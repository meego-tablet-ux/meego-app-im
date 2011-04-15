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

Item {
    id: toolBar

    height: scene.callAgent.existingCall ? background.height : 0
    opacity: scene.callAgent.existingCall ? 1 : 0
    visible: opacity > 0

    Behavior on height {
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

    Item {
        id: bottomRow
        anchors.fill: parent

        //property int numButtons : scene.callAgent.existingCall ? 3 : 4
        property int numButtons : 4

        function calculateButtonX (button) {
            var buttonAreaWidth = button.parent.width / button.parent.numButtons;
            var x = buttonAreaWidth * button.numButton;
            x += (buttonAreaWidth - button.width) / 2;
            return x;
        }

        IconButton {
            id: videoOnOffButton
            property int numButton: 0
            x : parent.calculateButtonX(videoOnOffButton)
            opacity: scene.fullscreen ? 0 : 1
            visible: opacity > 0
            anchors.verticalCenter: parent.verticalCenter
            icon: scene.callAgent.videoSent ?
                       "image://meegotheme/icons/actionbar/turn-video-off" :
                       "image://meegotheme/icons/actionbar/turn-video-on"
            iconDown: icon + "-active"
            anchors.margins: 10
            hasBackground: false
            onClicked: {
                scene.callAgent.videoSent = !scene.callAgent.videoSent;
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: 500
                }
            }
        }

        IconButton {
            id: endCallButton
            property int numButton: 0
            x : parent.calculateButtonX(endCallButton)
            opacity: scene.fullscreen ? 1 : 0
            visible: opacity > 0
            active: scene.callAgent.existingCall
            anchors.verticalCenter: parent.verticalCenter
            icon: "image://meegotheme/icons/actionbar/call-audio-stop"
            iconDown: icon + "-active"
            anchors.margins: 10
            hasBackground: false
            onClicked: {
                scene.callAgent.endCall();
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: 500
                }
            }
        }

        IconButton {
            id: volumeOnButton
            property int numButton: 1
            x : parent.calculateButtonX(volumeOnButton)
            active: true
            anchors.verticalCenter: parent.verticalCenter
            icon: "image://meegotheme/icons/actionbar/turn-audio-off"
            iconDown: icon + "-active"
            anchors.margins: 10
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
        }

        IconButton {
            id: muteButton
            property int numButton: 2
            x : parent.calculateButtonX(muteButton)
            anchors.verticalCenter: parent.verticalCenter
            icon: scene.callAgent.mutedCall ?
                       "image://meegotheme/icons/actionbar/microphone-unmute" :
                       "image://meegotheme/icons/actionbar/microphone-mute"
            iconDown: icon + "-active"
            anchors.margins: 10
            hasBackground: false
            onClicked: {
                scene.callAgent.setMuteCall(!scene.callAgent.mutedCall);
            }
        }

        Text {
            id: callInfoText
            property int numButton: 3
            x : parent.calculateButtonX(callInfoText)
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
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
