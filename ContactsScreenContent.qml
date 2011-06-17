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

            model: contactsModel
            delegate: ContactDelegate {}
            header: requestsViewComponent
            clip: true

            interactive: contentHeight > height
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
    }

    function hideActionMenu()
    {
        contactContentMenu.hide();
    }

    function setInfoBarMessage()
    {
        var text;

        if (!networkOnline) {
            text = Constants.noNetworkText;
        } else {
            // check first whether the account is offline or just connecting
            text = accountStatusMessage(accountStatus);
            // if not, check whether contacts are loading
            if (text == "") {
                if (showLoadingContacts) {
                    text = Constants.contactScreenLoading;
                    showAddFriendsItem = false;
                } else if (showAddFriends && !showLoadingContacts) {
                    // check whether the contact list is really empty
                    if (accountsModel.actualContactsCount(window.currentAccountId) == 0) {
                        text = Constants.contactScreenNoFriends;
                        showAddFriendsItem = true;
                    } else {
                        showAddFriendsItem = false;
                    }
                } else {
                    showAddFriendsItem = false;
                }
            } else {
                showAddFriendsItem = false;
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

    function accountStatusMessage(status)
    {
        var connectionStatusReason = window.accountItem.data(AccountsModel.ConnectionStatusReasonRole)
        if (accountStatus == TelepathyTypes.ConnectionStatusDisconnected) {
            switch(connectionStatusReason) {
                case TelepathyTypes.ConnectionStatusReasonNoneSpecified:
                case TelepathyTypes.ConnectionStatusReasonRequested:
                    return Constants.contactScreenAccountOffline;
                case TelepathyTypes.ConnectionStatusReasonNetworkError:
                    return Constants.noNetworkText;
                case TelepathyTypes.ConnectionStatusReasonAuthenticationFailed:
                    return Constants.errorLogin;
                case TelepathyTypes.ConnectionStatusReasonEncryptionError:
                    return Constants.errorEncryptionDeselect;
                case TelepathyTypes.ConnectionStatusReasonNameInUse:
                    return Constants.errorLogoutConnectedElse;
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
                    return Constants.errorSslError;
                default:
                    return Constants.errorLoginTryLater;
            }
        } else if (accountStatus == TelepathyTypes.ConnectionStatusConnecting) {
            return Constants.contactScreenAccountConnecting;
        } else {
            return "";
            }
    }

    ContextMenu {
        id: contactContentMenu

        width: 300
        forceFingerMode: 2

        content: ContactContentMenu {
            currentPage: contactsScreenPage;
        }
    }

    TopItem { id: topItem }
}
