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
import TelepathyQML 0.1

Labs.ApplicationPage {
    id: contactsScreenPage
    
    property int count: listView.count
    property bool showLoadingContacts: false
    property int accountStatus: 0
    property bool showAccountOffline: (accountStatus == TelepathyTypes.ConnectionPresenceTypeOffline
                                       || accountStatus == TelepathyTypes.ConnectionPresenceTypeUnset
                                       || accountStatus == TelepathyTypes.ConnectionPresenceTypeUnknown
                                       || accountStatus == TelepathyTypes.ConnectionPresenceTypeError)


    property int requestedStatusType: 0
    property string requestedStatus: ""
    property string requestedStatusMessage: ""

    // this should be commented unless you are testing the search functionality
    // showSearch: true

    onSearch: {
        contactsModel.filterByString(needle);
    }

    anchors.fill: parent

    Component.onCompleted: {
        scene.title = scene.currentAccountName;
        accountStatus = scene.accountItem.data(AccountsModel.CurrentPresenceTypeRole);
    }

    onAccountStatusChanged: {
        contactsModel.filterByAccountId(currentAccountId);
        contactRequestModel.filterByAccountId(currentAccountId);
    }

    Connections {
        target: telepathyManager
        onUpgradingContacts: {
            showLoadingContacts = true;
        }

        onContactsUpgraded: {
            showLoadingContacts = false;
        }
    }

    Connections {
        target: scene.accountItem

        onChanged: {
            accountStatus = scene.accountItem.data(AccountsModel.CurrentPresenceTypeRole);
        }
    }

    Connections {
        target: scene

        onCurrentAccountNameChanged: {
            scene.title = scene.currentAccountName;
        }
    }

    // this connection is to handle
    Connections {
        target: dialogLoader.item
        onAccepted: {

            var icon = scene.accountItem.data(AccountsModel.IconRole);

            if (dialogLoader.item.instanceReason != "contact-menu-single-instance") {
                return;
            }

            // if the dialog was accepted we should disconnect all other accounts
            // of the same type
            accountFactory.disconnectOtherAccounts(icon, scene.currentAccountId);

            // and set the account online
            scene.accountItem.setRequestedPresence(requestedStatusType, requestedStatus, requestedStatusMessage);
            scene.accountItem.setAutomaticPresence(requestedStatusType, requestedStatus, requestedStatusMessage);
        }

        // no need to do anything if the dialog is rejected
        // onRejected:
    }

    Item {
        id: pageContent
        parent: contactsScreenPage.content
        anchors.fill: parent

        NoNetworkHeader {
            id: noNetworkItem
        }

        AccountOffline {
            id: accountOfflineInfo

            anchors {
                top: noNetworkItem.bottom
                left: parent.left
                right: parent.right
            }
            visible: showAccountOffline
        }

        ContactsLoading {
            id: contactsLoading
            anchors {
                top: accountOfflineInfo.bottom
                left: parent.left
                right: parent.right
            }
            visible: showLoadingContacts
        }

        Component {
            id: requestsViewComponent

            ListView {
                id: requestsView
                interactive: false
                property int itemHeight: 55;

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
                top: contactsLoading.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            visible: (!showAccountOffline)

            model: contactsModel
            delegate: ContactsDelegate {}
            clip: true

            header: requestsViewComponent
            interactive: contentHeight > height
        }
    }

    menuContent: ContactContentMenu {
        currentPage: contactsScreenPage;
    }
    menuWidth: 400

}
