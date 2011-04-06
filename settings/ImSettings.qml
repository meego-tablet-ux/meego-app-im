/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Labs.Components 0.1
import MeeGo.Settings 0.1
import MeeGo.App.IM 0.1

ApplicationPage {
    id: container
    title: qsTr("Instant Messaging Settings")
    anchors.fill: parent

    Connections {
        target: accountsModel
        onComponentsLoaded: {
            contactsModel.setBlockedOnly(true);
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
        ApplicationPage {
            id: accountSetupPage
            anchors.fill: parent

            Flickable {
                id: accountSetupArea
                parent: accountSetupPage.content
                anchors.fill: parent
                clip: true

                flickableDirection: "VerticalFlick"
                contentHeight: accountSetupItem.height

                AccountSetupContent {
                    id: accountSetupItem
                    anchors.left: parent.left
                    anchors.right: parent.right
                }
            }
        }
    }

    Component {
        id: confirmationDialogContent
        ConfirmationDialog {
            id: confirmationDialogItem
        }
    }

    Flickable {
        parent: container.content
        anchors.fill: parent
        flickableDirection: "VerticalFlick"
        contentHeight: contentColumn.height
        clip: true

        Column {
            id: contentColumn
            anchors.left: parent.left
            anchors.right: parent.right
            height: childrenRect.height

            Image {
                id: accountSettingsLabel
                width: parent.width
                source: "image://theme/settings/subheader"

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
                    model: accountsModel


                    AccountSetupDelegate {
                        anchors.left: parent.left
                        anchors.right: parent.right
                    }
            }

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                height: 40
                Button {
                    id: deleteAccountButton
                    anchors.centerIn: parent

                    height: 32

                    title: qsTr("Add another account")
                    color: theme_buttonFontColor
                    bgSourceUp: "image://meegotheme/widgets/common/button/button-default"
                    bgSourceDn: "image://meegotheme/widgets/common/button/button-default-pressed"

                    onClicked: container.addApplicationPage(accountSetupComponent)
                }
            }

            Image {
                id: generalSettingsLabel
                width: parent.width
                source: "image://theme/settings/subheader"

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
                id: spacer
                width: 10
                height: 10
            }

            Grid {
                columns: 2
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.right: parent.right
                spacing: 10

                Text {
                    anchors.margins: 10
                    text: qsTr("Show offline contacts")
                    font.pixelSize: theme_fontPixelSizeLarge
                }

                ToggleButton {
                    id: offlineContactsToggle
                    on: settingsHelper.showOfflineContacts
                    onToggled: settingsHelper.showOfflineContacts = isOn;
                }

                /*Text {
                    anchors.margins: 10
                    text: qsTr("Audio alert on new message")
                    font.pixelSize: theme_fontPixelSizeLarge
                }

                ToggleButton {
                    id: audioAlertToggle
                    on: settingsHelper.enableAudioAlerts
                    onToggled: settingsHelper.enableAudioAlerts = isOn;
                }*/

                Text {
                    anchors.margins: 10
                    text: qsTr("Notification on new message")
                    font.pixelSize: theme_fontPixelSizeLarge
                }

                ToggleButton {
                    id: notificationToggle
                    on: settingsHelper.enableNotifications;
                    onToggled: settingsHelper.enableNotifications = isOn;
                }

                /*Text {
                    anchors.margins: 10
                    text: qsTr("Vibrate on new message")
                    font.pixelSize: theme_fontPixelSizeLarge
                }

                ToggleButton {
                    id: vibrateToggle
                    on: settingsHelper.enableVibrate
                    onToggled: settingsHelper.enableVibrate = isOn;
                }*/
            }

            Item {
                width: 10
                height: 10
            }

            Image {
                id: blockedContactsLabel
                width: parent.width
                source: "image://theme/settings/subheader"

                visible: contactsModel.rowCount > 0

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
                visible: contactsModel.rowCount > 0
            }

            Item {
                width: 10
                height: 10
                visible: contactsModel.rowCount > 0
            }
        }
    }
}
