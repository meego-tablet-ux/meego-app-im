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
    width: 420
    height:  480
    acceptButtonEnabled: true
    acceptButtonText: qsTr("Accept")
    acceptButtonImage: "image://meegotheme/images/btn_blue_up"
    acceptButtonImagePressed:  "image://meegotheme/images/btn_blue_dn"
    cancelButtonText: qsTr("Decline")
    cancelButtonImage: "image://meegotheme/images/btn_red_up"
    cancelButtonImagePressed: "image://meegotheme/images/btn_red_dn"
    content: Item {

        height: 400
        width:  420

        Column {
            spacing: 5
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            Text {
                id: dialogText
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 10

                text: scene.incomingContactItem.data(AccountsModel.AliasRole);
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
            }

            Item {

                height: 300
                width: 300

                anchors.horizontalCenter: parent.horizontalCenter

                Avatar {
                    id: avatarItem

                    source: scene.incomingContactItem.data(AccountsModel.AvatarRole)
                    noAvatarImage: "image://meegotheme/widgets/common/avatar/avatar-default"
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

