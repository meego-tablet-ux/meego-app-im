/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.App.IM 0.1
import MeeGo.Components 0.1

Item {
    id: container

    width: itemsColumn.width
    height: itemsColumn.height

    signal fileSelected(string fileName);
    signal cancelled

    function hidePickers() {
        photoPicker.hide();
        videoPicker.hide();
        musicPicker.hide();
        contactsPicker.visible = false;
    }

    onVisibleChanged: {
        // make sure we hide all pickers
        hidePickers();
    }

    Column {
        id: itemsColumn
        anchors.left: parent.left
        height: childrenRect.height
        width: 200

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: headerText.height + 10

            Text {
                id: headerText
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 10
                anchors.left: parent.left
                color: theme_fontColorNormal
                text: qsTr("Send:")
                font.pixelSize: theme_fontPixelSizeLarge
            }
        }

        MenuItemSeparator {
            header: true
        }

        MenuItem {
            id: photoItem
            text: qsTr("Photo")

            onClicked: {
                photoPicker.parent = container;
                photoPicker.show();
            }
        }

        MenuItemSeparator { }

        MenuItem {
            id: videoItem
            text: qsTr("Video")

            onClicked: {
                videoPicker.parent = container;
                videoPicker.show()
            }
        }

        MenuItemSeparator { }

        MenuItem {
            id: musicItem
            text: qsTr("Music")

            onClicked: {
                musicPicker.parent = container;
                musicPicker.show()
            }
        }

        MenuItemSeparator { }

        MenuItem {
            id: contactItem
            text: qsTr("Contact Details")

            onClicked: {
                contactsPicker.show();
            }
        }
    }

    Connections {
        target: photoPicker
        onPhotoSelected: {
            container.fileSelected(uri.replace("file://", ""));
        }
    }

    Connections {
        target: videoPicker
        onVideoSelected: {
            container.fileSelected(uri.replace("file://", ""));
        }
    }

    Connections {
        target: musicPicker
        onSongSelected: {
            container.fileSelected(uri.replace("file://", ""));
        }
    }

    Connections {
        target: contactsPicker
        onContactSelected: {
            var filename = contact.name.firstName + "_" + contact.name.lastName;
            filename = filename.replace(" ", "_");
            filename = "/tmp/vcard_" +filename + ".vcf";
            peopleModel.exportContact(contact.guid.guid, filename);
            container.fileSelected(filename);
        }
        onCancelled: {
            container.cancelled();
        }
    }
}
