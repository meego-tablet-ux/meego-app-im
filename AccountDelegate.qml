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


Item {
    id: mainArea

    property int accountStatus: model.statusType

    height: childrenRect.height

    ContentRow {
        id: contentRow
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

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
            target: dialogLoader.item
            onAccepted: {
                if (dialogLoader.item.instanceReason != "account-delegate-single-instance") {
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

        Component {
            id: contextComponent
            ContextMenu {
                menuWidth: 350
                onClose: contextLoader.sourceComponent = undefined
                ApplicationsModel {
                    id: appModel
                }
                onTriggered: {
                    if (index == 0)
                    {
                        if(payload.data(AccountsModel.ConnectionStatusRole) == TelepathyTypes.ConnectionStatusConnected) {
                            payload.setRequestedPresence(TelepathyTypes.ConnectionPresenceTypeOffline,
                                                "offline", // i18n ok
                                                payload.data(AccountsModel.ConnectionStatusRole));
                        } else {
                            var icon = payload.data(AccountsModel.IconRole);
                            var id = payload.data(AccountsModel.IdRole);
                            var serviceName = protocolsModel.titleForId(icon);

                            // if the protocol only allows to have one account connected at a time,
                            // ask the user if he really wants to do that
                            if (protocolsModel.isSingleInstance(icon) &&
                                accountFactory.otherAccountsOnline(icon, id)) {
                                // show the dialog asking the user if he really wants to connect the account
                                showModalDialog(confirmationDialogContent);
                                dialogLoader.item.dialogTitle = qsTr("Multiple accounts connected");
                                dialogLoader.item.mainText = qsTr("Do you really want to connect this account?");
                                dialogLoader.item.subText = qsTr("By doing this all other %1 accounts will be disconnected.").arg(serviceName);
                                dialogLoader.item.instanceReason = "account-delegate-single-instance"; // i18n ok
                            } else {
                                payload.setRequestedPresence(TelepathyTypes.ConnectionPresenceTypeAvailable,
                                                             "available", // i18n ok
                                                             payload.data(AccountsModel.CurrentPresenceStatusMessageRole));
                            }
                        }

                    }
                    else if (index == 1)
                    {
                        // Account settings
                        var cmd = "/usr/bin/meego-qml-launcher --app meego-ux-settings --opengl --fullscreen --cmd showPage --cdata \"IM\"";  //i18n ok
                        appModel.launch(cmd);
                        //scene.addApplicationPage(accountFactory.componentForAccount(payload.data(AccountsModel.IdRole), scene));
                    }
                    else
                    {
                        // Some Other Action
                    }

                    // By setting the sourceComponent of the loader to undefined,
                    // then the QML engine will destruct the context menu element
                    // much like doing a c++ delete
                    contextLoader.sourceComponent = undefined;
                }
            }
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

        MouseArea {
            ListModel { id: menu}

            anchors.fill: parent
            onClicked: {
                scene.currentAccountId = model.id;
                accountScreenPage.addApplicationPage(contactsScreenContent);
            }

            onPressAndHold: {
                var map = mapToItem(scene, mouseX, mouseY);
                menu.clear();

                if(model.connectionStatus == TelepathyTypes.ConnectionStatusConnected) {
                    menu.append({"modelData":qsTr("Log out")});
                } else {
                    menu.append({"modelData":qsTr("Log in to %1").arg(telepathyManager.accountServiceName(model.icon))})
                }
                menu.append({"modelData":qsTr("Settings")});

                scene.openContextMenu(contextComponent,
                                      contextLoader,
                                      map.x, map.y, model.item, menu);

            }
        }
    }
}
