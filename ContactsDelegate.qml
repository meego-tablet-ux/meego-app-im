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

Item {
    id: contactsDelegate

    width: parent.width
    height: childrenRect.height
    property bool tabDiv: false
    property string nameColor: "black"
    property bool active: (model.presenceType == TelepathyTypes.ConnectionPresenceTypeAvailable
                           || model.presenceType == TelepathyTypes.ConnectionPresenceTypeBusy
                           || model.presenceType == TelepathyTypes.ConnectionPresenceTypeAway
                           || model.presenceType == TelepathyTypes.ConnectionPresenceTypeExtendedAway)

    property variant avatarList: model.avatarsList

    Component.onCompleted: {
        if(model.presenceMessage != "") {
            message.text = model.presenceMessage;
        } else {
            message.text = scene.presenceStatusText(model.presenceType);
        }
    }

    Connections {
        target: model
        onPresenceTypeChanged: {
            if(model.presenceMessage != "") {
                message.text = model.presenceMessage;
            } else {
                message.text = scene.presenceStatusText(model.presenceType);
            }
        }
    }

    ContentRow {
        id: mainArea
        width: parent.width
        active: contactsDelegate.active

        MouseArea {
            ListModel { id: menu}

            anchors.fill: parent
            onClicked: {
                contactsDelegate.ListView.view.currentIndex = index;

                if(contactsDelegate.avatarList == undefined) {
                    scene.startConversation(model.id);
                } else {
                    scene.startGroupConversation(model.id);
                }
            }
            onPressAndHold: {
                var map = mapToItem(scene, mouseX, mouseY);
                contextMenu.setPosition( map.x, map.y);
                menuContent.model = model;
                contextMenu.show();
            }
        }

        ModalContextMenu {
            id: contextMenu
            content: Column {
                id: menuContent

                property variant model
                height: childrenRect.height

                MenuItem {
                    id: textChatItem
                    text: (model.chatOpened ? qsTr("Return to chat") :
                                              qsTr("Open chat"))
                    visible: model.textChat
                    onClicked: {
                        scene.startConversation(model.id, scene);
                        contextMenu.hide();
                    }
                }

                MenuItemSeparator { visible: textChatItem.visible }

                MenuItem {
                    id: callItem
                    text: qsTr("Call")
                    visible: model.audioCall
                    onClicked: {
                        scene.startAudioCall(model.id, scene);
                        contextMenu.hide();
                    }
                }

                MenuItemSeparator { visible: callItem.visible }

                MenuItem {
                    id: videoCallItem
                    text: qsTr("Video call")
                    visible: model.videoCall
                    onClicked: {
                        scene.startVideoCall(model.id, scene);
                        contextMenu.hide();
                    }
                }

                MenuItemSeparator { visible: videoCallItem.visible }

                MenuItem {
                    id: blockItem
                    text: (model.blocked ? qsTr("Unblock") :
                                           qsTr("Block"))
                    visible: model.canBlockContacts
                    onClicked: {
                        if (model.blocked) {
                        accountsModel.unblockContact(scene.currentAccountId, model.id);
                        } else {
                            accountsModel.blockContact(scene.currentAccountId, model.id);
                        }
                        contextMenu.hide();
                    }
                }

                MenuItemSeparator { visible: blockItem.visible }

                MenuItem {
                    id: endChatItem
                    text: qsTr("End chat")
                    visible: model.chatOpened
                    onClicked: {
                        accountsModel.endChat(scene.currentAccountId, model.id);
                        contextMenu.hide();
                    }
                }

                MenuItemSeparator { visible: endChatItem.visible }

                MenuItem {
                    text: qsTr("Delete contact")
                    onClicked: {
                        accountsModel.removeContact(scene.currentAccountId, model.id);
                        contextMenu.hide();
                    }
                }
            }
        }

        Avatar {
            id: avatar
            active: contactsDelegate.active
            source: model.avatar
            anchors.left: parent.left
            anchors.top:  parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 3
            visible: (model.avatar != "MULTIPLE")
        }

        AvatarsGroupList {
            id: avatasListView

            anchors {
                left: mainArea.left;
                right: chatIcon.left
                top: mainArea.top
                bottom: mainArea.bottom
                margins: 3
            }

            clip: true

            avatarList: contactsDelegate.avatarList
            visible: (contactsDelegate.avatarList != undefined)
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
                color: nameColor
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
                    color: theme_fontColorNormal
                    font.pixelSize: theme_fontPixelSizeLarge
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

            visible: ((model.chatOpened)
                      || model.pendingMessages > 0
                      || model.missedAudioCalls > 0
                      || model.missedVideoCalls > 0)
        }
    }
}
