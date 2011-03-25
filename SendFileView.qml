/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.App.IM 0.1
import MeeGo.Labs.Components 0.1
import MeeGo.Media 0.1

Item {
    id: container

    width: itemsColumn.width + itemsColumn.anchors.margins * 2
    height: itemsColumn.height

    signal fileSelected(string fileName);

    function hidePickers() {
        photoPicker.visible = false;
        videoPicker.visible = false;
        musicPicker.visible = false;
        //contactsPicker.visible = false;
    }

    onVisibleChanged: {
        // make sure we hide all pickers
        hidePickers();
    }

    Column {
        id: itemsColumn
        anchors.left: parent.left
        width: childrenRect.width
        anchors.margins: 10

        spacing: 10

        Text {
            color: theme_fontColorNormal
            text: qsTr("Send:")
            font.pixelSize: theme_fontPixelSizeLarge
        }

        Image {
            source: "image://meegotheme/widgets/common/menu/menu-item-separator-header"
            anchors.left: parent.left
            anchors.right: parent.right
        }

        Item {
            anchors.left: parent.left
            width: childrenRect.width

            height: photoRow.height

            Row {
                id: photoRow
                anchors.left: parent.left
                anchors.top: parent.top
                spacing: 10

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: theme_fontColorNormal
                    text: qsTr("Photo")
                    font.pixelSize: theme_fontPixelSizeLargest
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    hidePickers();
                    photoPicker.show()
                }
            }
        }

        Item {
            anchors.left: parent.left
            width: childrenRect.width

            height: videoRow.height

            Row {
                id: videoRow
                anchors.left: parent.left
                anchors.top: parent.top
                spacing: 10

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: theme_fontColorNormal
                    text: qsTr("Video")
                    font.pixelSize: theme_fontPixelSizeLargest
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    hidePickers();
                    videoPicker.show()
                }
            }
        }

        Item {
            anchors.left: parent.left
            width: childrenRect.width
            height: musicRow.height

            Row {
                id: musicRow
                anchors.left: parent.left
                anchors.top: parent.top

                spacing: 10

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: theme_fontColorNormal
                    text: qsTr("Music")
                    font.pixelSize: theme_fontPixelSizeLargest
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    hidePickers();
                    musicPicker.show()
                }
            }
        }

        Item {
            anchors.left: parent.left
            width: childrenRect.width
            height: contactRow.height
            Row {
                id: contactRow
                anchors.left: parent.left
                anchors.top: parent.top

                spacing: 10

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: theme_fontColorNormal
                    text: qsTr("Contact Details")
                    font.pixelSize: theme_fontPixelSizeLargest
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    hidePickers();
                    //contactsPicker.show()
                }
            }
        }
    }

    PhotoPicker {
        id: photoPicker
        parent: scene

        onPhotoSelected: {
            container.fileSelected(uri.replace("file://", ""));
            container.visible = false;
        }
    }

    VideoPicker {
        id: videoPicker
        parent: scene

        onVideoSelected: {
            container.fileSelected(uri.replace("file://", ""));
            container.visible = false;
        }
    }

    MusicPicker {
        id: musicPicker
        parent: scene

        onSongSelected: {
            // TODO: once the music picker gets a uri, use it
            //container.fileSelected(uri.replace("file://", ""));
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
