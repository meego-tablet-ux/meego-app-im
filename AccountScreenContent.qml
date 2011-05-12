/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Components 0.1
import MeeGo.Labs.Components 0.1 as Labs
import MeeGo.App.IM 0.1

AppPage {
    id: accountScreenPage
    pageTitle: qsTr("Account list")
    anchors.fill: parent

    Component.onCompleted: {
        window.reloadFilterModel();
    }

    Connections {
        target: window
        onComponentsLoaded: {
            accountsRepeater.model = accountsSortedModel;
            loadingAccounts.visible = false;
        }
    }

    Item {
        id: mainArea
        anchors.fill: parent

        Flickable {
            id: flickable
            flickableDirection: Flickable.VerticalFlick
            interactive: contentHeight > height
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: toolBar.top
            clip: true
            contentHeight: flickableContent.height

            Item {
                id: flickableContent
                width:  parent.width
                height: childrenRect.height

                NoNetworkHeader {
                    id: noNetworkItem
                }

                LoadingAccounts {
                    id: loadingAccounts
                    visible: true
                    anchors {
                        top: noNetworkItem.bottom
                        left: parent.left
                        right: parent.right
                    }
                }

                Title {
                    id: accountsHeader
                    anchors.top: loadingAccounts.bottom
                    text: qsTr("Choose an account")
                    visible: accountsRepeater.count
                }

                Column {
                    id: accountsListView

                    anchors.top: accountsHeader.visible ? accountsHeader.bottom :
                    noNetworkItem.bottom
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
                    text: qsTr("Add an account")
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
            source: "image://meegotheme/widgets/common/action-bar/action-bar-background"

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            Labs.ApplicationsModel {
                id: appModel
            }

            IconButton {
                id: settingsButton
                icon: "image://meegotheme/icons/actionbar/show-settings"
                iconDown: icon + "-active"
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 10
                hasBackground: false

                onClicked: {
                    var cmd = "/usr/bin/meego-qml-launcher --app meego-ux-settings --opengl --fullscreen --cmd showPage --cdata \"IM\"";  //i18n ok
                    appModel.launch(cmd);
                }
            }

            Image {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: settingsButton.right
                anchors.leftMargin: 10
                source: "image://meegotheme/widgets/common/action-bar/action-bar-separator"
            }
        }
    }
}
