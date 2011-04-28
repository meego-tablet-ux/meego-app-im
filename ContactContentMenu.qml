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
        confirmationDialogItem.title = qsTr("Multiple accounts connected");
        confirmationDialogItem.text = qsTr("Do you really want to connect this account? By doing this all other %1 accounts will be disconnected.").arg(serviceName);
        confirmationDialogItem.instanceReason = "contact-menu-single-instance"; // i18n ok
        confirmationDialogItem.show();
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

    width: 200
    height: optionColumn.height

    Column {
        id: optionColumn
        anchors.right: parent.right
        anchors.left: parent.left

        Avatar {
            id: avatarImage
            anchors.leftMargin: 10
            anchors.left: parent.left
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 2
            height: width
            anchors.bottomMargin: 2
            source: "image://avatars/" + scene.accountItem.data(AccountsModel.IdRole) + // i18n ok
                    "?" + accountFactory.avatarSerial
            noAvatarImage: "image://meegotheme/widgets/common/avatar/avatar-default"

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
        }

        Item {
            id: avatarSeparator
            height: 5
            anchors.left: parent.left
            anchors.right: parent.right
        }

        Row {
            id: statusRow
            spacing: 5
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10

            Text {
                id: displayName
                width: parent.width - presenceIcon.width - 5
                text: scene.accountItem.data(AccountsModel.NicknameRole)
                color: nameColor
                font.pixelSize: theme_fontPixelSizeSmall
                elide: Text.ElideRight
            }

            PresenceIcon {
                id: presenceIcon
                anchors.verticalCenter: displayName.verticalCenter
                status: scene.accountItem.data(AccountsModel.CurrentPresenceTypeRole)
            }
        }

        Text {
            id: statusMessage
            text: ""
            color: theme_contextMenuFontColor
            font.pixelSize: theme_fontPixelSizeSmall
            width: parent.width - presenceIcon.width
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
        }

        MenuItemSeparator { id: statusSeparator }

        MenuItem {
            id: meUpdateStatusItem
            text: qsTr("Update status")

            onClicked: {
                if(statusMenu.visible == false) {
                    statusMenu.opacity = 1;
                    customMessageBox.focus = true;
                    avatarImage.visible = false;
                    avatarSeparator.visible = false;
                    statusRow.visible = false;
                    statusMessage.visible = false;
                    statusSeparator.visible = false;
                    updateStatusSeparator.visible = false;
                    updateNickItem.visible = false;
                    updateNick.visible = false;
                    nicknameSeparator.visible = false;
                    addIMContactItem.visible = false;
                    addAFriend.visible = false;
                    friendSeparator.visible = false;
                    clearHistoryItem.visible = false;
                    historySeparator.visible = false;
                    logOutItem.visible = false;
                } else {
                    statusMenu.opacity = 0;
                    avatarImage.visible = true;
                    avatarSeparator.visible = true;
                    statusRow.visible = true;
                    statusMessage.visible = true;
                    statusSeparator.visible = true;
                    updateStatusSeparator.visible = true;
                    updateNickItem.visible = true;
                    updateNick.visible = true;
                    nicknameSeparator.visible = true;
                    addIMContactItem.visible = true;
                    addAFriend.visible = true;
                    friendSeparator.visible = true;
                    clearHistoryItem.visible = true;
                    historySeparator.visible = true;
                    logOutItem.visible = true;
                }
            }
        }

        Item {
            id: meUpdateStatus
            height: statusMenu.visible ? childrenRect.height + 2 * statusMenu.anchors.topMargin : 0
            width: parent.width

            Column {
                id: statusMenu
                anchors.top: parent.top
                anchors.topMargin: 10
                anchors.right: parent.right
                anchors.leftMargin: 10
                anchors.left: parent.left
                visible: opacity > 0
                opacity: 0
                spacing: 5

                Behavior on opacity {
                    NumberAnimation {
                        duration:  250
                    }
                }

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

                Repeater {
                    id: statusView
                    model: statusModel
                    anchors.left: parent.left
                    anchors.right: parent.right

                    delegate: Component {
                        Item {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: childrenRect.height

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

                TextEntry {
                    id: customMessageBox
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 15
                    defaultText: qsTr("Custom status message");
                    text: scene.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole)
                }

                Button {
                    id: updateStatusButton
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    text: qsTr("Update")
                    textColor: theme_buttonFontColor
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

        MenuItemSeparator { id: updateStatusSeparator }

        MenuItem {
            id: updateNickItem
            text: qsTr("Update user display name")

            onClicked: {
                if (nicknameColumn.visible) {
                    nicknameColumn.opacity = 0;
                } else {
                    nicknameColumn.opacity = 1;
                }
            }
        }

        Item {
            id: updateNick
            height: nicknameColumn.visible ? childrenRect.height + 2 * nicknameColumn.spacing : 0
            width: parent.width

            Timer {
                id: nicknameHideTimer
                interval: 1000
                repeat: false

                onTriggered: nicknameColumn.opacity = 0
            }

            Column {
                id: nicknameColumn
                anchors.top: parent.top
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
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    text: qsTr("Update")
                    textColor: theme_buttonFontColor
                    bgSourceUp: "image://meegotheme/widgets/common/button/button-default"
                    bgSourceDn: "image://meegotheme/widgets/common/button/button-default-pressed"
                    onClicked: {
                        nicknameBox.updateNickname();
                    }
                }
            }
        }

        MenuItemSeparator { id: nicknameSeparator }

        MenuItem {
            id: addIMContactItem
            text: qsTr("Add a friend")
            visible: scene.accountItem.data(AccountsModel.ConnectionStatusRole) == TelepathyTypes.ConnectionStatusConnected

            onClicked: {
                if(addAFriend.opacity == 1) {
                    addAFriend.opacity = 0;
                    addAFriend.resetHelper();
                } else {
                    addAFriend.opacity = 1;
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

        MenuItemSeparator { id: friendSeparator }

        MenuItem {
            id: clearHistoryItem
            text: qsTr("Clear chat history")

            onClicked: {
                accountsModel.clearAccountHistory(scene.currentAccountId);
            }
        }

        MenuItemSeparator { id: historySeparator }

        MenuItem {
            id: logOutItem
            text: (scene.accountItem.data(AccountsModel.ConnectionStatusRole) == TelepathyTypes.ConnectionStatusDisconnected?
                       qsTr("Log in") : qsTr("Log out"))

            onClicked: {
                if(scene.accountItem.data(AccountsModel.ConnectionStatusRole) == TelepathyTypes.ConnectionStatusDisconnected) {
                    contactsScreenPage.requestedStatusType = TelepathyTypes.ConnectionPresenceTypeAvailable;
                    contactsScreenPage.requestedStatus = "available"; // i18n ok
                    contactsScreenPage.requestedStatusMessage = scene.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole);

                    var icon = scene.accountItem.data(AccountsModel.IconRole);
                    var id = scene.accountItem.data(AccountsModel.IdRole);

                    if (!protocolsModel.isSingleInstance(icon) ||
                            accountFactory.otherAccountsOnline(icon, id) == 0) {
                        scene.accountItem.setRequestedPresence(contactsScreenPage.requestedStatusType, contactsScreenPage.requestedStatus, customMessageBox.text);
                        scene.accountItem.setAutomaticPresence(contactsScreenPage.requestedStatusType, contactsScreenPage.requestedStatus, customMessageBox.text);
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

    AccountHelper {
        id: accountHelper
    }

    PhotoPicker {
        id: photoPicker

        onPhotoSelected: {
            accountHelper.setAccount(scene.accountItem);
            accountHelper.avatar = uri;
            accountFactory.avatarSerial++;
            avatarImage.source = accountHelper.avatar;
        }
    }
}
