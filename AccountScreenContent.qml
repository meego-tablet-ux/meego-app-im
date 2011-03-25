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

ApplicationPage {
    id: accountScreenPage
    title: qsTr("Account list")
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.bottom: toolBar.top

    Component.onCompleted: {
        scene.title = qsTr("Account list");
        scene.reloadFilterModel();
    }

    Item {
        id: mainArea
        parent: accountScreenPage.content
        anchors.fill: parent

        Flickable {
            id: flickable
            flickableDirection: Flickable.VerticalFlick
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: toolBar.top

            contentHeight: flickableContent.height

            Item {
                id: flickableContent
                width:  parent.width
                height: childrenRect.height

                NoNetworkHeader {
                    id: noNetworkItem
                }

                Title {
                    id: accountsHeader
                    anchors.top: noNetworkItem.bottom
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
                        model: accountsSortedModel
                        AccountDelegate {
                            id: accountDelegate
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

            ApplicationsModel {
                id: appModel
            }

            IconButton {
                icon: "image://meegotheme/icons/actionbar/show-settings"
                iconDown: icon + "-active"
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 10

                onClicked: {
                    var cmd = "/usr/bin/meego-qml-launcher --app meego-ux-settings --opengl --fullscreen --cmd showPage --cdata \"IM\"";  //i18n ok
                    appModel.launch(cmd);
                }
            }
        }
    }
}
