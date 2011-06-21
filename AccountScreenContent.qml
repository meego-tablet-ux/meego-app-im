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
        showInfoBar("");
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
            showInfoBar("");
        }
    }

    Connections {
        // those signals are not created yet, so wait till they are, then the target will be set
        // and the connections will be made then.
        target: null
        id: accountsModelConnections

        onAccountConnectionStatusChanged: {
            showInfoBar(accountId);
        }
    }

    Item {
        id: mainArea
        anchors.fill: parent

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

    function showInfoBar(accountId)
    {
        var text = "";

        if (!networkOnline) {
            text = Constants.noNetworkText;
        } else if (!modelsLoaded) {
            text = Constants.accountsLoading;
        } else if (accountId == "") {
            // check account status
            for (var i = 0; i < accountsModel.accountCount; ++i) {
                var status = accountsModel.dataByRow(i, AccountsModel.ConnectionStatusRole);
                var id = accountsModel.dataByRow(i, AccountsModel.IdRole);
                if (status == TelepathyTypes.ConnectionStatusDisconnected) {
                    var reason = accountsModel.dataByRow(i, AccountsModel.ConnectionStatusReasonRole);
                    var accountName =  accountsModel.dataByRow(i, AccountsModel.DisplayNameRole);
                    var accountText = accountStatusMessage(status, reason, accountName);
                    if (accountText != "") {
                        if (text != "") {
                            text += "<br\>";
                        }
                        text += accountText;
                    }
                }
            }
        } else if (accountId != "") {
            // we access it by row because we need to display the service name
            // having the id, we could just get the id, but then the display name would not be the service name we need
            for (var i = 0; i < accountsModel.accountCount; ++i) {
                var id = accountsModel.dataByRow(i, AccountsModel.IdRole);
                if (accountId == id) {
                    var status = accountsModel.dataByRow(i, AccountsModel.ConnectionStatusRole);
                    var reason = accountsModel.dataByRow(i, AccountsModel.ConnectionStatusReasonRole);
                    var accountName =  accountsModel.dataByRow(i, AccountsModel.DisplayNameRole);

                    // calculate the error message and add it to the current one
                    var accountText = accountStatusMessage(status, reason, accountName);
                    if (accountText != "") {
                        if (text != "") {
                            text += "<br\>";
                        }
                        text += accountText;
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
