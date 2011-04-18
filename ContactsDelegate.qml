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
                contactsDelegate.ListView.view.currentIndex = index;
                menu.clear();

                // Add items to menu according to contact capabilities
                if(model.textChat) {
                    if(model.chatOpened) {
                        menu.append({"modelData":qsTr("Return to chat")});
                    } else {
                        menu.append({"modelData":qsTr("Open chat")});
                    }
                }
                if(model.audioCall) {
                    menu.append({"modelData":qsTr("Call")});
                }
                if(model.videoCall) {
                    menu.append({"modelData":qsTr("Video call")});
                }
                // Show depending on block state
                if(model.canBlockContacts) {
                    if(model.blocked) {
                        menu.append({"modelData":qsTr("Unblock")});
                    } else {
                        menu.append({"modelData":qsTr("Block")});
                    }
                }

                //show chat options
                if(model.chatOpened) {
                    menu.append({"modelData":qsTr("End chat")});
                }

                // show the delete option
                menu.append({"modelData":qsTr("Delete contact")});


                var map = mapToItem(scene, mouseX, mouseY);
                contextMenu.setPosition( map.x, map.y);
                actionMenu.model = menu;
                actionMenu.payload = model;
                contextMenu.show();
            }
        }

        ModalContextMenu {
            id: contextMenu
            content: ActionMenu {
                id: actionMenu

                ListModel {id: menuIndex}

                onTriggered: {
                    // clear the existing menu
                    menuIndex.clear();

                    // Recreate menu according to contact capabilities
                    // this is done because the menu is dynamic, and we can't reuse the
                    // menu previously used because the contactDelegate is not available when
                    // this is called through the loader. Only the selected index and the payload
                    // are available
                    if(payload.textChat) {
                        menuIndex.append({"modelData":1});
                    }
                    if(payload.audioCall) {
                        menuIndex.append({"modelData":3});
                    }
                    if(payload.videoCall) {
                        menuIndex.append({"modelData":4});
                    }

                    // Show depending on block state and block capability
                    if(payload.canBlockContacts) {
                        if(payload.blocked) {
                            menuIndex.append({"modelData":5});
                        } else {
                            menuIndex.append({"modelData":6});
                        }
                    }

                    if(payload.chatOpened) {
                        menuIndex.append({"modelData":7});
                    }

                    // Always show the delete option
                    menuIndex.append({"modelData":9});

                    // get the selected action and do whatever was requested
                    var actionIndex = menuIndex.get(index).modelData;
                    if (actionIndex == 1) {
                        // chat
                        scene.startConversation(payload.id, scene);
                    } else if (actionIndex == 3) {
                        // audio call
                        scene.startAudioCall(payload.id, scene);
                    } else if (actionIndex == 4) {
                        // video call
                        scene.startVideoCall(payload.id, scene);
                    } else if (actionIndex == 5) {
                        // unblock contact
                        accountsModel.unblockContact(scene.currentAccountId, payload.id);
                    } else if (actionIndex == 6) {
                        //block contact
                        accountsModel.blockContact(scene.currentAccountId, payload.id);
                    } else if (actionIndex == 7) {
                        // end chat
                        accountsModel.endChat(scene.currentAccountId, payload.id);
                    } else if (actionIndex == 9) {
                        // delete contact
                        console.debug("Going to ")
                        accountsModel.removeContact(scene.currentAccountId, payload.id);
                    }

                    // By setting the sourceComponent of the loader to undefined,
                    // then the QML engine will destruct the context menu element
                    // much like doing a c++ delete
                    contextMenu.hide();
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
