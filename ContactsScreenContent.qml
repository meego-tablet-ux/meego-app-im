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
    id: contactsScreenPage

    anchors.fill: parent
    enableCustomActionMenu: true
    actionMenuOpen: contactContentMenu.visible
    
    property int count: listView.count + contactRequestModel.rowCount
    property int accountStatus: 0
    property int contactListState: 0
    property bool showAccountOffline: (accountStatus == TelepathyTypes.ConnectionStatusDisconnected
                                       || accountStatus == TelepathyTypes.ConnectionStatusConnecting)
    property bool showLoadingContacts: (!showAccountOffline && contactListState != TelepathyTypes.ContactListStateSuccess)
    property bool showAddFriends: !count && !showAccountOffline && !window.showToolBarSearch
                                  && contactListState == TelepathyTypes.ContactListStateSuccess
    property bool showAddFriendsItem: false
    property int requestedStatusType: 0
    property string requestedStatus: ""
    property string requestedStatusMessage: ""

    Component.onCompleted: {
        pageTitle = window.currentAccountName;
        accountStatus = window.accountItem.data(AccountsModel.ConnectionStatusRole);
        contactListState = window.accountItem.data(AccountsModel.ContactListStateRole);
        setInfoBarMessage();
    }

    onAccountStatusChanged: {
        contactsModel.filterByAccountId(currentAccountId);
        contactRequestModel.filterByAccountId(currentAccountId);
        setInfoBarMessage();
    }

    onActionMenuIconClicked: {
        contactContentMenu.setPosition( mouseX, mouseY);
        contactContentMenu.show();
    }

    onShowAccountOfflineChanged: {
        setInfoBarMessage();
    }

    onShowLoadingContactsChanged: {
        setInfoBarMessage();
    }

    onShowAddFriendsChanged: {
        setInfoBarMessage();
    }

    onActivated: {
        window.currentScreen = "contacts"; // i18n ok
    }

    Connections {
        target: window.accountItem

        onChanged: {
            accountStatus = window.accountItem.data(AccountsModel.ConnectionStatusRole);
            contactListState = window.accountItem.data(AccountsModel.ContactListStateRole);
        }
    }

    Connections {
        target: window

        onCurrentAccountNameChanged: {
            pageTitle = window.currentAccountName;
        }
    }

    Connections {
        target: accountsModel

        onNetworkStatusChanged: {
            setInfoBarMessage();
        }
    }

    // this connection is to handle
    Connections {
        target: confirmationDialogItem
        onAccepted: {

            var icon = window.accountItem.data(AccountsModel.IconRole);

            if (confirmationDialogItem.instanceReason != "contact-menu-single-instance") {
                return;
            }

            // if the dialog was accepted we should disconnect all other accounts
            // of the same type
            accountFactory.disconnectOtherAccounts(icon, window.currentAccountId);

            // and set the account online
            window.accountItem.setRequestedPresence(requestedStatusType, requestedStatus, requestedStatusMessage);
            window.accountItem.setAutomaticPresence(requestedStatusType, requestedStatus, requestedStatusMessage);
        }

        // no need to do anything if the dialog is rejected
        // onRejected:
    }

    Item {
        id: pageContent
        anchors.fill: parent

        Image {
            anchors.fill: parent
            source: "image://themedimage/widgets/common/backgrounds/global-background-texture"
        }

        InfoBar {
            id: infoBar

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

        Component {
            id: requestsViewComponent

            ListView {
                id: requestsView
                interactive: false
                property int itemHeight: theme_commonBoxHeight;

                height: itemHeight * count

                anchors {
                    left: parent.left
                    right: parent.right
                }
                visible: listView.visible
                model: contactRequestModel
                delegate: ContactRequestDelegate {
                    itemHeight: requestsView.itemHeight
                }
            }

        }

        ListView {
            id: listView

            anchors {
                top: infoBar.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            visible: (!showAccountOffline)

            cacheBuffer: 600
            model: contactsModel
            delegate: ContactDelegate {}
            header: requestsViewComponent
            clip: true

            interactive: contentHeight > height

            onContentHeightChanged: {
                if (count > 0) {
                    positionViewAtIndex(0, ListView.End);
                }
            }
        }

        Title {
            id: friendsTitle
            anchors.top: infoBar.bottom
            text: Constants.contactScreenAddFriend
            visible: showAddFriendsItem
        }

        AddAFriend {
            id: addAFriendItem

            visible: showAddFriendsItem
            width: 200
            anchors.top: friendsTitle.bottom
            anchors.margins: 10
            anchors.left: parent.left
        }


        Loader {
            id: contextMenuLoader
        }

        Component {
            id: contextMenuComponent
            ContextMenu {
                id: contextMenu

                property string contactId : ""
                property bool chatOpened : false
                property bool textChat : false
                property bool audioCall : false
                property bool videoCall : false
                property bool canBlockContacts : false
                property bool blocked : false
                property bool canReportAbuse : false

                content: Column {
                    id: menuContent

                    height: childrenRect.height
                    width: 200

                    MenuItem {
                        id: textChatItem
                        text: (contextMenu.chatOpened ? Constants.contactReturnToChat :
                                                        Constants.contactOpenChat)
                        visible: contextMenu.textChat
                        onClicked: {
                            window.startConversation(contextMenu.contactId, window);
                            contextMenu.hide();
                        }
                    }

                    MenuItemSeparator { visible: textChatItem.visible }

                    MenuItem {
                        id: callItem
                        text: Constants.contactCall
                        visible: contextMenu.audioCall
                        onClicked: {
                            window.startAudioCall(contextMenu.contactId, window);
                            contextMenu.hide();
                        }
                    }

                    MenuItemSeparator { visible: callItem.visible }

                    MenuItem {
                        id: videoCallItem
                        text: Constants.contactVideoCall
                        visible: contextMenu.videoCall
                        onClicked: {
                            window.startVideoCall(contextMenu.contactId, window);
                            contextMenu.hide();
                        }
                    }

                    MenuItemSeparator { visible: videoCallItem.visible }

                    MenuItem {
                        id: blockItem
                        text: (contextMenu.blocked ? Constants.contactUnblock :
                                               Constants.contactBlock)
                        visible: contextMenu.canBlockContacts
                        onClicked: {
                            if (contextMenu.blocked) {
                            accountsModel.unblockContact(window.currentAccountId, contextMenu.contactId);
                            } else {
                                accountsModel.blockContact(window.currentAccountId, contextMenu.contactId);
                            }
                            contextMenu.hide();
                        }
                    }

                    MenuItemSeparator { visible: blockItem.visible }

                    MenuItem {
                        id: abuseItem
                        text: qsTr("Report abuse")
                        visible: contextMenu.canReportAbuse && !contextMenu.blocked
                        onClicked: {
                            accountsModel.blockContact(window.currentAccountId, contextMenu.contactId, true);
                            contextMenu.hide();
                        }
                    }

                    MenuItemSeparator { visible: abuseItem.visible }

                    MenuItem {
                        id: endChatItem
                        text: Constants.contactEndChat
                        visible: contextMenu.chatOpened
                        onClicked: {
                            accountsModel.endChat(window.currentAccountId, contextMenu.contactId);
                            accountsModel.endCall(window.currentAccountId, contextMenu.contactId);
                            contextMenu.hide();
                        }
                    }

                    MenuItemSeparator { visible: endChatItem.visible }

                    MenuItem {
                        text: Constants.contactDeleteContact
                        onClicked: {
                            accountsModel.removeContact(window.currentAccountId, contextMenu.contactId);
                            contextMenu.hide();
                        }
                    }
                }
            }
        }
    }

    function hideActionMenu()
    {
        contactContentMenu.hide();
    }

    function setInfoBarMessage()
    {
        var text = "";
        var useTimer = true;

        if (!networkOnline) {
            text = Constants.noNetworkText;
        } else  if (accountStatus != TelepathyTypes.ConnectionStatusConnected) {
            // check first whether the account is offline or just connecting
            text = window.accountStatusMessage(accountStatus);
            showAddFriendsItem = false;
        } else if (showLoadingContacts) { // if not, check whether contacts are loading
            text = Constants.contactScreenLoading;
            showAddFriendsItem = false;
            useTimer = false;
        } else if (showAddFriends && !showLoadingContacts) {
            // check whether the contact list is really empty
            if (accountsModel.actualContactsCount(window.currentAccountId) == 0) {
                text = Constants.contactScreenNoFriends;
                showAddFriendsItem = true;
            } else {
                text = Constants.contactScreenLoading;
                showAddFriendsItem = false;
                useTimer = false;
            }
        } else {
            showAddFriendsItem = false;
        }

        // assign and show/hide as necessary
        infoBar.text = text;
        if (text == "") {
            infoBar.hide();
            infoBarTimer.stop();
        } else {
            infoBar.show();
            if (useTimer) {
                infoBarTimer.restart();
            } else {
                infoBarTimer.stop();
            }
        }
    }

    ContextMenu {
        id: contactContentMenu

        width: 350
        forceFingerMode: 2

        content: ContactContentMenu {
            currentPage: contactsScreenPage;
        }
    }

    TopItem { id: topItem }
}
