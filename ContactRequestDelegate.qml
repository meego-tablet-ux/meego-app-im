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
    width: parent.width
    property int itemHeight: 50
    height: itemHeight
    id: mainArea

    property variant contactItem: model.item

    Image {
        anchors.fill: parent
        source: "image://meegotheme/widgets/apps/chat/friend-request-background"
    }

    Avatar {
        id: avatar
        width: mainArea.height - 4
        height: mainArea.height - 4
        anchors.margins: 2
        anchors.left: mainArea.left
        anchors.top: mainArea.top

        source: model.avatar
    }

    Text {
        id: displayText
        anchors.margins: 10
        anchors.left: avatar.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: acceptButton.left
        // TODO: check width and display alias or username accordingly
        text: qsTr("Request from %1").arg(model.aliasName)
        elide: Text.ElideRight
        font.weight: Font.Bold
        color: theme_fontColorNormal
    }

    Button {
        id: acceptButton
        anchors {
            margins: 10
            right: cancelButton.left
            verticalCenter: parent.verticalCenter
        }

        height: 32

        title: qsTr("Accept")
        color: theme_buttonFontColor
        bgSourceUp: "image://meegotheme/widgets/common/button/button-default"
        bgSourceDn: "image://meegotheme/widgets/common/button/button-default-pressed"

        onClicked: contactItem.setData(AccountsModel.PublishStateRole,
                                       TelepathyTypes.PresenceStateYes)
    }

    Button {
        id: cancelButton
        anchors {
            margins: 10
            right: parent.right
            verticalCenter: parent.verticalCenter
        }

        height: 32

        title: qsTr("Cancel")
        color: theme_buttonFontColor
        bgSourceUp: "image://meegotheme/widgets/common/button/button-negative"
        bgSourceDn: "image://meegotheme/widgets/common/button/button-negative-pressed"

        // TODO: check if we need to remove the contact
        onClicked: contactItem.setData(AccountsModel.PublishStateRole,
                                      TelepathyTypes.PresenceStateNo)
    }
}
