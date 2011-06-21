/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Ux.Components.Common 0.1
import "imconstants.js" as IMConstants

Column {
    anchors.left: parent.left
    anchors.right: parent.right
    height: childrenRect.height

    Item {
        id: spacing1
        width: 10
        height: 10
    }

    Item {
        width: parent.width
        height: childrenRect.height
        Item {
            id: centerItem
            width: childrenRect.width + cancelButton.anchors.margins
            height: childrenRect.height
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                id: updateButton
                anchors {
                    left: parent.left
                    top: parent.top
                }

                text: IMConstants.accountSetupUpdate
                textColor: theme_buttonFontColor
                bgSourceUp: "image://themedimage/widgets/common/button/button-default"
                bgSourceDn: "image://themedimage/widgets/common/button/button-default-pressed"

                onClicked: accountContent.createAccount()
            }

            Button {
                id: cancelButton
                anchors {
                    leftMargin: 10
                    left: updateButton.right
                    top: parent.top
                }

                text: IMConstants.accountSetupCancel
                textColor: theme_buttonFontColor
                bgSourceUp: "image://themedimage/widgets/common/button/button-negative"
                bgSourceDn: "image://themedimage/widgets/common/button/button-negative-pressed"

                // reset the values to the ones previously set
                onClicked: accountContent.prepareAccountEdit()
            }
        }
    }

    Item {
        id: spacing2
        width: 10
        height: 10
    }

    Item {
        width: parent.width
        height: childrenRect.height

        Button {
            id: deleteAccountButton
            anchors.horizontalCenter: parent.horizontalCenter

            text: IMConstants.accountSetupDelete
            textColor: theme_buttonFontColor
            bgSourceUp: "image://themedimage/widgets/common/button/button-negative"
            bgSourceDn: "image://themedimage/widgets/common/button/button-negative-pressed"

            // TODO: maybe it would be good to ask if the user really
            // wants to remove the account?
            onClicked: messageBox.show();
        }
    }

    ModalMessageBox {
        id: messageBox
        parent: container

        height: 300
        width: 400

        text: IMConstants.accountSetupDeleteText

        title: IMConstants.accountSetupDeleteTitle
        showAcceptButton: true
        showCancelButton: true
        fogClickable: false
        fogMaskVisible: false
        anchors.horizontalCenter: parent.left
        anchors.verticalCenter: parent.top

        onAccepted: {
            accountContent.removeAccount();
            hide();
        }
        onRejected: {
            hide();
        }
    }
}
