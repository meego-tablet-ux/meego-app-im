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

ModalDialog {
    id: container
    dialogTitle: qsTr("Incoming Call")
    dialogWidth: 420
    dialogHeight: 480
    leftButtonText: qsTr("Accept")
    bgSourceUpLeft: "image://meegotheme/images/btn_blue_up"
    bgSourceDnLeft: "image://meegotheme/images/btn_blue_dn"
    rightButtonText: qsTr("Decline")
    bgSourceUpRight: "image://meegotheme/images/btn_red_up"
    bgSourceDnRight: "image://meegotheme/images/btn_red_dn"
    contentLoader.sourceComponent: Item {
        anchors.fill: contentLoader

        DialogText {
            id: dialogText
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            height: parent.height - avatarItem.height
            text: qsTr("Incoming call: %1").arg(scene.incomingContactItem.data(AccountsModel.AliasRole));
            horizontalAlignment: Text.AlignHCenter
        }

        Avatar {
            id: avatarItem
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height / 3
            source: scene.incomingContactItem.data(AccountsModel.AvatarRole)
            noAvatarImage: "image://meegotheme/widgets/common/avatar/avatar-default"
        }
    }

    property string accountId: ""
    property string contactId: ""

    Connections {
        target: scene.incomingCallAgent
        onCallStatusChanged: {
            if (scene.incomingCallAgent.callStatus == CallAgent.CallStatusNoCall
             || scene.incomingCallAgent.callStatus == CallAgent.CallStatusHangingUp) {
                dialogLoader.sourceComponent = undefined;
            }
        }
    }

    Component.onCompleted: {
        callSound.playSound();
    }

    onDialogClicked: {
        dialogLoader.sourceComponent = undefined;
        if (button == 1) {
            scene.callAgent = scene.incomingCallAgent
            scene.callAgent.acceptCall();
            scene.acceptCall(accountId, scene.incomingContactItem.data(AccountsModel.IdRole));
            callSound.stopSound();
            dialogLoader.sourceComponent = undefined;
        } else if (button == 2) {
            scene.incomingCallAgent.endCall();
            callSound.stopSound();
            dialogLoader.sourceComponent = undefined;
        }
    }

    IMSound {
        id: callSound
        repeat: true
        soundSource: "/usr/share/sounds/meego/stereo/ring-1.wav"
    }
}

