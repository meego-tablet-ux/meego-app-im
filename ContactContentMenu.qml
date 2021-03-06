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
import MeeGo.Media 0.1
import "constants.js" as Constants

Column {
    id: optionColumn

    width: 350

    property variant currentPage
    property QtObject accountHelper : null
    property string avatarSerial : ""

    signal accountChanged

    Component.onCompleted: {
        if (window.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole) != "") {
            statusMessage.text = window.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole);
        } else {
            statusMessage.text = window.presenceStatusText(window.accountItem.data(AccountsModel.CurrentPresenceTypeRole));
        }
        avatarSerial = accountFactory.avatarSerial;
    }

    onVisibleChanged: {
        if(!visible) {
            resetMenu();
        }
    }

    onAvatarSerialChanged: {
        avatarImage.source = "image://avatars/" + window.accountItem.data(AccountsModel.IdRole) + // i18n ok
                        "?" + avatarSerial;
    }

    Connections {
        target: window.accountItem
        // a small trick
        onChanged: {
            window.accountItem = window.accountItem
            if (window.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole) != "") {
                statusMessage.text = window.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole);
            } else {
                statusMessage.text = window.presenceStatusText(window.accountItem.data(AccountsModel.CurrentPresenceTypeRole));
            }
            displayName.text = window.accountItem.data(AccountsModel.NicknameRole);
            presenceIcon.status = window.accountItem.data(AccountsModel.CurrentPresenceTypeRole);
            avatarSerial = accountFactory.avatarSerial;
        }
    }

    Connections {
        target: window
        onCurrentAccountIdChanged: {
            window.accountItem = accountsModel.accountItemForId(window.currentAccountId);
        }
    }

    Connections {
        target: avatarPhotoPicker

        onPhotoSelected: {
            createAccountHelper();
            accountHelper.setAccount(window.accountItem);
            accountHelper.avatar = uri;
        }
    }

    Connections {
        target: accountHelper

        onAvatarChanged: {
            accountFactory.avatarSerial++;
            avatarImage.source = accountHelper.avatar;
            avatarSerial = accountFactory.avatarSerial;
        }
    }

    Avatar {
        id: avatarImage
        anchors.left: parent.left
        anchors.leftMargin: 15
        width: 180
        height: width
        anchors.bottomMargin: 2
        source: "image://avatars/" + window.accountItem.data(AccountsModel.IdRole) + // i18n ok
                "?" + avatarSerial
        noAvatarImage: "image://themedimage/widgets/common/avatar/avatar-default"

        Component {
            id: avatarMenu
            PictureChangeMenu {
                id: pictureChangeMenu
                onClose: {
                    currentPage.hideActionMenu();
                }
            }
        }

        MouseArea {
            id: avatarMouseArea
            anchors.fill: avatarImage

            onClicked: {
                createAvatarPhotoPicker();
                avatarPhotoPicker.show();
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
        anchors.margins: 15

        Text {
            id: displayName
            width: parent.width - presenceIcon.width - 10
            text: window.accountItem.data(AccountsModel.NicknameRole)
            color: theme_fontColorNormal
            font.weight: Font.Bold
            font.pixelSize: theme_fontPixelSizeNormal
            elide: Text.ElideRight
        }

        PresenceIcon {
            id: presenceIcon
            anchors.verticalCenter: displayName.verticalCenter
            status: window.accountItem.data(AccountsModel.CurrentPresenceTypeRole)
        }
    }

    Text {
        id: statusMessage
        text: ""
        color: theme_fontColorInactive
        font.pixelSize: theme_fontPixelSizeNormal
        width: parent.width - presenceIcon.width
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 15
    }

    Item {
        id: statusMessageSeparator
        height: 15
        anchors.left: parent.left
        anchors.right: parent.right
    }

    MenuItemSeparator { id: statusSeparator }

    MenuItem {
        id: updateStatusItem
        text: Constants.contactUpdateStatus

        onClicked: {
            statusRadioGroup.select(window.accountItem.data(AccountsModel.CurrentPresenceTypeRole));
            if(statusMenu.visible == false) {
                statusMenu.opacity = 1;
                customMessageBox.focus = true;
                avatarImage.visible = false;
                avatarSeparator.visible = false;
                statusRow.visible = false;
                statusMessage.visible = false;
                statusMessageSeparator.visible = false;
                statusSeparator.visible = false;
                updateStatusItem.visible = false;
                updateStatusSeparator.visible = false;
                updateNickItem.showUpdateNick = false;
                updateNick.visible = false;
                nicknameSeparator.showUpdateNick = false;
                addIMContactItem.showAddFriend = false;
                addAFriend.visible = false;
                friendSeparator.showAddFriend = false;
                clearHistoryItem.visible = false;
                historySeparator.visible = false;
                logOutItem.visible = false;
                customMessageBox.textInput.forceActiveFocus();
            }
        }
    }

    Item {
        id: updateStatus
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

            RadioGroup {
                id: statusRadioGroup
            }

            property string statusString: ""

            Text {
                text: Constants.contactYourStatus
                color: theme_fontColorNormal
                font.pixelSize: theme_contextMenuFontPixelSize
                elide: Text.ElideRight

            }

            Repeater {
                id: statusView
                model: StatusModel {
                           id: statusModel
                       }

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
                                var icon = window.accountItem.data(AccountsModel.IconRole);
                                var id = window.accountItem.data(AccountsModel.IdRole);
                                // if the protocol doesn t allow for multiple accounts to be online
                                // at the same time, we need to ask the user if he wants to disconnect
                                // the other accounts
                                if (!protocolsModel.isSingleInstance(icon) ||
                                        accountFactory.otherAccountsOnline(icon, id) == 0 ||
                                        model.type == TelepathyTypes.ConnectionPresenceTypeOffline) {
                                    window.accountItem.setRequestedPresence(model.type, model.status, customMessageBox.text);
                                    window.accountItem.setAutomaticPresence(model.type, model.status, customMessageBox.text);
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
                                height: statusText.height
                                width: height
                            }

                            Text {
                                id: statusText
                                anchors.verticalCenter: statusRadioButton.verticalCenter
                                text: model.text
                                font.pixelSize: theme_contextMenuFontPixelSize
                                color: theme_contextMenuFontColor
                            }
                        }
                    }
                }

                onVisibleChanged: {
                    if (visible) {
                        var n, count = statusModel.count;
                        for (n=0;n<count;++n) {
                            console.log("Checking presence "+statusModel.get(n).type);
                            if (!window.accountItem.isPresenceSupported(statusModel.get(n).type)) {
                                statusModel.remove(n--);
                                --count;
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
                defaultText: Constants.contactCustomStatusMessage
                text: window.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole)
                onAccepted: {
                    customMessageBox.updateStatus();
                }

                function updateStatus()
                {
                    var status;
                    for(var i = 0; i < statusModel.count; ++i) {
                        if (statusRadioGroup.selectedValue == statusModel.get(i).type) {
                            status = statusModel.get(i).text;
                        }
                    }

                    window.accountItem.setRequestedPresence(statusRadioGroup.selectedValue, status, customMessageBox.text);
                    window.accountItem.setAutomaticPresence(statusRadioGroup.selectedValue, status, customMessageBox.text);
                    currentPage.hideActionMenu();
                }
            }

            Button {
                id: updateStatusButton
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: Constants.contactStatusUpdate
                textColor: theme_buttonFontColor
                bgSourceUp: "image://themedimage/widgets/common/button/button-default"
                bgSourceDn: "image://themedimage/widgets/common/button/button-default-pressed"
                onClicked: {
                    customMessageBox.updateStatus();
                }
            }
        }
    }

    MenuItemSeparator { id: updateStatusSeparator }

    MenuItem {
        id: updateNickItem
        text: Constants.contactChangeDisplayName
        visible: (showUpdateNick && window.currentAccountStatus == TelepathyTypes.ConnectionStatusConnected)

        property bool showUpdateNick: true

        onClicked: {
            if (nicknameColumn.visible) {
                updateNickItem.hideUpdateNick();
            } else {
                nicknameColumn.opacity = 1;
                avatarImage.visible = false;
                avatarSeparator.visible = false;
                statusRow.visible = false;
                statusMessage.visible = false;
                statusMessageSeparator.visible = false;
                statusSeparator.visible = false;
                updateStatusItem.visible = false;
                updateStatusSeparator.visible = false;
                nicknameSeparator.showUpdateNick = false;
                addIMContactItem.showAddFriend = false;
                addAFriend.visible = false;
                friendSeparator.showAddFriend = false;
                clearHistoryItem.visible = false;
                historySeparator.visible = false;
                logOutItem.visible = false;
                nicknameBox.textInput.forceActiveFocus();
            }
        }

        function hideUpdateNick()
        {
            nicknameColumn.opacity = 0;
            avatarImage.visible = true;
            avatarSeparator.visible = true;
            statusRow.visible = true;
            statusMessage.visible = true;
            statusMessageSeparator.visible = true;
            statusSeparator.visible = true;
            updateStatusItem.visible = true;
            updateStatusSeparator.visible = true;
            nicknameSeparator.showUpdateNick = true;
            addIMContactItem.showAddFriend = true;
            addAFriend.visible = true;
            friendSeparator.showAddFriend = true;
            clearHistoryItem.visible = true;
            historySeparator.visible = true;
            logOutItem.visible = true;
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

            onTriggered: {
                updateNickItem.hideUpdateNick();
            }
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
                    nicknameBox.text = window.accountItem.data(AccountsModel.NicknameRole)
                }
            }

            TextEntry {
                id: nicknameBox
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.right: parent.right
                anchors.rightMargin: 15
                defaultText: Constants.contactChangeDisplayNameText
                text: window.accountItem.data(AccountsModel.NicknameRole)

                onAccepted: {
                    updateNickname();
                }

                function updateNickname() {
                    if (nicknameBox.text != "") {
                        window.accountItem.setNickname(nicknameBox.text);
                        nicknameHideTimer.start();
                    }
                }
            }

            Button {
                id: updateNicknameButton
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: Constants.contactDisplayNameUpdate
                textColor: theme_buttonFontColor
                bgSourceUp: "image://themedimage/widgets/common/button/button-default"
                bgSourceDn: "image://themedimage/widgets/common/button/button-default-pressed"
                onClicked: {
                    nicknameBox.updateNickname();
                }
            }
        }
    }

    MenuItemSeparator {
        id: nicknameSeparator
        visible: (showUpdateNick && window.currentAccountStatus == TelepathyTypes.ConnectionStatusConnected)

        property bool showUpdateNick: true
    }

    MenuItem {
        id: addIMContactItem
        text: Constants.contactAddFriend
        visible: (showAddFriend && window.currentAccountStatus == TelepathyTypes.ConnectionStatusConnected)

        property bool showAddFriend: true

        onClicked: {
            if(addAFriend.opacity == 1) {
                addAFriend.opacity = 0;
                addAFriend.resetHelper();
                avatarImage.visible = true;
                avatarSeparator.visible = true;
                statusRow.visible = true;
                statusMessage.visible = true;
                statusMessageSeparator.visible = true;
                statusSeparator.visible = true;
                updateStatusItem.visible = true;
                updateStatusSeparator.visible = true;
                updateNick.visible = true;
                updateNickItem.showUpdateNick = true;
                nicknameSeparator.showUpdateNick = true;
                friendSeparator.showAddFriend = true;
                clearHistoryItem.visible = true;
                historySeparator.visible = true;
                logOutItem.visible = true;
            } else {
                addAFriend.resetHelper();
                addAFriend.opacity = 1;
                avatarImage.visible = false;
                avatarSeparator.visible = false;
                statusRow.visible = false;
                statusMessage.visible = false;
                statusMessageSeparator.visible = false;
                statusSeparator.visible = false;
                updateStatusItem.visible = false;
                updateStatusSeparator.visible = false;
                updateNick.visible = false;
                updateNickItem.showUpdateNick = false;
                nicknameSeparator.showUpdateNick = false;
                friendSeparator.showAddFriend = false;
                clearHistoryItem.visible = false;
                historySeparator.visible = false;
                logOutItem.visible = false;
                addAFriend.textInput.textInput.forceActiveFocus();
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

        onStateChanged: {
            if (state == "sent") {
                sentTimer.running = true;
            }
        }

        Timer {
            id: sentTimer

            interval: 3000
            running: false
            onTriggered: {
                currentPage.hideActionMenu();
            }
        }
    }

    MenuItemSeparator {
        id: friendSeparator
        visible: (showAddFriend && window.currentAccountStatus == TelepathyTypes.ConnectionStatusConnected)

        property bool showAddFriend: true
    }

    MenuItem {
        id: clearHistoryItem
        text: Constants.contactClearChatHistory

        onClicked: {
            accountsModel.clearAccountHistory(window.currentAccountId);
            currentPage.hideActionMenu();
        }
    }

    MenuItemSeparator { id: historySeparator }

    MenuItem {
        id: logOutItem
        text: (window.currentAccountStatus == TelepathyTypes.ConnectionStatusDisconnected ?
                   Constants.contactLogin : Constants.contactLogout)

        onClicked: {
            if(window.currentAccountStatus == TelepathyTypes.ConnectionStatusDisconnected) {
                contactsScreenPage.requestedStatusType = TelepathyTypes.ConnectionPresenceTypeAvailable;
                contactsScreenPage.requestedStatus = "available"; // i18n ok
                contactsScreenPage.requestedStatusMessage = window.accountItem.data(AccountsModel.CurrentPresenceStatusMessageRole);

                var icon = window.accountItem.data(AccountsModel.IconRole);
                var id = window.accountItem.data(AccountsModel.IdRole);

                if (!protocolsModel.isSingleInstance(icon) ||
                        accountFactory.otherAccountsOnline(icon, id) == 0) {
                    window.accountItem.setRequestedPresence(contactsScreenPage.requestedStatusType, contactsScreenPage.requestedStatus, customMessageBox.text);
                    window.accountItem.setAutomaticPresence(contactsScreenPage.requestedStatusType, contactsScreenPage.requestedStatus, customMessageBox.text);
                } else {
                    confirmAccountLogin();
                }
                currentPage.hideActionMenu();
            } else {
                window.accountItem.setRequestedPresence(TelepathyTypes.ConnectionPresenceTypeOffline,
                                                       "offline", // i18n ok
                                                       window.accountItem.data(AccountsModel.CurrentPresenceMessageRole));
                currentPage.hideActionMenu();
                window.popPage();
            }
        }
    }


    function createAccountHelper() {
        if (accountHelper == null) {
            console.log("Creating AccountHelper");
            var sourceCode = "import Qt 4.7;"
                           + "import MeeGo.App.IM 0.1;"
                           + "import TelepathyQML 0.1;"
                           + "AccountHelper {}";
            accountHelper = Qt.createQmlObject(sourceCode, optionColumn);
        }
    }

    function confirmAccountLogin()
    {
        var serviceName = protocolsModel.titleForId(window.accountItem.data(AccountsModel.IconRole));

        // show the dialog to ask for user confirmation
        confirmationDialogItem.title = Constants.multipleAccountsTitle
        confirmationDialogItem.text = Constants.multipleAccountsText.arg(serviceName);
        confirmationDialogItem.instanceReason = "contact-menu-single-instance"; // i18n ok
        confirmationDialogItem.accountId = window.currentAccountId;
        confirmationDialogItem.show();
    }

    function resetMenu()
    {
        statusMenu.opacity = 0;
        nicknameColumn.opacity = 0;
        addAFriend.opacity = 0;
        avatarImage.visible = true;
        avatarSeparator.visible = true;
        statusRow.visible = true;
        statusMessage.visible = true;
        statusMessageSeparator.visible = true;
        statusSeparator.visible = true;
        updateStatusItem.visible = true;
        updateStatusSeparator.visible = true;
        updateNick.visible = true;
        updateNickItem.showUpdateNick = true;
        nicknameSeparator.showUpdateNick = true;
        addIMContactItem.showAddFriend = true;
        addAFriend.visible = true;
        friendSeparator.showAddFriend = true;
        clearHistoryItem.visible = true;
        historySeparator.visible = true;
        logOutItem.visible = true;
    }
}
