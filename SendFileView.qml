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

    // FIXME remove after full migration to MeegGo.Components
    property variant window : scene

    width: itemsColumn.width
    height: itemsColumn.height

    signal fileSelected(string fileName);

    function hidePickers() {
        photoPicker.hide();
        videoPicker.hide();
        musicPicker.hide();
        //contactsPicker.hide();
    }

    onVisibleChanged: {
        // make sure we hide all pickers
        hidePickers();
    }

    Column {
        id: itemsColumn
        anchors.left: parent.left
        height: childrenRect.height

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
                photoPicker.show();
            }
        }

        MenuItemSeparator { }

        MenuItem {
            id: videoItem
            text: qsTr("Video")

            onClicked: {
                videoPicker.show()
            }
        }

        MenuItemSeparator { }

        MenuItem {
            id: musicItem
            text: qsTr("Music")

            onClicked: {
                musicPicker.show()
            }
        }

        MenuItemSeparator { }

        MenuItem {
            id: contactItem
            text: qsTr("Contact Details")

            onClicked: {
                //contactsPicker.show()
            }
        }
    }

    PhotoPicker {
        id: photoPicker

        onPhotoSelected: {
            container.fileSelected(uri.replace("file://", ""));
            container.visible = false;
        }
    }

    VideoPicker {
        id: videoPicker

        onVideoSelected: {
            container.fileSelected(uri.replace("file://", ""));
            container.visible = false;
        }
    }

    MusicPicker {
        id: musicPicker
        selectSongs: true

        onSongSelected: {
            container.fileSelected(uri.replace("file://", ""));
            container.visible = false;
        }
    }

    /*
    TODO: the contacts picker is broken, re-enable when it is fixed
    ContactsPicker {
        id: contactsPicker
        parent: scene

        onContactSelected: {
            // TODO: check how to send that
            container.visible = false;
        }
    }*/
}
