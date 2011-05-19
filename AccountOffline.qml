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
import TelepathyQML 0.1

Item {
    id: container

    height: (visible? panel.height : 0)

    InfoPanel {
        id: panel

        width: parent.width

        Text {
            id: accountOfflineText

            text: qsTr("Account is offline");
            anchors {
                horizontalCenter: parent.horizontalCenter
                verticalCenter: parent.verticalCenter
            }
            color: theme_fontColorHighlight
            font.pixelSize: theme_fontPixelSizeLarge
            font.weight: Font.Bold
        }

        Item {
            id: spinner
            anchors.verticalCenter: accountOfflineText.verticalCenter
            anchors.left: accountOfflineText.right
            anchors.leftMargin: 15
            visible: (container.visible && showSpinner)

            property bool showSpinner: false
            Spinner {
                id: loadingIcon

                width: theme_fontPixelSizeLarge
                height: theme_fontPixelSizeLarge
                spinning: spinner.visible

                onSpinningChanged: {
                    if(spinner.visible && !spinning) {
                        spinning = true;
                    }
                }
            }
        }
    }

    function setInfoMessage(status)
    {
        if (accountStatus == TelepathyTypes.ConnectionStatusDisconnected) {
            accountOfflineText.text = qsTr("Account is offline");
            spinner.showSpinner = false;
        } else if (accountStatus == TelepathyTypes.ConnectionStatusConnecting) {
            accountOfflineText.text = qsTr("Account is connecting");
            spinner.showSpinner = true;
        } else {
            accountOfflineText.text = "";
            spinner.showSpinner = false;
        }
    }


}
