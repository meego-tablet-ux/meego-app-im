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

ApplicationPage {
    id: accountEditPage
    anchors.fill: parent

    // The label is used as the user visible string in the crumb trail
    title: accountFactory.accountServiceName(accountContent.icon)

    property variant accountContent

    Component.onCompleted: {
        accountContent.parent = accountContentPlaceholder;
        accountContent.anchors.top = accountContent.parent.top;
        accountContent.anchors.topMargin = 10;
        scene.title = accountFactory.accountServiceName(accountContent.icon)
    }

    Connections {
        target: accountContent
        onFinished: {
            modalSurface.visible = false;
            scene.previousApplicationPage();
            // this second call is for the settings module
            scene.previousApplicationPage();
        }

        onAccountCreationAborted: {
            modalSurface.visible = false;
        }
    }

    Item {
        id: pageContent
        parent: accountEditPage.content
        anchors.fill: parent

        AccountContentFactory {
            id: accountFactory
        }

        Title {
            id: title
            text:  accountFactory.accountServiceName(accountContent.icon)
            subtext: " " // i18n ok

            Image {
                id: accountIcon
                source: accountFactory.accountIcon(accountContent.icon)
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.margins: 5
            }


            label.anchors.left: accountIcon.right
            label.anchors.top: undefined
            label.anchors.verticalCenter: title.verticalCenter
            label.font.pixelSize: theme_fontPixelSizeLargest
        }

        Flickable {
            id: flickable
            flickableDirection: Flickable.VerticalFlick
            anchors.top: title.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            contentHeight: mainArea.height

            Column {
                id: mainArea
                anchors.left:  parent.left
                anchors.right: parent.right
                height: childrenRect.height

                ContentRow {
                    id: accountContentPlaceholder
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: childrenRect.height + 20
                }

                ContentRow {
                    Item {
                        id: centerItem
                        width: childrenRect.width
                        height: parent.height
                        anchors.centerIn: parent

                        Button {
                            id: doneButton
                            anchors {
                                margins: 10
                                left: parent.left
                                verticalCenter: parent.verticalCenter
                            }

                            height: 32

                            title: qsTr("Done")
                            color: theme_buttonFontColor
                            bgSourceUp: "image://meegotheme/widgets/common/button/button-default"
                            bgSourceDn: "image://meegotheme/widgets/common/button/button-default-pressed"

                            onClicked: {
                                modalSurface.visible = true;
                                accountContent.createAccount();
                            }
                        }

                        Button {
                            id: cancelButton
                            anchors {
                                margins: 10
                                left: doneButton.right
                                verticalCenter: parent.verticalCenter
                            }

                            height: 32

                            title: qsTr("Cancel")
                            color: theme_buttonFontColor
                            bgSourceUp: "image://meegotheme/widgets/common/button/button-negative"
                            bgSourceDn: "image://meegotheme/widgets/common/button/button-negative-pressed"

                            onClicked: scene.previousApplicationPage();
                        }
                    }
                }
            }
        }

        ModalSurface {
            id: modalSurface
            anchors.fill: parent
            autoCenter: true
            closeUponFogClicked: false

            content: Component {

                Spinner {
                    spinning: true
                    onSpinningChanged: {
                        if (!spinning) {
                            spinning = modalSurface.visible;
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                // just to avoid clicking in the underlying components
            }
        }
    }
}
