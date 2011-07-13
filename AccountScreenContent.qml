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

AppPage {
    id: accountScreenPage
    pageTitle: Constants.accountList
    anchors.fill: parent

    property bool modelsLoaded: false

    Component.onCompleted: {
        window.reloadFilterModel();
        showInfoBar();
    }

    onActivated: {
        window.currentScreen = "accounts"; // i18n ok
    }

    Connections {
        target: window
        onComponentsLoaded: {
            modelsLoaded = true;
            accountsRepeater.model = accountsSortedModel;
            accountsModelConnections.target = accountsModel;
            showInfoBar();
        }
    }

    Connections {
        // those signals are not created yet, so wait till they are, then the target will be set
        // and the connections will be made then.
        target: null
        id: accountsModelConnections

        onAccountConnectionStatusChanged: {
            showInfoBar();
        }
    }

    Item {
        id: mainArea
        anchors.fill: parent

        Image {
            anchors.fill: parent
            source: "image://themedimage/widgets/common/backgrounds/global-background-texture"
        }

        InfoBar {
            id: infoBar
            text: Constants.accountsLoading

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            Timer {
                id: infoBarTimer

                interval: 10000
                running: false
                onTriggered: {
                    infoBar.hide();
                }
            }
        }

        Flickable {
            id: flickable
            flickableDirection: Flickable.VerticalFlick
            interactive: contentHeight > height
            anchors.top: infoBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: toolBar.top
            clip: true
            contentHeight: flickableContent.height

            Item {
                id: flickableContent
                width:  parent.width
                height: childrenRect.height

                Title {
                    id: accountsHeader
                    anchors.top: parent.top
                    text: Constants.accountChoose
                    visible: accountsRepeater.count
                }

                Column {
                    id: accountsListView

                    anchors.top: accountsHeader.visible ? accountsHeader.bottom :
                    parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right

                    Behavior on height {
                        NumberAnimation {
                            duration: 250
                        }
                    }

                    Repeater {
                        id: accountsRepeater
                        AccountDelegate {
                            id: accountDelegate
                            width: accountsListView.width
                        }
                    }
                }

                Title {
                    id: setupSeparator
                    anchors.top: accountsListView.bottom
                    text: Constants.accountAdd
                }

                AccountSetupContent {
                    anchors.top:  setupSeparator.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: childrenRect.height
                }
            }
        }

        BottomToolBar {
            id: toolBar

            Component.onCompleted: {
                show()
            }

            content: BottomToolBarRow {
                id: bottomRow
                contentVerticalMargins: 0

                leftContent: [
                    IconButton {
                        id: settingsButton
                        icon: "image://themedimage/icons/actionbar/show-settings"
                        iconDown: icon + "-active"
                        hasBackground: false

                        onClicked: {
                            var cmd = "/usr/bin/meego-qml-launcher --app meego-ux-settings --opengl --fullscreen --cmd showPage --cdata \"IM\"";  //i18n ok
                            createAppModel();
                            appModel.launch(cmd);
                        }

                        Image
                        {
                            anchors.right: parent.right
                            source: "image://themedimage/widgets/common/action-bar/action-bar-separator"
                            height: parent.height
                        }
                    }
                ]
            }
        }
    }

    Loader {
        id: contextMenuLoader
    }

    Component {
        id: contextMenuComponent
        ContextMenu {
            id: contextMenu

            property alias model : actionMenu.model
            property alias payload : actionMenu.payload

            // if we don't change the parent here, the maximum height of the context menu is that of the account row,
            // and not of the whole list
            parent: accountsListView
            content:  ActionMenu {
                id: actionMenu

                onTriggered: {
                    if (index == 0)
                    {
                        if(payload.data(AccountsModel.ConnectionStatusRole) != TelepathyTypes.ConnectionStatusDisconnected) {
                            payload.setRequestedPresence(TelepathyTypes.ConnectionPresenceTypeOffline,
                                                "offline", // i18n ok
                                                payload.data(AccountsModel.RequestedPresenceStatusMessageRole));
                        } else {
                            var icon = payload.data(AccountsModel.IconRole);
                            var id = payload.data(AccountsModel.IdRole);
                            var serviceName = protocolsModel.titleForId(icon);

                            // if the protocol only allows to have one account connected at a time,
                            // ask the user if he really wants to do that
                            if (protocolsModel.isSingleInstance(icon) &&
                                accountFactory.otherAccountsOnline(icon, id)) {
                                // show the dialog asking the user if he really wants to connect the account

                                confirmationDialogItem.title = Constants.multipleAccountsTitle;
                                confirmationDialogItem.text = Constants.multipleAccountsText.arg(serviceName);
                                confirmationDialogItem.instanceReason = "account-delegate-single-instance"; // i18n ok
                                confirmationDialogItem.accountId = id;
                                confirmationDialogItem.show();
                            } else {
                                if(payload.data(AccountsModel.AutomaticPresenceTypeRole) != TelepathyTypes.ConnectionPresenceTypeOffline) {
                                    payload.setRequestedPresence(payload.data(AccountsModel.AutomaticPresenceTypeRole),
                                                                 payload.data(AccountsModel.AutomaticPresenceRole),
                                                                 payload.data(AccountsModel.AutomaticPresenceStatusMessageRole));
                                } else {
                                    payload.setRequestedPresence(TelepathyTypes.ConnectionPresenceTypeAvailable,
                                                                 payload.data(AccountsModel.AutomaticPresenceRole),
                                                                 payload.data(AccountsModel.AutomaticPresenceStatusMessageRole));
                                }
                            }
                        }

                    }
                    else if (index == 1)
                    {
                        // Account settings
                        var cmd = "/usr/bin/meego-qml-launcher --app meego-ux-settings --opengl --fullscreen --cmd showPage --cdata \"IM\"";  //i18n ok
                        createAppModel();
                        appModel.launch(cmd);
                    }
                    contextMenu.hide();
                }
            }
        }
    }

    function showInfoBar()
    {
        var text = "";

        if (!networkOnline) {
            text = Constants.noNetworkText;
        } else if (!modelsLoaded) {
            text = Constants.accountsLoading;
        } else {
            // check account status
            for (var i = 0; i < accountsModel.accountCount; ++i) {
                var status = accountsModel.dataByRow(i, AccountsModel.ConnectionStatusRole);
                var id = accountsModel.dataByRow(i, AccountsModel.IdRole);
                if (status != TelepathyTypes.ConnectionStatusConnected) {
                    var reason = accountsModel.dataByRow(i, AccountsModel.ConnectionStatusReasonRole);
                    var accountName =  accountsModel.dataByRow(i, AccountsModel.DisplayNameRole);
                    var accountText = accountStatusMessage(status, reason, accountName);
                    if (accountText != "") {
                        // make sure the text is not duplicated
                        if (text.indexOf(accountText) == -1) {
                            if (text != "") {
                                text += "<br\>";
                            }
                            text += accountText;
                        }
                    }
                }
            }
        }

        // assign and show/hide as necessary
        infoBar.text = text;
        if (text == "") {
            infoBar.hide();
            infoBarTimer.stop();
        } else {
            infoBar.show();
            infoBarTimer.restart();
        }
    }

    function accountStatusMessage(status, reason, accountName)
    {
        if (status == TelepathyTypes.ConnectionStatusDisconnected) {
            switch(reason) {
                case TelepathyTypes.ConnectionStatusReasonRequested:
                    return "";
                case TelepathyTypes.ConnectionStatusReasonNetworkError:
                    return Constants.noNetworkText;
                case TelepathyTypes.ConnectionStatusReasonAuthenticationFailed:
                    return Constants.errorLoginAccount.arg(accountName)
                case TelepathyTypes.ConnectionStatusReasonEncryptionError:
                    return Constants.errorEncryptionAccountDeselect.arg(accountName);
                case TelepathyTypes.ConnectionStatusReasonNameInUse:
                    return Constants.errorLogoutAccountConnectedElse.arg(accountName);
                case TelepathyTypes.ConnectionStatusReasonCertUntrusted:
                case TelepathyTypes.ConnectionStatusReasonCertExpired:
                case TelepathyTypes.ConnectionStatusReasonCertNotActivated:
                case TelepathyTypes.ConnectionStatusReasonCertHostnameMismatch:
                case TelepathyTypes.ConnectionStatusReasonCertFingerprintMismatch:
                case TelepathyTypes.ConnectionStatusReasonCertSelfSigned:
                case TelepathyTypes.ConnectionStatusReasonCertOtherError:
                case TelepathyTypes.ConnectionStatusReasonCertRevoked:
                case TelepathyTypes.ConnectionStatusReasonCertInsecure:
                case TelepathyTypes.ConnectionStatusReasonCertLimitExceeded:
                    return Constants.errorSslAccountError.arg(accountName);
                case TelepathyTypes.ConnectionStatusReasonNoneSpecified:
                default:
                    return Constants.errorLoginAccountTryLater.arg(accountName);
            }
        } else if (status == TelepathyTypes.ConnectionStatusConnecting) {
            return Constants.contactScreenAccountConnecting;
        } else {
            return "";
        }
    }
}
