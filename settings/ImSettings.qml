/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Components 0.1
import MeeGo.Settings 0.1
import MeeGo.App.IM 0.1
import TelepathyQML 0.1

AppPage {
    id: container
    pageTitle: qsTr("Instant Messaging Settings")
    height: childrenRect.height

    //property alias window: scene

    property QtObject appModel : null
    property bool modelsLoaded: false

    property alias mainChatWindow: container

    function createAppModel() {
        if (appModel == null) {
            console.log("creating ApplicationsModel");
            var sourceCode = "import Qt 4.7;"
                           + "import MeeGo.Labs.Components 0.1 as Labs;"
                           + "Labs.ApplicationsModel {}";
            appModel = Qt.createQmlObject(sourceCode, window);
        }
    }

    Component.onCompleted: {
        if (typeof(contactsModel) != 'undefined') {
            contactsModel.setBlockedOnly(true);
        }
        showInfoBar();
    }

    Connections {
        target: accountsModel
        onComponentsLoaded: {
            modelsLoaded = true;
            contactsModel.setBlockedOnly(true);
            showInfoBar("");
        }

        onAccountConnectionStatusChanged: {
            showInfoBar(accountId);
        }
    }

    Translator {
        catalog: "meego-app-im"
    }

    AccountContentFactory {
        id: accountFactory
    }

    // the account setup page
    Component {
        id: accountSetupComponent
        AccountSetupPage {}
    }

    ConfirmationDialog {
        id: confirmationDialogItem
        anchors.horizontalCenter: parent.left
        anchors.verticalCenter: parent.top
    }

    InfoBar {
        id: infoBar
        text: accountsLoading

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        Timer {
            id: infoBarTimer

            interval: 10000
            running: false
            onTriggered: {
                infoBar.hide();
            }
        }
    }

    Column {
        id: contentColumn
        anchors.top: infoBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height

        Image {
            id: accountSettingsLabel
            width: parent.width
            source: "image://themedimage/widgets/common/header/header-small"

            Text{
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: qsTr("Accounts");
                font.pixelSize: theme_fontPixelSizeLarge
                height: parent.height
                width: parent.width
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
        }

        Repeater {
                id: accountsView
                model: typeof(accountsSortedModel) != 'undefined' ? accountsSortedModel : null

                AccountSetupDelegate {
                    parent: contentColumn
                    anchors.left: parent.left
                    anchors.right: parent.right
                }
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: childrenRect.height + 20
            Button {
                id: addAccountButton
                y: 10
                anchors.horizontalCenter: parent.horizontalCenter

                text: (accountsView.count > 0 ? qsTr("Add another account") :
                                                qsTr("Add an account"))
                textColor: theme_buttonFontColor
                bgSourceUp: "image://themedimage/widgets/common/button/button-default"
                bgSourceDn: "image://themedimage/widgets/common/button/button-default-pressed"

                onClicked: window.addPage(accountSetupComponent)
            }
        }

        Image {
            id: generalSettingsLabel
            width: parent.width
            source: "image://themedimage/widgets/common/header/header-small"

            Text{
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: qsTr("General Settings");
                font.pixelSize: theme_fontPixelSizeLarge
                height: parent.height
                width: parent.width
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
        }

        Item {
            width: 10
            height: 10
        }

        Item {
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            height: childrenRect.height

            Text {
                id: offlineContactsText
                anchors.left: parent.left
                anchors.right: offlineContactsToggle.left
                anchors.rightMargin: 10
                text: qsTr("Show offline contacts")
                font.pixelSize: theme_fontPixelSizeLarge
                wrapMode: Text.Wrap
            }

            ToggleButton {
                id: offlineContactsToggle
                on: settingsHelper.showOfflineContacts
                onToggled: settingsHelper.showOfflineContacts = isOn;
                anchors.margins: 10
                anchors.right: parent.right
                anchors.verticalCenter: offlineContactsText.verticalCenter
            }
        }

        Item {
            width: 10
            height: 10
        }

        /*Text {
                anchors.left: parent.left
                anchors.right: audioAlertToggle.left
                anchors.rightMargin: 10
                text: qsTr("Audio alert on new message")
                font.pixelSize: theme_fontPixelSizeLarge
                elide: Text.ElideRight
            }

            ToggleButton {
                id: audioAlertToggle
                on: settingsHelper.enableAudioAlerts
                onToggled: settingsHelper.enableAudioAlerts = isOn;
            }*/

        Item {
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            height: childrenRect.height

            Text {
                id: notificationText
                anchors.left: parent.left
                anchors.right: notificationToggle.left
                anchors.rightMargin: 10
                text: qsTr("Notification on new message")
                font.pixelSize: theme_fontPixelSizeLarge
                wrapMode: Text.Wrap
            }

            ToggleButton {
                id: notificationToggle
                on: settingsHelper.enableNotifications;
                onToggled: settingsHelper.enableNotifications = isOn;
                anchors.margins: 10
                anchors.right: parent.right
                anchors.verticalCenter: notificationText.verticalCenter
            }
        }
            /*Text {
                anchors.left: parent.left
                anchors.right: vibrateToggle.left
                anchors.rightMargin: 10
                text: qsTr("Vibrate on new message")
                font.pixelSize: theme_fontPixelSizeLarge
                elide: Text.ElideRight
            }

            ToggleButton {
                id: vibrateToggle
                on: settingsHelper.enableVibrate
                onToggled: settingsHelper.enableVibrate = isOn;
            }*/
        Item {
            width: 10
            height: 10
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: childrenRect.height + 20
            Button {
                id: clearHistoryButton
                y: 10
                anchors.horizontalCenter: parent.horizontalCenter

                text: qsTr("Clear chat history")
                textColor: theme_buttonFontColor
                bgSourceUp: "image://themedimage/widgets/common/button/button-default"
                bgSourceDn: "image://themedimage/widgets/common/button/button-default-pressed"

                onClicked: accountsModel.clearHistory();
            }
        }

        Item {
            width: 10
            height: 10
        }

        Image {
            id: blockedContactsLabel
            width: parent.width
            source: "image://themedimage/widgets/common/header/header-small"

            visible: blockedList.height > 0

            Text{
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: qsTr("Blocked contacts");
                font.pixelSize: theme_fontPixelSizeLarge
                height: parent.height
                width: parent.width
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
        }

        Item {
            id: blockSpacer
            width: 10
            height: 10
        }

        ContactPickerView {
            id: blockedList
            anchors.margins: 10
            width: parent.width
        }

        Item {
            width: 10
            height: 10
            visible: typeof(contactsModel) != 'undefined' ? contactsModel.rowCount > 0 : false
        }
    }

    function showInfoBar(accountId)
    {
        var text = "";

        if (!networkOnline) {
            text = noNetworkText;
        } else if (!modelsLoaded) {
            text = accountsLoading;
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
                            text += "<br/>";
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
                    return noNetworkText;
                case TelepathyTypes.ConnectionStatusReasonAuthenticationFailed:
                    return qsTr("Sorry, there was a problem logging in to %1. Please go to Settings and retype your username and password.").arg(accountName);
                case TelepathyTypes.ConnectionStatusReasonEncryptionError:
                    return qsTr("Sorry, there was a problem logging in to %1. Deselecting the 'Encryption required' option in Advanced settings should solve this problem.").arg(accountName);
                case TelepathyTypes.ConnectionStatusReasonNameInUse:
                    return qsTr("It looks like you have logged in to %1 from somewhere else, so we have logged you out from this tablet. Try logging in again").arg(accountName);
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
                    return qsTr("Sorry, there was a problem logging in to %1. Selecting the 'Ignore SSL certificate errors' option in the account Advanced settings should solve this problem.").arg(accountName);
                case TelepathyTypes.ConnectionStatusReasonNoneSpecified:
                default:
                    return qsTr("Sorry, there was a problem logging in to %1. Please try again later.").arg(accountName);
            }
        } else {
            return "";
        }
    }

    property string noNetworkText: qsTr("Sorry, we can't login because it is not connected to a network. Go to Wireless & Network settings and connect to an available network");
    property string accountsLoading: qsTr("Loading accounts...");

}
