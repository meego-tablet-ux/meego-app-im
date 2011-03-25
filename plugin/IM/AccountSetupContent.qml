/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Labs.Components 0.1

Item {
    id: accountTypeItem
    height: childrenRect.height

    Column {

        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height

        Repeater {
            model: protocolsModel

            Item {
                id: accountSetupItem
                height: childrenRect.height
                width: parent.width

                ContentRow {
                    id: contentRow
                    anchors.left: parent.left
                    anchors.right: parent.right

                    Image {
                        id: serviceIcon
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.margins: 10
                        anchors.left: parent.left
                        source: model.icon
                    }

                    Text {
                        id: accountSetupLabel
                        anchors.margins: 30
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: serviceIcon.right
                        text: model.title
                        font.pixelSize: theme_fontPixelSizeLargest
                        font.bold: true
                    }

                    Image {
                        anchors.margins: 30
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        source: "image://theme/panels/pnl_icn_arrowright"
                    }
                }
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent

                    onClicked: {
                        scene.addApplicationPage(
                                    accountFactory.componentForNewAccount(model.id, scene)
                                    );
                    }
                }
            }
        }
    }
}
