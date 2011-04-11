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
    id: contactsDelegate

    width: parent.width
    height: childrenRect.height
    property bool tabDiv: false
    property variant currentPage
    property string nameColor: "black"
    property bool active: (model.presenceType == TelepathyTypes.ConnectionPresenceTypeAvailable
                           || model.presenceType == TelepathyTypes.ConnectionPresenceTypeBusy
                           || model.presenceType == TelepathyTypes.ConnectionPresenceTypeAway
                           || model.presenceType == TelepathyTypes.ConnectionPresenceTypeExtendedAway)

    Component.onCompleted: {
        if(model.presenceMessage != "") {
            message.text = model.presenceMessage;
        } else {
            message.text = scene.presenceStatusText(model.presenceType);
        }
    }

    Item {
        id: mainArea
        width: parent.width
        height: 75

        MouseArea {
            ListModel { id: menu}

            anchors.fill: parent
            onClicked: {
                contactsDelegate.ListView.view.currentIndex = index;
                var map = mapToItem(scene, mouseX, mouseY);
                menu.clear();

                // Add items to menu according to contact capabilities
                if(model.textChat && scene.chatAgent.isConference) {
                    menu.append({"modelData":qsTr("Private chat")});
                }
                // check if contact is known
                if(!accountsModel.isContactKnown(scene.currentAccountId, scene.chatAgent.channelPath, model.id)) {
                    menu.append({"modelData":qsTr("Add to contacts")});
                }

                // set the global variable to make sure it is available for the menu
                scene.currentPage = contactsDelegate.currentPage;

                // open menu
                scene.openContextMenu(contextMenu,
                                      contextLoader,
                                      map.x, map.y, model, menu);
            }
        }

        Component {
            id: contextMenu
            ContextMenu {

                ListModel {id: menuIndex}

                menuWidth: 350
                onClose: contextLoader.sourceComponent = undefined
                onTriggered: {
                    // clear the existing menu
                    menuIndex.clear();

                    // Recreate menu according to contact capabilities
                    // this is done because the menu is dynamic, and we can't reuse the
                    // menu previously used because the contactDelegate is not available when
                    // this is called through the loader. Only the selected index and the payload
                    // are available
                    if(payload.textChat && scene.chatAgent.isConference) {
                        menuIndex.append({"modelData":1});
                    }
                    if(!accountsModel.isContactKnown(scene.currentAccountId, scene.chatAgent.channelPath, payload.id)) {
                        menuIndex.append({"modelData":2});
                    }

                    // get the selected action and do whatever was requested
                    var actionIndex = menuIndex.get(index).modelData;
                    if (actionIndex == 1) {
                        // chat
                        var contactid = accountsModel.startPrivateChat(scene.currentAccountId, scene.chatAgent.channelPath, payload.id);
                        scene.currentPage.closeMenu();
                        scene.previousApplicationPage();
                        scene.startConversation(contactid);
                    } else if (actionIndex == 2) {
                        // add contact
                        accountsModel.addContactFromGroupChat(scene.currentAccountId, scene.chatAgent.channelPath, payload.id);
                        scene.currentPage.closeMenu();
                    }

                    // By setting the sourceComponent of the loader to undefined,
                    // then the QML engine will destruct the context menu element
                    // much like doing a c++ delete
                    contextLoader.sourceComponent = undefined;
                }
            }
        }

        Avatar {
            id: avatar
            active: contactsDelegate.active
            source: model.avatar
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            //anchors.top:  parent.top
            anchors.bottom: itemSeparator.top
            anchors.margins: 3
        }

        Column {
            id: nameColumn

            anchors {
                left: avatar.right;
                right: mainArea.right
                verticalCenter: avatar.verticalCenter
                margins: 10
            }
            height: childrenRect.height

            Text {
                id: displayText
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

        Image {
            id: itemSeparator
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: height
            source: "image://meegotheme/widgets/common/menu/menu-item-separator"
            visible: scene.chatAgent.isGroupChatCapable
        }
    }
}
