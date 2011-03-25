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
        component += "import \"" + protocolsModel.modulePath + "\";";
        component += "Component {";
        component += "    id: accountContentComponent;";
        component += "    Column {";
        component += "        id: details;";
        component += "        anchors.top: parent.top;";
        component += "        anchors.left: parent.left;";
        component += "        anchors.right: parent.right;";
        component += "        height: childrenRect.height;";
        component += accountContentFactory.contentForType(type) + "{";
        component += "            id: accountContent;";
        component += "            accountId: \"" + accountId + "\" ;";
        component += "        }";
        component += "        AccountSetupBar { id: accountSetupBar; }";
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
}
