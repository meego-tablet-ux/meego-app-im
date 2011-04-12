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


ApplicationPage {
    id: contactPickerPage

    anchors.fill: parent
    // showSearch: true

    onSearch: {
        contactsModel.filterByString(needle);
    }

    signal itemSelected(string parentId, string itemId);
    signal itemDeselected(string parentId, string itemId);

    onItemSelected: {
        var exists = false;
        for(var i = 0; i < selectedItems.count; ++i) {
            if(selectedItems.get(i).itemId == itemId) {
                exists = true;
            }
        }
        if(exists == false) {
            selectedItems.append({"itemId":itemId});
        }
    }

    onItemDeselected: {
        for(var i = 0; i < selectedItems.count; ++i) {
            if(selectedItems.get(i).itemId == itemId) {
                selectedItems.remove(i);
            }
        }
    }

    Component.onCompleted: {
        scene.title = qsTr("Add contacts to chat");
        var contactsList;
        if(scene.currentContactId == "") {
            contactsList = accountsModel.channelContacts(scene.currentAccountId, scene.chatAgent.channelPath);
        } else {
            contactsList = accountsModel.channelContacts(scene.currentAccountId, scene.currentContactId);
        }
        contactsModel.skipContacts(contactsList);
        contactsModel.setContactsOnly(true);
    }

    Component.onDestruction: {
        contactsModel.setContactsOnly(false);
        contactsModel.clearSkippedContacts();
    }

    ListModel {
        id: selectedItems
    }

    function isItemSelected(contactId)
    {
        for(var i = 0; i < selectedItems.count; ++i) {
            if(selectedItems.get(i).itemId == contactId) {
                return true;
            }
        }
        return false;
    }

    Item {
        id: pageContent
        parent: contactPickerPage.content
        anchors.fill: parent

        Image {
            id: background
            anchors.fill: parent
            source: "image://meegotheme/widgets/common/picker/picker-background"
        }

        ListView {
            id: listView

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                bottom: buttonRowImage.top
            }

            model: contactsModel
            delegate: ContactPickerDelegate {}
            clip: true
        }

        Image {
            id: buttonRowImage

            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            height: 100
            source: "image://meegotheme/widgets/common/action-bar/action-bar-background"

            Row {
                id: buttonRow

                height:  acceptButton.height
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                spacing:  10

                Button {
                    id: acceptButton
                    anchors {
                        margins: 10
                        verticalCenter: parent.verticalCenter
                    }

                    height: 32

                    title: qsTr("Add")
                    color: theme_buttonFontColor
                    bgSourceUp: "image://meegotheme/widgets/common/button/button-positive"
                    bgSourceDn: "image://meegotheme/widgets/common/button/button-positive-pressed"

                    onClicked: {
                        var contactsList = ""; // i18n ok
                        for(var i = 0; i < selectedItems.count; ++i) {
                            contactsList = contactsList + " " + selectedItems.get(i).itemId; // i18n ok
                        }

                        if(scene.currentContactId == "") {
                            accountsModel.addContactsToChat(scene.currentAccountId, scene.chatAgent.channelPath, contactsList);
                        } else {
                            accountsModel.addContactsToChat(scene.currentAccountId, scene.currentContactId, contactsList);
                        }

                        scene.previousApplicationPage();

                        // if cannot add contacts, go back to the contacts screen
                        // otherwise reloading the messageScreenPage has problems
                        // the onDestroyed and onCompleted methods don't get called in order
                        // therefore it's safest to go back all the way to the contacts list
                        // then reload the messageScreenPage when the signal is received
                        if(!scene.chatAgent.canAddContacts) {
                            scene.previousApplicationPage();
                        }
                    }
                }

                Button {
                    id: cancelButton
                    anchors {
                        margins: 10
                        verticalCenter: parent.verticalCenter
                    }

                    height: 32

                    title: qsTr("Cancel")
                    color: theme_buttonFontColor
                    bgSourceUp: "image://meegotheme/widgets/common/button/button-negative"
                    bgSourceDn: "image://meegotheme/widgets/common/button/button-negative-pressed"

                    // TODO: check if we need to remove the contact
                    onClicked: {
                        scene.previousApplicationPage();
                    }
                }
            }
        }
    }
}
