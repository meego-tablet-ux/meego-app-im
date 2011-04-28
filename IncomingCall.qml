/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Components 0.1
import MeeGo.App.IM 0.1
import TelepathyQML 0.1

ModalDialog {
    id: container
    title: qsTr("Incoming Call")
    acceptButtonEnabled: true
    acceptButtonText: qsTr("Accept")
    acceptButtonImage: "image://meegotheme/images/btn_blue_up"
    acceptButtonImagePressed:  "image://meegotheme/images/btn_blue_dn"
    cancelButtonText: qsTr("Decline")
    cancelButtonImage: "image://meegotheme/images/btn_red_up"
    cancelButtonImagePressed: "image://meegotheme/images/btn_red_dn"

    content:
        Row {
            spacing: 10
            height: avatarItem.height * 2
            width: childrenRect.width
            anchors.horizontalCenter: parent.horizontalCenter

            Avatar {
                id: avatarItem
                height: 75
                width: 75
                anchors.verticalCenter: parent.verticalCenter

                source: scene.incomingContactItem.data(AccountsModel.AvatarRole)
                noAvatarImage: "image://meegotheme/widgets/common/avatar/avatar-default"
            }

            Column {
                id: nameColumn
                width: parent.width

                anchors {
                    left: avatar.right
                    verticalCenter: avatarItem.verticalCenter
                    margins: 10
                }
                height: childrenRect.height

                Text {
                    id: dialogText
                    anchors.top: avatarItem.top

                    text: scene.incomingContactItem.data(AccountsModel.AliasRole);
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                }

                Row {
                    spacing: 5
                    height: message.height

                    PresenceIcon {
                        id: presence
                        status: scene.incomingContactItem.data(AccountsModel.PresenceTypeRole);
                        anchors.verticalCenter: message.verticalCenter
                        anchors.topMargin: 5
                    }

                    Text {
                        id: message
                        text: scene.incomingContactItem.data(AccountsModel.PresenceMessageRole);
                        width: parent.width - presence.width - 10
                        color: theme_fontColorNormal
                        font.pixelSize: theme_fontPixelSizeLarge
                        elide: Text.ElideRight
                    }
                }
            }
        }

    property string accountId: ""
    property string contactId: ""

    Connections {
        target: scene.incomingCallAgent
        onCallStatusChanged: {
            if (scene.incomingCallAgent.callStatus == CallAgent.CallStatusNoCall
             || scene.incomingCallAgent.callStatus == CallAgent.CallStatusHangingUp) {
                container.hide();
            }
        }
    }

    Component.onCompleted: {
        callSound.playSound();
    }

    onAccepted: {
        scene.callAgent = scene.incomingCallAgent
        scene.callAgent.acceptCall();
        scene.acceptCall(accountId, scene.incomingContactItem.data(AccountsModel.IdRole));
        callSound.stopSound();
        container.hide();
    }

    onRejected: {
        scene.incomingCallAgent.endCall();
        callSound.stopSound();
        container.hide();
    }

    IMSound {
        id: callSound
        repeat: true
        soundSource: "/usr/share/sounds/meego/stereo/ring-1.wav"
    }
}

