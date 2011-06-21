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
import "constants.js" as Constants

Item {
    id: container

    property variant currentPage
    property variant contactsModel

    width: 400
    height: meColumn.height

    onVisibleChanged: {
        if(visible) {
            contactsView.model = window.chatAgent.contactsModel();
        }
    }

    Column {
        id: meColumn

        height: childrenRect.height

        anchors {
            right: parent.right
            left: parent.left
        }

        Repeater {
            id: contactsView

            delegate: MessageContactDelegate {
                parent: meColumn
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                currentPage: container.currentPage
            }
            clip: true
        }

        MenuItem {
            id: addContactItem
            visible: window.chatAgent != undefined ? window.chatAgent.isGroupChatCapable : false

            text: Constants.messageAddContacts
            onClicked: {
                currentPage.hideActionMenu();
                window.pickContacts();
            }
        }

        Image {
            anchors.left: parent.left
            anchors.right: parent.right
            source: "image://themedimage/widgets/common/menu/menu-item-separator"
        }

        MenuItem {
            id: meClearHistory
            text: Constants.messageClearChatHistory
            onClicked: {
                if(window.chatAgent.isConference) {
                    accountsModel.clearRoomHistory(window.currentAccountId, window.chatAgent.channelPath);
                } else {
                    accountsModel.clearContactHistory(window.currentAccountId, window.currentContactId);
                }
                currentPage.hideActionMenu();
            }
        }

        Image {
            anchors.left: parent.left
            anchors.right: parent.right
            source: "image://themedimage/widgets/common/menu/menu-item-separator"
        }

        MenuItem {
            id: meEndChat
            text: Constants.messageEndChat
            onClicked: {
                currentPage.closeConversation();
                currentPage.hideActionMenu();
            }
        }
    }
}
