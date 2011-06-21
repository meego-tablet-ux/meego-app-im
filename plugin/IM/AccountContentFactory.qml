/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.App.IM 0.1
import TelepathyQML 0.1
import "imconstants.js" as IMConstants

Item {
    id: accountContentFactory

    property int avatarSerial: 0


    function contentForType(type) {
        return protocolsModel.contentForId(type)
    }

    function componentForNewAccount(type, parent)
    {
        var component = "import Qt 4.7; "; // i18n ok
        component += "import \"" + protocolsModel.modulePath + "\";";
        component += "Component { id: accountComponent;"
        component += "AccountContentDialog { id: accountContentDialog;"
        component += "accountContent: " + accountContentFactory.contentForType(type) + "{";
        component += " id: accountItem;";
        component += "} } }";

        return Qt.createQmlObject(component, parent);
    }

    function componentForAccount(accountId, parent)
    {
        // do not i18n any string in this function
        var item = accountsModel.accountItemForId(accountId);
        var type = item.data(AccountsModel.IconRole);
        var component = "import Qt 4.7; "; // i18n ok
        component += "import \"" + protocolsModel.modulePath + "\";";
        component += "Component { id: accountComponent;"
        component += "AccountContentDialog { id: accountContentDialog;"
        component += "accountContent: " + accountContentFactory.contentForType(type) + "{";
        component += " id: accountContentItem;";
        component += " accountId: \"" + accountId + "\" ;";
        component += "} } }";

        return Qt.createQmlObject(component, parent);
    }

    function embeddedAccountContent(accountId, parent)
    {
        // do not i18n any string in this function
        var item = accountsModel.accountItemForId(accountId);
        var type = item.data(AccountsModel.IconRole);
        var component = "import Qt 4.7; "; // i18n ok
        component += "import MeeGo.Ux.Components.Common 0.1;";
        component += "import \"" + protocolsModel.modulePath + "\";";
        component += "Component {";
        component += "    id: accountContentComponent;";
        component += "    Column {";
        component += "        id: details;";
        component += "        anchors.top: parent.top;";
        component += "        anchors.left: parent.left;";
        component += "        anchors.right: parent.right;";
        component += "        anchors.topMargin: 10;";
        component += "        anchors.bottomMargin: 10;";
        component += "        height: childrenRect.height + 20;";
        component += "        property alias accountContent: accountContent;";
        component += accountContentFactory.contentForType(type) + "{";
        component += "            id: accountContent;";
        //component += "            accountId: \"" + accountId + "\" ;";
        component += "        }";
        component += "        AccountSetupBar { id: accountSetupBar; }";
        component += "}    }";

        return Qt.createQmlObject(component, parent);
    }

    function embeddedNewAccountContent(type, parent)
    {
        // do not i18n any string in this function
        var component = "import Qt 4.7; "; // i18n ok
        component += "import MeeGo.Ux.Components.Common 0.1;";
        component += "import \"" + protocolsModel.modulePath + "\";";
        component += "Component {";
        component += "    id: accountContentComponent;";
        component += "    Column {";
        component += "        id: details;";
        component += "        anchors.top: parent.top;";
        component += "        anchors.left: parent.left;";
        component += "        anchors.right: parent.right;";
        component += "        anchors.topMargin: 10;";
        component += "        anchors.bottomMargin: 10;";
        component += "        height: childrenRect.height + 20;";
        component += "        property alias accountContent: accountContent;";
        component += accountContentFactory.contentForType(type) + "{";
        component += "            id: accountContent;";
        component += "            onAccountCreationAborted: { signInButton.enabled = true; }";
        component += "            onFinished: window.popPage();";
        component += "            Button {";
        component += "                id: signInButton;";
        component += "                text: \"" + IMConstants.accountFactoryAddAccount + "\";"
        component += "                textColor: theme_buttonFontColor;";
        component += "                bgSourceUp: \"image://themedimage/widgets/common/button/button-default\";";
        component += "                bgSourceDn: \"image://themedimage/widgets/common/button/button-default-pressed\";";
        component += "                onClicked: {";
        component += "                    enabled = false;";
        component += "                    accountContent.createAccount();";
        component += "                }";
        component += "            }";
        component += "        }";
        component += "}    }";

        return Qt.createQmlObject(component, parent);
    }

    function accountIcon(type, status)
    {
        var icon = protocolsModel.iconForId(type);

        if (status == TelepathyTypes.ConnectionStatusDisconnected)
            icon += "-offline"; // i18n ok

        return icon;
    }

    function accountServiceName(type)
    {
        return protocolsModel.titleForId(type)
    }

    function otherAccountsOnline(type, accountId) {
        var ids = accountsModel.accountIdsOfType(type);
        var count = 0;

        for (var i in ids) {
            if (ids[i] == accountId) {
                continue;
            }

            var item = accountsModel.accountItemForId(ids[i]);
            var status = item.data(AccountsModel.ConnectionStatusRole);
            if (status != TelepathyTypes.ConnectionStatusDisconnected) {
                count++;
            }
        }

        return count;
    }

    function disconnectOtherAccounts(type, accountId) {
        var ids = accountsModel.accountIdsOfType(type);
        var count = 0;

        for (var i in ids) {
            if (ids[i] == accountId) {
                continue;
            }

            var item = accountsModel.accountItemForId(ids[i]);
            var status = item.data(AccountsModel.ConnectionStatusRole);
            if (status != TelepathyTypes.ConnectionStatusDisconnected) {
                item.setRequestedPresence(TelepathyTypes.ConnectionPresenceTypeOffline,
                                          "offline", // i18n ok
                                          item.data(AccountsModel.ConnectionStatusRole));
            }
        }
    }
}
