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
        color: (mouseArea.pressed || (contextMenuLoader.item != null && contextMenuLoader.item.visible) ? theme_buttonFontColorActive : theme_commonBoxColor)

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

        Loader {
            id: contextMenuLoader
        }

        Component {
            id: contextMenuComponent
            ContextMenu {
                id: contextMenu

                property string contactId : ""
                property bool chatOpened : false
                property bool textChat : false
                property bool audioCall : false
                property bool videoCall : false
                property bool canBlockContacts : false
                property bool blocked : false
                property bool canReportAbuse : false

                content: Column {
                    id: menuContent

                    height: childrenRect.height
                    width: 200

                    MenuItem {
                        id: textChatItem
                        text: (contextMenu.chatOpened ? Constants.contactReturnToChat :
                                                        Constants.contactOpenChat)
                        visible: contextMenu.textChat
                        onClicked: {
                            window.startConversation(contextMenu.contactId, window);
                            contextMenu.hide();
                        }
                    }

                    MenuItemSeparator { visible: textChatItem.visible }

                    MenuItem {
                        id: callItem
                        text: Constants.contactCall
                        visible: contextMenu.audioCall
                        onClicked: {
                            window.startAudioCall(contextMenu.contactId, window);
                            contextMenu.hide();
                        }
                    }

                    MenuItemSeparator { visible: callItem.visible }

                    MenuItem {
                        id: videoCallItem
                        text: Constants.contactVideoCall
                        visible: contextMenu.videoCall
                        onClicked: {
                            window.startVideoCall(contextMenu.contactId, window);
                            contextMenu.hide();
                        }
                    }

                    MenuItemSeparator { visible: videoCallItem.visible }

                    MenuItem {
                        id: blockItem
                        text: (contextMenu.blocked ? Constants.contactUnblock :
                                               Constants.contactBlock)
                        visible: contextMenu.canBlockContacts
                        onClicked: {
                            if (contextMenu.blocked) {
                            accountsModel.unblockContact(window.currentAccountId, contextMenu.contactId);
                            } else {
                                accountsModel.blockContact(window.currentAccountId, contextMenu.contactId);
                            }
                            contextMenu.hide();
                        }
                    }

                    MenuItemSeparator { visible: blockItem.visible }

                    MenuItem {
                        id: abuseItem
                        text: qsTr("Report abuse")
                        visible: contextMenu.canReportAbuse && !contextMenu.blocked
                        onClicked: {
                            accountsModel.blockContact(window.currentAccountId, contextMenu.contactId, true);
                            contextMenu.hide();
                        }
                    }

                    MenuItemSeparator { visible: abuseItem.visible }

                    MenuItem {
                        id: endChatItem
                        text: Constants.contactEndChat
                        visible: contextMenu.chatOpened
                        onClicked: {
                            accountsModel.endChat(window.currentAccountId, contextMenu.contactId);
                            accountsModel.endCall(window.currentAccountId, contextMenu.contactId);
                            contextMenu.hide();
                        }
                    }

                    MenuItemSeparator { visible: endChatItem.visible }

                    MenuItem {
                        text: Constants.contactDeleteContact
                        onClicked: {
                            accountsModel.removeContact(window.currentAccountId, contextMenu.contactId);
                            contextMenu.hide();
                        }
                    }
                }
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
