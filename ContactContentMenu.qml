/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Components 0.1 as Ux
import MeeGo.Labs.Components 0.1
import MeeGo.App.IM 0.1
import TelepathyQML 0.1
import MeeGo.Media 0.1

Item {
    id: meTabItem

    property variant currentPage
    property string nameColor: "black"

    // FIXME remove after full migration to MeegGo.Components
    property variant window : scene

    signal accountChanged

    function confirmAccountLogin()
    {
        var serviceName = protocolsModel.titleForId(scene.accountItem.data(AccountsModel.IconRole));

        // show the dialog to ask for user confirmation
        showModalDialog(confirmationDialogContent);
        dialogLoader.item.dialogTitle = qsTr("Multiple accounts connected");
        dialogLoader.item.mainText = qsTr("Do you really want to connect this account?");
        dialogLoader.item.subText = qsTr("By doing this all other %1 accounts will be disconnected.").arg(serviceName);
        dialogLoader.item.instanceReason = "contact-menu-single-instance"; // i18n ok
    }

    Component.onCompleted: {
        if (scene.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole) != "") {
            statusMessage.text = scene.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole);
        } else {
            statusMessage.text = scene.presenceStatusText(scene.accountItem.data(AccountsModel.CurrentPresenceTypeRole));
        }
        statusRadioGroup.select(scene.accountItem.data(AccountsModel.CurrentPresenceTypeRole));
    }

    Connections {
        target: scene.accountItem
        // a small trick
        onChanged: {
            scene.accountItem = scene.accountItem
            if (scene.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole) != "") {
                statusMessage.text = scene.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole);
            } else {
                statusMessage.text = scene.presenceStatusText(scene.accountItem.data(AccountsModel.CurrentPresenceTypeRole));
            }
            displayName.text = scene.accountItem.data(AccountsModel.NicknameRole);
            presenceIcon.status = scene.accountItem.data(AccountsModel.CurrentPresenceTypeRole);
        }
    }

    Connections {
        target: scene
        onCurrentAccountIdChanged: {
             scene.accountItem = accountsModel.accountItemForId(scene.currentAccountId);
        }
    }

    width: 400
    height: meColumn.height

    Column {
        id: meColumn
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: theme_fontPixelSizeLarge / 2

        Item {
            id: meInfo
            height: 64
            width: parent.width

            Image {
                anchors.fill: parent
                source: "image://meegotheme/widgets/common/menu/menu-background"
            }

            Avatar {
                id: avatarImage
                anchors.leftMargin: 10
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 2
                source: "image://avatars/" + scene.accountItem.data(AccountsModel.IdRole) + // i18n ok
                            "?" + accountFactory.avatarSerial
                noAvatarImage: "image://meegotheme/widgets/common/avatar/avatar-default"
            }

            Component {
                id: avatarMenu
                PictureChangeMenu {
                    id: pictureChangeMenu
                    onClose: contextLoader.sourceComponent = undefined
                }
            }

            MouseArea {
                id: avatarMouseArea
                anchors.fill: avatarImage

                onClicked: {
                    photoPicker.show();
                }
            }

            Column {
                id: accountInfoColumn
                anchors.margins: 5
                anchors.left: avatarImage.right
                anchors.right: parent.right
                anchors.verticalCenter: avatarImage.verticalCenter
                spacing: 5

                Text {
                    id: displayName
                    width: parent.width
                    text: scene.accountItem.data(AccountsModel.NicknameRole)
                    color: nameColor
                    font.pixelSize: theme_fontPixelSizeLarge
                    elide: Text.ElideRight
                }

                Row {
                    id: statusRow
                    spacing: 5
                    anchors.right: parent.right
                    anchors.left: parent.left

                    PresenceIcon {
                        id: presenceIcon
                        anchors.verticalCenter: statusMessage.verticalCenter
                        status: scene.accountItem.data(AccountsModel.CurrentPresenceTypeRole)
                    }

                    Text {
                        id: statusMessage
                        text: ""
                        color: theme_contextMenuFontColor
                        font.pixelSize: theme_fontPixelSizeLarge
                        width: parent.width - presenceIcon.width
                        elide: Text.ElideRight
                    }
                }
            }
        }

        Image {
            anchors.left: parent.left
            anchors.right: parent.right
            source: "image://meegotheme/widgets/common/menu/menu-item-separator"
        }

        Item {
            id: meUpdateStatus
            height: (statusMenu.visible ?
                         meUpdateStatusText.height + statusMenu.height + theme_contextMenuFontPixelSize: meUpdateStatusText.height)
            width: parent.width

            Behavior on height {
                NumberAnimation { duration: 250 }
            }

            Text {
                id: meUpdateStatusText
                anchors.top: parent.top
                anchors.leftMargin: 10
                anchors.left: parent.left
                anchors.right: parent.right
                verticalAlignment: Text.AlignVCenter
                text: qsTr("Update status")
                font.pixelSize: theme_contextMenuFontPixelSize
                color: theme_contextMenuFontColor
            }

            MouseArea {
                id: updateMouseArea
                anchors.fill: meUpdateStatusText

                onClicked: {
                    if(statusMenu.visible == false) {
                        statusMenu.visible = true;
                        customMessageBox.focus = true;
                    } else {
                        statusMenu.visible = false;
                    }
                }
            }

            Column {
                id: statusMenu
                anchors.top: meUpdateStatusText.bottom
                anchors.topMargin: 10
                anchors.right: parent.right
                anchors.leftMargin: 10
                anchors.left: parent.left
                visible: false
                spacing: 5

                ListModel {
                    id: statusModel
                    ListElement {
                        status: "available"; // i18n ok
                        type: TelepathyTypes.ConnectionPresenceTypeAvailable
                        text: QT_TR_NOOP("Available")
                    }
                    ListElement {
                        status: "away"; // i18n ok
                        type: TelepathyTypes.ConnectionPresenceTypeAway
                        text: QT_TR_NOOP("Away")
                    }
                    ListElement {
                        status: "busy"; // i18n ok
                        type: TelepathyTypes.ConnectionPresenceTypeBusy
                        text: QT_TR_NOOP("Busy")
                    }
                    ListElement {
                        status: "invisible"; // i18n ok
                        type: TelepathyTypes.ConnectionPresenceTypeHidden
                        text: QT_TR_NOOP("Invisible")
                    }
                    ListElement {
                        status: "offline"; // i18n ok
                        type: TelepathyTypes.ConnectionPresenceTypeOffline
                        text: QT_TR_NOOP("Offline")
                    }
                }

                RadioGroup {
                    id: statusRadioGroup
                }

                property string statusString: ""

                ListView {
                    id: statusView
                    model: statusModel
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: statusModel.count * (theme_contextMenuFontPixelSize + 14)

                    delegate: Component {
                        Item {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: childrenRect.height

                            Image {
                                anchors.fill: delegateRow
                                source: "image://meegotheme/widgets/common/menu/menu-background"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    statusRadioGroup.select(model.type);
                                    var icon = scene.accountItem.data(AccountsModel.IconRole);
                                    var id = scene.accountItem.data(AccountsModel.IdRole);
                                    // if the protocol doesn t allow for multiple accounts to be online
                                    // at the same time, we need to ask the user if he wants to disconnect
                                    // the other accounts
                                    if (!protocolsModel.isSingleInstance(icon) ||
                                        accountFactory.otherAccountsOnline(icon, id) == 0 ||
                                        model.type == TelepathyTypes.ConnectionPresenceTypeOffline) {
                                            scene.accountItem.setRequestedPresence(model.type, model.status, customMessageBox.text);
                                            scene.accountItem.setAutomaticPresence(model.type, model.status, customMessageBox.text);
                                    } else {
                                        contactsScreenPage.requestedStatusType = model.type;
                                        contactsScreenPage.requestedStatus = model.status;
                                        contactsScreenPage.requestedStatusMessage = customMessageBox.text;
                                        confirmAccountLogin();
                                    }
                                }
                            }

                            Row {
                                id: delegateRow
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.leftMargin: 5
                                height: statusRadioButton.height + 5
                                spacing: 10

                                RadioButton {
                                    id: statusRadioButton
                                    value: model.type
                                    group: statusRadioGroup
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Text {
                                    id: statusText
                                    anchors.verticalCenter: statusRadioButton.verticalCenter
                                    text: qsTr(model.text)
                                    font.pixelSize: theme_contextMenuFontPixelSize
                                    color: theme_contextMenuFontColor
                                }
                            }
                        }
                    }
                }

                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.rightMargin: 15
                    height: customMessageBox.height + 20

                    BorderImage {
                        id: customMessageBorder
                        anchors.fill: parent
                        source: "image://meegotheme/widgets/common/text-area/text-area-background"
                        border.left: 5; border.top: 5
                        border.right: 5; border.bottom: 5
                    }

                    TextArea {
                        id: customMessageBox
                        anchors {
                            left: parent.left
                            right: parent.right
                            leftMargin: 15
                            rightMargin: 15
                            verticalCenter: parent.verticalCenter
                        }
                        text: scene.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole)
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                        Behavior on height {
                            NumberAnimation { duration: 250 }
                        }
                    }
                }

                Button {
                    id: updateStatusButton
                    height: 30
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    title: qsTr("Update")
                    color: theme_buttonFontColor
                    bgSourceUp: "image://meegotheme/widgets/common/button/button-default"
                    bgSourceDn: "image://meegotheme/widgets/common/button/button-default-pressed"
                    onClicked: {
                        var status;
                        for(var i = 0; i < statusModel.count; ++i) {
                            if (statusRadioGroup.selectedValue == statusModel.get(i).type) {
                                status = statusModel.get(i).text;
                            }
                        }

                        scene.accountItem.setRequestedPresence(statusRadioGroup.selectedValue, status, customMessageBox.text);
                        scene.accountItem.setAutomaticPresence(statusRadioGroup.selectedValue, status, customMessageBox.text);
                        currentPage.closeMenu();
                    }
                }
            }
        }

        Image {
            anchors.left: parent.left
            anchors.right: parent.right
            source: "image://meegotheme/widgets/common/menu/menu-item-separator"
        }

        Item {
            id: meUpdateNick
            height: nicknameColumn.visible ? childrenRect.height : meUpdateNicknameText.height
            width: parent.width

            Image {
                anchors.fill: parent
                source: "image://meegotheme/widgets/common/menu/menu-background"
            }

            Text {
                id: meUpdateNicknameText
                anchors.top: parent.top
                anchors.leftMargin: 10
                anchors.left: parent.left
                anchors.right: parent.right
                verticalAlignment: Text.AlignVCenter
                text: qsTr("Update user display name")
                font.pixelSize: theme_contextMenuFontPixelSize
                color: theme_contextMenuFontColor
            }

            MouseArea {
                id: updateNicknameMouseArea
                anchors.fill: meUpdateNicknameText

                onClicked: {
                    if (nicknameColumn.visible) {
                        nicknameColumn.opacity = 0;
                    } else {
                        nicknameColumn.opacity = 1;
                    }
                }
            }

            Timer {
                id: nicknameHideTimer
                interval: 1000
                repeat: false

                onTriggered: nicknameColumn.opacity = 0
            }

            Column {
                id: nicknameColumn
                anchors.top: meUpdateNicknameText.bottom
                anchors.topMargin: theme_contextMenuFontPixelSize / 2
                anchors.left: parent.left
                anchors.right: parent.right
                height: childrenRect.height
                spacing: 10
                visible: opacity > 0
                opacity: 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 250
                    }
                }

                onVisibleChanged: {
                    if (visible) {
                        nicknameBox.text = scene.accountItem.data(AccountsModel.NicknameRole)
                    }
                }

                TextEntry {
                    id: nicknameBox
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 15
                    height: 30

                    text: scene.accountItem.data(AccountsModel.NicknameRole)

                    function updateNickname() {
                        if (nicknameBox.text != "") {
                            scene.accountItem.setNickname(nicknameBox.text);
                            nicknameHideTimer.start();
                        }
                    }
                }

                Button {
                    id: updateNicknameButton
                    height: 30
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    title: qsTr("Update")
                    color: theme_buttonFontColor
                    bgSourceUp: "image://meegotheme/widgets/common/button/button-default"
                    bgSourceDn: "image://meegotheme/widgets/common/button/button-default-pressed"
                    onClicked: {
                        nicknameBox.updateNickname();
                    }
                }
            }
        }

        Image {
            anchors.left: parent.left
            anchors.right: parent.right
            source: "image://meegotheme/widgets/common/menu/menu-item-separator"
        }

        Item {
            id: addIMContact
            height: addIMContactText.height
            width: parent.width
            visible: scene.accountItem.data(AccountsModel.ConnectionStatusRole) == TelepathyTypes.ConnectionStatusConnected

            Image {
                anchors.fill: parent
                source: "image://meegotheme/widgets/common/menu/menu-background"
            }

            Text {
                id: addIMContactText
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 10
                anchors.left: parent.left
                anchors.right: parent.right
                verticalAlignment: Text.AlignVCenter
                text: qsTr("Add a friend")
                font.pixelSize: theme_contextMenuFontPixelSize
                color: theme_contextMenuFontColor
            }

            MouseArea {
                id: addIMContactMouseArea
                anchors.fill: parent
                onClicked: {
                    if(addAFriend.opacity == 1) {
                        addAFriend.opacity = 0;
                        addAFriend.resetHelper();
                    } else {
                        addAFriend.opacity = 1;
                    }
                }
            }
        }

        AddAFriend {
            id: addAFriend
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            visible: opacity > 0
            opacity: 0

            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                }
            }
        }

        Image {
            anchors.left: parent.left
            anchors.right: parent.right
            source: "image://meegotheme/widgets/common/menu/menu-item-separator"
        }

        Item {
            id: meLogOut
            height: meLogOutText.height + meLogOutText.height / 2
            width: parent.width

            Image {
                anchors.fill: parent
                source: "image://meegotheme/widgets/common/menu/menu-background"
            }

            Text {
                id: meLogOutText
                anchors.top: parent.top
                anchors.leftMargin: 10
                anchors.left: parent.left
                anchors.right: parent.right
                verticalAlignment: Text.AlignVCenter
                text: (scene.accountItem.data(AccountsModel.ConnectionStatusRole) == TelepathyTypes.ConnectionStatusDisconnected?
                           qsTr("Log in") : qsTr("Log out"))
                font.pixelSize: theme_contextMenuFontPixelSize
                color: theme_contextMenuFontColor
            }

            MouseArea {
                id: logOutMouseArea
                anchors.fill: parent

                onClicked: {
                    if(scene.accountItem.data(AccountsModel.ConnectionStatusRole) == TelepathyTypes.ConnectionStatusDisconnected) {
                        contactsScreenPage.requestedStatusType = TelepathyTypes.ConnectionPresenceTypeAvailable;
                        contactsScreenPage.requestedStatus = "available"; // i18n ok
                        contactsScreenPage.requestedStatusMessage = scene.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole);

                        var icon = scene.accountItem.data(AccountsModel.IconRole);
                        var id = scene.accountItem.data(AccountsModel.IdRole);

                        if (!protocolsModel.isSingleInstance(icon) ||
                            accountFactory.otherAccountsOnline(icon, id) == 0) {
                                scene.accountItem.setRequestedPresence(model.type, model.status, customMessageBox.text);
                                scene.accountItem.setAutomaticPresence(model.type, model.status, customMessageBox.text);
                        } else {
                            confirmAccountLogin();
                        }
                        currentPage.closeMenu();
                    } else {
                        scene.accountItem.setRequestedPresence(TelepathyTypes.ConnectionPresenceTypeOffline,
                                                      "offline", // i18n ok
                                                      scene.accountItem.data(AccountsModel.CurrentPresenceMessageRole));
                        currentPage.closeMenu();
                        scene.previousApplicationPage();
                    }
                }
            }
        }
    }

    AccountHelper {
        id: accountHelper
    }

    Ux.PhotoPicker {
        id: photoPicker

        onPhotoSelected: {
            accountHelper.setAccount(scene.accountItem);
            accountHelper.avatar = uri;
            accountFactory.avatarSerial++;
            avatarImage.source = accountHelper.avatar;
        }
    }
}
