/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Components 0.1
import MeeGo.App.IM 0.1
import "constants.js" as Constants

AppPage {
    id: accountScreenPage
    pageTitle: Constants.accountList
    anchors.fill: parent

    property bool modelsLoaded: false

    Component.onCompleted: {
        console.log("AccountScreenContent.onCompleted");
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
            showInfoBar();
        }
    }

    Item {
        id: mainArea
        anchors.fill: parent

        Flickable {
            id: flickable
            flickableDirection: Flickable.VerticalFlick
            interactive: contentHeight > height
            anchors.fill: parent
            clip: true
            contentHeight: flickableContent.height

            Item {
                id: flickableContent
                width:  parent.width
                height: childrenRect.height

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

                Title {
                    id: accountsHeader
                    anchors.top: infoBar.bottom
                    text: Constants.accountChoose
                    visible: accountsRepeater.count
                }

                Column {
                    id: accountsListView

                    anchors.top: accountsHeader.visible ? accountsHeader.bottom :
                    infoBar.bottom
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

        BorderImage {
            id: toolBar
            verticalTileMode: BorderImage.Stretch
            source: "image://themedimage/widgets/common/action-bar/action-bar-background"

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            IconButton {
                id: settingsButton
                icon: "image://themedimage/icons/actionbar/show-settings"
                iconDown: icon + "-active"
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 10
                hasBackground: false

                onClicked: {
                    var cmd = "/usr/bin/meego-qml-launcher --app meego-ux-settings --opengl --fullscreen --cmd showPage --cdata \"IM\"";  //i18n ok
                    createAppModel();
                    appModel.launch(cmd);
                }
            }

            Image {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: settingsButton.right
                anchors.leftMargin: 10
                source: "image://themedimage/widgets/common/action-bar/action-bar-separator"
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

    function accountStatusMessage(status, accountName)
    {
        var connectionStatusReason = window.accountItem.data(AccountsModel.ConnectionStatusReasonRole)
        if (accountStatus == TelepathyTypes.ConnectionStatusDisconnected) {
            switch(connectionStatusReason) {
                case TelepathyTypes.ConnectionStatusReasonNoneSpecified:
                case TelepathyTypes.ConnectionStatusReasonRequested:
                case TelepathyTypes.ConnectionStatusReasonNetworkError:
                    return "";
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
                default:
                    return Constants.errorLoginTryLater;
            }
        } else if (accountStatus == TelepathyTypes.ConnectionStatusConnecting) {
            return Constants.contactScreenAccountConnecting;
        } else {
            return "";
        }
    }
}
