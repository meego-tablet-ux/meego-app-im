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
import TelepathyQML 0.1
import "constants.js" as Constants

Item {
    id: mainArea

    property int itemHeight: theme_commonBoxHeight
    property variant contactItem
    property string aliasName: ""
    property string avatar: ""

    width: parent.width
    height: itemHeight

    Image {
        anchors.fill: parent
        source: "image://themedimage/widgets/apps/chat/friend-request-background"
    }

    Avatar {
        id: avatarItem

        anchors.left: parent.left
        anchors.top:  parent.top
        anchors.bottom: parent.bottom

        source: avatar
    }

    Column {
        anchors.margins: 10
        anchors.left: avatarItem.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: acceptButton.left

        Text {
            text: Constants.contactRequestFrom
            elide: Text.ElideRight
            color: theme_fontColorInactive
            width: parent.width
            font.pixelSize: theme_fontPixelSizeNormal
        }


        Text {
            id: displayText

            // TODO: check width and display alias or username accordingly
            text: aliasName
            elide: Text.ElideRight
            color: theme_fontColorNormal
            width: parent.width
            font.pixelSize: theme_fontPixelSizeLarge
        }
    }

    Button {
        id: acceptButton
        anchors {
            margins: 10
            right: cancelButton.left
            verticalCenter: parent.verticalCenter
        }

        text: Constants.contactRequestAccept
        textColor: theme_buttonFontColor
        bgSourceUp: "image://themedimage/widgets/common/button/button-default"
        bgSourceDn: "image://themedimage/widgets/common/button/button-default-pressed"

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

        text: Constants.contactRequestCancel
        textColor: theme_buttonFontColor
        bgSourceUp: "image://themedimage/widgets/common/button/button-negative"
        bgSourceDn: "image://themedimage/widgets/common/button/button-negative-pressed"

        // TODO: check if we need to remove the contact
        onClicked: contactItem.setData(AccountsModel.PublishStateRole,
                                      TelepathyTypes.PresenceStateNo)
    }
}
