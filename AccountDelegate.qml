/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Ux.Components.Common 0.1
import MeeGo.App.IM 0.1
import TelepathyQML 0.1
import "constants.js" as Constants

Item {
    id: mainArea

    property int accountStatus: model.connectionStatus

    height: childrenRect.height

    Connections {
        target: accountsModel

        onNewAccountItem: {
            if (model.id == accountId) {
                contentRow.forceActiveFocus();
            }
        }
    }

    ContentRow {
        id: contentRow
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: (mouseArea.pressed || (contextMenuLoader.item != null && contextMenuLoader.item.model.id == model.id && contextMenuLoader.item.visible) ? theme_buttonFontColorActive : theme_commonBoxColor)

        Image {
            id: serviceIcon
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            source: accountFactory.accountIcon(model.icon, model.connectionStatus)
            opacity: loadingIcon.visible ? 0 : 1
            smooth: true

        }

        Spinner {
            id: loadingIcon
            anchors.centerIn: serviceIcon
            visible: model.connectionStatus == TelepathyTypes.ConnectionStatusConnecting

            spinning: visible

            onSpinningChanged: {
                if(loadingIcon.visible && !spinning) {
                    spinning = true;
                }
            }
        }

        Text {
            id: accountTypeName
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: serviceIcon.right
            anchors.right: parent.right
            anchors.margins: 10
            text: model.displayName

            elide: Text.ElideRight
            font.weight: Font.Bold
            color: theme_fontColorNormal
            font.pixelSize: theme_fontPixelSizeLargest
        }

        Connections {
            target: confirmationDialogItem
            onAccepted: {
                if (confirmationDialogItem.instanceReason != "account-delegate-single-instance") {
                    return;
                }

                // filter out signals that are not for this account
                if (confirmationDialogItem.accountId != model.id) {
                    return;
                }

                // if the dialog was accepted we should disconnect all other accounts
                // of the same type
                accountFactory.disconnectOtherAccounts(model.icon, model.id);

                // and set the account online
                model.item.setRequestedPresence(TelepathyTypes.ConnectionPresenceTypeAvailable,
                                                "available", // i18n ok
                                                model.item.data(AccountsModel.CurrentPresenceStatusMessageRole));
            }

            // no need to do anything if the dialog is rejected
            // onRejected:
        }

        CallCountIcon {
            id: chatIcon

            messageCount: model.pendingMessages
            missedAudioCalls: model.missedAudioCalls
            missedVideoCalls: model.missedVideoCalls
            openChat: model.chatOpened

            anchors.margins: 5
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            visible: (model.chatOpened
                      || model.missedAudioCalls > 0
                      || model.missedVideoCalls > 0)
        }

        ListModel { id: menu}

        MouseArea {
            id: mouseArea

            anchors.fill: parent
            onClicked: {
                window.currentAccountId = model.id;
                window.showContactsScreen();
            }

            onPressAndHold: {
                if (contextMenuLoader.item == null) {
                    contextMenuLoader.sourceComponent = contextMenuComponent;
                }

                menu.clear();

                if(model.connectionStatus != TelepathyTypes.ConnectionStatusDisconnected) {
                    menu.append({"modelData":Constants.accountLogout});
                } else {
                    menu.append({"modelData":Constants.accountLogin.arg(telepathyManager.accountServiceName(model.icon))})
                }
                menu.append({"modelData":Constants.accountSettings});

                var map = mapToItem(window, mouseX, mouseY);
                contextMenuLoader.item.setPosition( map.x, map.y);
                contextMenuLoader.item.model = menu;
                contextMenuLoader.item.payload = model.item;
                contextMenuLoader.item.show();
            }
        }
    }
}
