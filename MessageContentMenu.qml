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
    id: container

    property variant currentPage
    property variant contactsModel

    width: 590
    height: meColumn.height

    Column {
        id: meColumn

        spacing: theme_contextMenuFontPixelSize / 2
        anchors {
            right: parent.right
            left: parent.left
        }

        ListView {
            id: contactsView

            height: 75 * model.rowCount

            anchors {
                left: parent.left
                right: parent.right
            }

            model: scene.chatAgent.contactsModel()
            delegate: MessageContactsDelegate {
                currentPage: container.currentPage
                }
            clip: true
        }

        Item {
            id: addContactItem
            height: addContactText.height
            width: parent.width

            visible: scene.chatAgent.isGroupChatCapable

            Text {
                id: addContactText
                anchors.top: parent.top
                anchors.leftMargin: 10
                anchors.left: parent.left
                anchors.right: parent.right
                verticalAlignment: Text.AlignVCenter
                text: qsTr("Add contacts to chat")
                font.pixelSize: theme_contextMenuFontPixelSize
                color: theme_fontColorNormal
            }

            MouseArea {
                id: addContactMouseArea
                anchors.fill: parent

                onClicked: {
                    scene.pickContacts(messageScreenPage);
                    currentPage.closeMenu();
                }
            }
        }

        Image {
            anchors.left: parent.left
            anchors.right: parent.right
            source: "image://meegotheme/widgets/common/menu/menu-item-separator"
        }

        Item {
            id: meEndChat
            height: meEndChatText.height
            width: parent.width

            Text {
                id: meEndChatText
                anchors.top: parent.top
                anchors.leftMargin: 10
                anchors.left: parent.left
                anchors.right: parent.right
                verticalAlignment: Text.AlignVCenter
                text: qsTr("End chat")
                font.pixelSize: theme_contextMenuFontPixelSize
                color: theme_fontColorNormal
            }

            MouseArea {
                id: endChatMouseArea
                anchors.fill: parent

                onClicked: {
                    currentPage.closeConversation();
                    currentPage.closeMenu();
                }
            }
        }
    }
}
