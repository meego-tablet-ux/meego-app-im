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
    id: contactDelegate

    width: parent.width
    height: childrenRect.height
    property bool tabDiv: false
    property bool active: (presenceType == TelepathyTypes.ConnectionPresenceTypeAvailable
                           || presenceType == TelepathyTypes.ConnectionPresenceTypeBusy
                           || presenceType == TelepathyTypes.ConnectionPresenceTypeAway
                           || presenceType == TelepathyTypes.ConnectionPresenceTypeExtendedAway)

    property variant avatarList: []
    property string contactId: ""
    property string aliasName: ""
    property string presenceMessage: ""
    property int presenceType: 0
    property string avatar: ""
    property bool chatOpened : false
    property bool textChat : false
    property bool audioCall : false
    property bool videoCall : false
    property bool canBlockContacts : false
    property bool blocked : false
    property bool canReportAbuse : false
    property int messageCount: 0
    property int missedAudioCalls: 0
    property int missedVideoCalls: 0
    property bool openChat: false

    Component.onCompleted: {
        if(presenceMessage != "") {
            message.text = presenceMessage;
        } else {
            message.text = window.presenceStatusText(presenceType);
        }
    }

    Connections {
        target: model
        onPresenceTypeChanged: {
            if(presenceMessage != "") {
                message.text = presenceMessage;
            } else {
                message.text = window.presenceStatusText(presenceType);
            }
        }
    }

    ContentRow {
        id: mainArea
        width: parent.width
        active: contactDelegate.active
        color: (mouseArea.pressed || (contextMenuLoader.item != null && contextMenuLoader.item.contactId == contactId && contextMenuLoader.item.visible) ? theme_buttonFontColorActive : theme_commonBoxColor)

        MouseArea {
            id: mouseArea
            ListModel { id: menu}

            anchors.fill: parent
            onClicked: {
                if(avatarList == undefined) {
                    window.startConversation(contactId);
                } else {
                    window.startGroupConversation(contactId);
                }
            }
            onPressAndHold: {
                if (contextMenuLoader.item == null) {
                    contextMenuLoader.sourceComponent = contextMenuComponent;
                }
                var map = mapToItem(window, mouseX, mouseY);
                contextMenuLoader.item.setPosition( map.x, map.y);
                contextMenuLoader.item.contactId = contactId;
                contextMenuLoader.item.chatOpened = chatOpened;
                contextMenuLoader.item.textChat = textChat;
                contextMenuLoader.item.audioCall = audioCall;
                contextMenuLoader.item.videoCall = videoCall;
                contextMenuLoader.item.canBlockContacts = canBlockContacts;
                contextMenuLoader.item.blocked = blocked;
                contextMenuLoader.item.canReportAbuse = canReportAbuse;
                contextMenuLoader.item.show();
            }
        }


        Avatar {
            id: avatarItem
            active: contactDelegate.active
            source: avatar
            anchors.left: parent.left
            anchors.top:  parent.top
            anchors.bottom: parent.bottom
            anchors.margins: -1
            visible: (avatar != "MULTIPLE")
        }

        AvatarsGroupList {
            id: avatarListView

            anchors {
                left: mainArea.left;
                right: chatIcon.left
                top: mainArea.top
                bottom: mainArea.bottom
                margins: 0
            }

            clip: true

            avatarList: contactDelegate.avatarList
            visible: (contactDelegate.avatarList != undefined)
        }

        Column {
            id: nameColumn

            visible: (avatar != "MULTIPLE")
            anchors { 
                left: avatarItem.right;
                right: chatIcon.left
                verticalCenter: avatarItem.verticalCenter
                margins: 10
            }
            height: childrenRect.height
            
            Text {
                id: displayText
                // TODO: check width and display alias or username accordingly
                text: aliasName
                width: parent.width
                elide: Text.ElideRight
                color: theme_fontColorNormal
                font.pixelSize: theme_fontPixelSizeLarge
            }
            
            Row {
                spacing: 5
                width: parent.width
                height: message.height

                PresenceIcon {
                    id: presence
                    status: presenceType
                    anchors.verticalCenter: message.verticalCenter
                    anchors.topMargin: 5
                }
            
                Text {
                    id: message
                    text: ""
                    width: parent.width - presence.width - 10
                    color: theme_fontColorInactive
                    font.pixelSize: theme_fontPixelSizeNormal
                    elide: Text.ElideRight
                }
            }
        }

        CallCountIcon {
            id: chatIcon

            messageCount: pendingMessages
            missedAudioCalls: missedAudioCalls
            missedVideoCalls: missedVideoCalls
            openChat: chatOpened

            anchors.margins: 10
            anchors.right: mainArea.right
            anchors.top: mainArea.top
            anchors.bottom: mainArea.bottom

            visible: (openChat || (messageCount + missedAudioCalls + missedVideoCalls) > 0)
        }
    }
}
