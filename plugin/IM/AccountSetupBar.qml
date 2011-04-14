/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Components 0.1

Column {
    anchors.left: parent.left
    anchors.right: parent.right
    height: childrenRect.height

    Item {
        width: parent.width
        height: 64
        Item {
            id: centerItem
            width: childrenRect.width
            height: parent.height
            anchors.centerIn: parent

            Button {
                id: updateButton
                anchors {
                    margins: 10
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }

                height: 40

                text: qsTr("Update")
                textColor: theme_buttonFontColor
                bgSourceUp: "image://meegotheme/widgets/common/button/button-default"
                bgSourceDn: "image://meegotheme/widgets/common/button/button-default-pressed"

                onClicked: accountContent.createAccount()
            }

            Button {
                id: cancelButton
                anchors {
                    margins: 10
                    left: updateButton.right
                    verticalCenter: parent.verticalCenter
                }

                height: 40

                text: qsTr("Cancel")
                textColor: theme_buttonFontColor
                bgSourceUp: "image://meegotheme/widgets/common/button/button-negative"
                bgSourceDn: "image://meegotheme/widgets/common/button/button-negative-pressed"

                // reset the values to the ones previously set
                onClicked: accountContent.prepareAccountEdit()
            }
        }
    }
    Item {
        width: parent.width
        height: 64
        Button {
            id: deleteAccountButton
            anchors.centerIn: parent

            height: 40

            text: qsTr("Delete account")
            textColor: theme_buttonFontColor
            bgSourceUp: "image://meegotheme/widgets/common/button/button-negative"
            bgSourceDn: "image://meegotheme/widgets/common/button/button-negative-pressed"

            // TODO: maybe it would be good to ask if the user really
            // wants to remove the account?
            onClicked: accountContent.removeAccount();
        }
    }
}
