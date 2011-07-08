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
    property bool active: (model.presenceType == TelepathyTypes.ConnectionPresenceTypeAvailable
                           || model.presenceType == TelepathyTypes.ConnectionPresenceTypeBusy
                           || model.presenceType == TelepathyTypes.ConnectionPresenceTypeAway
                           || model.presenceType == TelepathyTypes.ConnectionPresenceTypeExtendedAway)

    property variant avatarList: model.avatarsList

    Component.onCompleted: {
        if(model.presenceMessage != "") {
            message.text = model.presenceMessage;
        } else {
            message.text = window.presenceStatusText(model.presenceType);
        }
    }

    Connections {
        target: model
        onPresenceTypeChanged: {
            if(model.presenceMessage != "") {
                message.text = model.presenceMessage;
            } else {
                message.text = window.presenceStatusText(model.presenceType);
            }
        }
    }

    ContentRow {
        id: mainArea
        width: parent.width
        active: contactDelegate.active
        color: (mouseArea.pressed || (contextMenuLoader.item != null && contextMenuLoader.contactId == model.id && contextMenuLoader.item.visible) ? theme_buttonFontColorActive : theme_commonBoxColor)

        MouseArea {
            id: mouseArea
            ListModel { id: menu}

            anchors.fill: parent
            onClicked: {
                contactDelegate.ListView.view.currentIndex = index;

                if(contactDelegate.avatarList == undefined) {
                    window.startConversation(model.id);
                } else {
                    window.startGroupConversation(model.id);
                }
            }
            onPressAndHold: {
                if (contextMenuLoader.item == null) {
                    contextMenuLoader.sourceComponent = contextMenuComponent;
                    contextMenuLoader.parent = mainArea;
                }
                var map = mapToItem(window, mouseX, mouseY);
                contextMenuLoader.item.setPosition( map.x, map.y);
                contextMenuLoader.item.contactId = model.id;
                contextMenuLoader.item.chatOpened = model.chatOpened;
                contextMenuLoader.item.textChat = model.textChat;
                contextMenuLoader.item.audioCall = model.audioCall;
                contextMenuLoader.item.videoCall = model.videoCall;
                contextMenuLoader.item.canBlockContacts = model.canBlockContacts;
                contextMenuLoader.item.blocked = model.blocked;
                contextMenuLoader.item.canReportAbuse = model.canReportAbuse;
                contextMenuLoader.item.show();
            }
        }


        Avatar {
            id: avatar
            active: contactDelegate.active
            source: model.avatar
            anchors.left: parent.left
            anchors.top:  parent.top
            anchors.bottom: parent.bottom
            anchors.margins: -1
            visible: (model.avatar != "MULTIPLE")
        }

        AvatarsGroupList {
            id: avatasListView

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

            visible: (model.avatar != "MULTIPLE")
            anchors { 
                left: avatar.right;
                right: chatIcon.left
                verticalCenter: avatar.verticalCenter
                margins: 10
            }
            height: childrenRect.height
            
            Text {
                id: displayText
                // TODO: check width and display alias or username accordingly
                text: model.aliasName
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
                    status: model.presenceType
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

            messageCount: model.pendingMessages
            missedAudioCalls: model.missedAudioCalls
            missedVideoCalls: model.missedVideoCalls
            openChat: model.chatOpened

            anchors.margins: 10
            anchors.right: mainArea.right
            anchors.top: mainArea.top
            anchors.bottom: mainArea.bottom

            visible: (openChat || (messageCount + missedAudioCalls + missedVideoCalls) > 0)
        }
    }
}
