/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.App.IM 0.1
import TelepathyQML 0.1

Item {
    id: container

    property variant currentPage
    property variant contactsModel

    width: 590
    height: meColumn.height

    Column {
        id: meColumn

        height: childrenRect.height

        anchors {
            right: parent.right
            left: parent.left
        }

        Repeater {
            id: contactsView

            model: scene.chatAgent.contactsModel()
            delegate: MessageContactDelegate {
                anchors.left: parent.left
                anchors.right: parent.right
                currentPage: container.currentPage
            }
            clip: true
        }

        MenuItem {
            id: addContactItem
            visible: scene.chatAgent.isGroupChatCapable

            text: qsTr("Add contacts to chat")
            onClicked: {
                scene.pickContacts(messageScreenPage);
                currentPage.closeMenu();
            }
        }

        Image {
            anchors.left: parent.left
            anchors.right: parent.right
            source: "image://meegotheme/widgets/common/menu/menu-item-separator"
        }

        MenuItem {
            id: meClearHistory
            text: qsTr("Clear chat history")
            onClicked: {
                if(scene.chatAgent.isConference) {
                    accountsModel.clearRoomHistory(scene.currentAccountId, scene.chatAgent.channelPath);
                } else {
                    accountsModel.clearContactHistory(scene.currentAccountId, scene.currentContactId);
                }
                currentPage.closeMenu();
            }
        }

        Image {
            anchors.left: parent.left
            anchors.right: parent.right
            source: "image://meegotheme/widgets/common/menu/menu-item-separator"
        }

        MenuItem {
            id: meEndChat
            text: qsTr("End chat")
            onClicked: {
                currentPage.closeConversation();
                currentPage.closeMenu();
            }
        }
    }
}
