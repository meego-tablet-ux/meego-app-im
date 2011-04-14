/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Labs.Components 0.1 as Labs
import MeeGo.App.IM 0.1
import TelepathyQML 0.1

Labs.Window {
    id: scene

    // FIXME remove once migration to Meego UX components is completed
    signal orientationChangeFinished();

    title: qsTr("Chat")
    fullscreen: true

    property int animationDuration: 250

    property string currentAccountId: ""
    property int    currentAccountStatus: -1 // will get filled when accountItem gets filled
    property string currentAccountName: "" // will get filled when accountItem gets filled
    property variant accountItem
    property variant contactItem
    property variant callAgent
    property variant fileTransferAgent
    property variant chatAgent
    property variant incomingContactItem
    property variant incomingCallAgent
    property variant currentPage
    property string cmdCommand: ""
    property string cmdAccountId: ""
    property string cmdContactId: ""
    property variant accountFilterModel: [ ]

    // this property will be set right before opening the conversation screen
    // TODO: check how can we do that on group chat
    property string currentContactId: ""

    Labs.FuzzyDateTime {
        id: fuzzyDateTime
    }

    Timer {
        id: fuzzyDateTimeUpdater
        interval: /*1 */ 60 * 1000 // 1 min
        repeat: true
        running: true
    }

    onCurrentAccountIdChanged: {
        contactsModel.filterByAccountId(currentAccountId);
        contactRequestModel.filterByAccountId(currentAccountId);
        accountItem = accountsModel.accountItemForId(scene.currentAccountId);
        currentAccountStatus = accountItem.data(AccountsModel.CurrentPresenceTypeRole);
        currentAccountName = accountItem.data(AccountsModel.DisplayNameRole);
        notificationManager.currentAccount = currentAccountId;
        accountItemConnections.target = accountItem;
    }

    onCurrentContactIdChanged: {
        notificationManager.currentContact = currentContactId;
    }

    onForegroundChanged: {
        notificationManager.applicationActive = foreground;
    }

    Component.onCompleted: {
        notificationManager.applicationActive = foreground;
    }

    Connections {
        target:  null;
        id: accountItemConnections

        onChanged: {
            currentAccountStatus = accountItem.data(AccountsModel.CurrentPresenceTypeRole);
            currentAccountName = accountItem.data(AccountsModel.DisplayNameRole);
            notificationManager.currentAccount = currentAccountId;
        }
    }

    Connections {
        // this signal is not created yet, so wait till it is, then the target will be set
        // and the connection will be made then.
        target: null;
        id: contactsModelConnections;
        onOpenLastUsedAccount: {
            if(cmdCommand == "") {
                currentAccountId = accountId;
                accountItem = accountsModel.accountItemForId(scene.currentAccountId);
                currentAccountId = accountItem.data(AccountsModel.IdRole);
                addApplicationPage(contactsScreenContent);
            } else {
                if(cmdCommand == "show-chat" || cmdCommand == "show-contacts") {
                    currentAccountId = cmdAccountId;
                    accountItem = accountsModel.accountItemForId(currentAccountId);

                    if(cmdCommand == "show-chat") {
                        currentContactId = cmdContactId;
                        contactItem = accountsModel.contactItemForId(currentAccountId, currentContactId);
                        startConversation(currentContactId);
                    } else if(cmdCommand == "show-contacts") {
                        addApplicationPage(contactsScreenContent);
                    }
                }
            }
        }
    }

    // this call will parse and store the parameters for later use
    // with the onLastUsedAccount, which is triggered when all models are loaded.
    // That way we also make sure that the lastUsedAccount call does not override
    // the command line parameters
    Connections {
        target: mainWindow
        onCall: {

            notificationManager.applicationActive = true;

            var cmd = parameters[0];
            var cdata = parameters[1];

            if(cmd == "show-chat" || cmd == "show-contacts") {
                cmdCommand = cmd;
                var parsedParameter = cdata.substr(0, cdata.length -1);
                cmdAccountId = parsedParameter.substr(0, indexOf("&") - 1);

                //message type
                if (cmd == "show-chat")	{
                    //also get the contact id to open a chat with
                    parsedParameter = parsedParameter.substr(parsedParameter.index("&") + 1, parsedParameter.length - 1);
                    cmdContactId = parsedParameter.substr(0, parsedParameter.indexOf("&") - 1);
                }
            }
        }
    }

    Connections {
        // the onComponentsLoaded signal is not created yet, but we can't use the same trick as the other
        // signals to connect them later since this is the signal that gets used to connect the others.
        // We then choose to ignore the runtime error for just this signal.
        target: accountsModel
        ignoreUnknownSignals: true
        onComponentsLoaded: {
            // the other signals should now be set up, so plug them in
            console.log("Setting up accountsModel and contactsModel connections");
            contactsModelConnections.target = contactsModel;
            accountsModelConnections.target = accountsModel;

            telepathyManager.registerClients();
            reloadFilterModel();
        }
    }

    Connections {
        // those signals are not created yet, so wait till they are, then the target will be set
        // and the connections will be made then.
        target: null
        id: accountsModelConnections

        onAccountConnectionStatusChanged: {
            var item = accountsModel.accountItemForId(accountId);
            if(accountId == currentAccountId) {
                contactsModel.filterByAccountId(currentAccountId);
                contactRequestModel.filterByAccountId(currentAccountId);
                accountItem = item;
                currentAccountStatus = accountItem.data(AccountsModel.CurrentPresenceTypeRole);
            }

            // check if there is a connection error and show the config dialog
            var connectionStatus = item.data(AccountsModel.ConnectionStatusRole)
            var connectionStatusReason = item.data(AccountsModel.ConnectionStatusReasonRole)

            if ((connectionStatus == TelepathyTypes.ConnectionStatusDisconnected) &&
                ((connectionStatusReason == TelepathyTypes.ConnectionStatusReasonAuthenticationFailed) ||
                 (connectionStatusReason == TelepathyTypes.ConnectionStatusReasonNameInUse))) {
                scene.addApplicationPage(accountFactory.componentForAccount(accountId, scene));
            }

        }

        onIncomingFileTransferAvailable: {
            if (!notificationManager.chatActive) {
                var oldContactId = currentContactId;
                var oldAccountId = currentAccountId;

                // set the current contact property
                currentContactId = contactId;
                currentAccountId = accountId;
                contactItem = accountsModel.contactItemForId(accountId, contactId);
                callAgent = accountsModel.callAgent(accountId, contactId);
                fileTransferAgent = accountsModel.fileTransferAgent(scene.currentAccountId, contactId);
                accountsModel.startChat(scene.currentAccountId, contactId);
                chatAgent = accountsModel.chatAgentByKey(scene.currentAccountId, contactId);

                scene.addApplicationPage(messageScreenContent);
            }
        }

        onIncomingCallAvailable: {
            scene.incomingContactItem = accountsModel.contactItemForId(accountId, contactId);
            scene.incomingCallAgent = accountsModel.callAgent(accountId, contactId)
            showModalDialog(incomingCallContent);
            dialogLoader.item.accountId = accountId;
            dialogLoader.item.contactId = contactId;
        }

        onRequestedGroupChatCreated: {
            scene.chatAgent = agent;

            scene.currentContactId = "";
            scene.contactItem = undefined;
            scene.callAgent = undefined;

            // and start the conversation
            if (notificationManager.chatActive) {
                scene.previousApplicationPage();
            }
            scene.addApplicationPage(messageScreenContent);
            scene.title = qsTr("Group conversation");
            accountsModel.startGroupChat(scene.currentAccountId, scene.chatAgent.channelPath)
        }

        onPasswordRequestRequired: {
            scene.addApplicationPage(accountFactory.componentForAccount(accountId, scene));
        }

        onDataChanged: {
            reloadFilterModel();
        }
    }

    filterModel: accountFilterModel
    applicationPage: accountScreenContent

    ///When a selection is made in the filter menu, you will get a signal here:
    onFilterTriggered: {
        if(index < accountsSortedModel.length) {
            currentAccountId = accountsSortedModel.dataByRow(index, AccountsModel.IdRole );
            accountItem = accountsModel.accountItemForId(currentAccountId);
            scene.previousApplicationPage();
            scene.addApplicationPage(contactsScreenContent);
        } else {
            // go back 4 levels. This should get you to the accounts list
            // FIXME: use this until we have a way to check how many pages to pop until it's on the accounts list screen
            scene.previousApplicationPage();
            scene.previousApplicationPage();
            scene.previousApplicationPage();
            scene.previousApplicationPage();
        }
    }

    function startConversation(contactId)
    {
        // set the current contact property
        currentContactId = contactId;
        contactItem = accountsModel.contactItemForId(scene.currentAccountId, scene.currentContactId);
        callAgent = accountsModel.callAgent(scene.currentAccountId, contactId);
        fileTransferAgent = accountsModel.fileTransferAgent(scene.currentAccountId, contactId);

        // and start the conversation
        if (notificationManager.chatActive) {
            scene.previousApplicationPage();
        }

        scene.title = qsTr("Chat with %1").arg(scene.contactItem.data(AccountsModel.AliasRole));
        scene.addApplicationPage(messageScreenContent);
        accountsModel.startChat(scene.currentAccountId, contactId);

        chatAgent = accountsModel.chatAgentByKey(scene.currentAccountId, contactId);
    }

    function startGroupConversation(channelPath)
    {
        scene.currentContactId = "";
        scene.contactItem = undefined;
        scene.callAgent = undefined;

        scene.chatAgent = accountsModel.chatAgentByKey(scene.currentAccountId, channelPath);

        // and start the conversation
        if (notificationManager.chatActive) {
            scene.previousApplicationPage();
        }
        scene.title = qsTr("Group conversation");
        scene.addApplicationPage(messageScreenContent);

        accountsModel.startGroupChat(scene.currentAccountId, scene.chatAgent.channelPath)
    }

    function acceptCall(accountId, contactId)
    {
        if (notificationManager.chatActive) {
            scene.previousApplicationPage();
        }

        // set the current contact property
        scene.currentContactId = contactId;
        scene.currentAccountId = accountId;
        scene.contactItem = accountsModel.contactItemForId(accountId, contactId);
        scene.callAgent = accountsModel.callAgent(accountId, contactId);
        scene.fileTransferAgent = accountsModel.fileTransferAgent(scene.currentAccountId, contactId);

        // and start the conversation
        scene.addApplicationPage(messageScreenContent);
        accountsModel.startChat(accountId, contactId);
        chatAgent = accountsModel.chatAgentByKey(accountId, contactId);
    }

    function startAudioCall(contactId, page)
    {
        // set the current contact property
        currentContactId = contactId;
        contactItem = accountsModel.contactItemForId(scene.currentAccountId, scene.currentContactId);

        //create the audio call agent
        //the message screen will then get the already created agent
        callAgent = accountsModel.callAgent(scene.currentAccountId, contactId);
        fileTransferAgent = accountsModel.fileTransferAgent(scene.currentAccountId, contactId);
        accountsModel.startChat(accountId, contactId);
        chatAgent = accountsModel.chatAgentByKey(accountId, contactId);
        callAgent.audioCall();

        // and start the conversation
        if (notificationManager.chatActive) {
            scene.previousApplicationPage();
        }
        page.addApplicationPage(messageScreenContent);
    }

    function startVideoCall(contactId, page)
    {
        // set the current contact property
        currentContactId = contactId;
        contactItem = accountsModel.contactItemForId(scene.currentAccountId, scene.currentContactId);

        //create the audio call agent
        //the message screen will then get the already created agent
        callAgent = accountsModel.callAgent(scene.currentAccountId, contactId);
        fileTransferAgent = accountsModel.fileTransferAgent(scene.currentAccountId, contactId);
        accountsModel.startChat(accountId, contactId);
        chatAgent = accountsModel.chatAgentByKey(accountId, contactId);
        callAgent.videoCall();

        // and start the conversation
        if (notificationManager.chatActive) {
            scene.previousApplicationPage();
        }
        page.addApplicationPage(messageScreenContent);
    }

    function pickContacts(page)
    {
        page.addApplicationPage(contactPickerContent)
    }

    function reloadFilterModel()
    {
        // do not do anything if accountsSortedModel is not created yet
        if (typeof(accountsSortedModel) == 'undefined')
            return;

        accountFilterModel = [];

        var accountsList = new Array();
        for (var i = 0; i < accountsSortedModel.length; ++i) {
            accountsList[i] = accountsSortedModel.dataByRow(i, AccountsModel.DisplayNameRole);
        }
        accountsList[accountsList.length] = qsTr("Account switcher");
        accountFilterModel = accountsList;
    }

    function presenceStatusText(type)
    {
        if(type == TelepathyTypes.ConnectionPresenceTypeAvailable) {
            return qsTr("available");
        } else if(type == TelepathyTypes.ConnectionPresenceTypeBusy) {
            return qsTr("busy");
        } else if(type == TelepathyTypes.ConnectionPresenceTypeAway) {
            return qsTr("away");
        } else if(type == TelepathyTypes.ConnectionPresenceTypeExtendedAway) {
            return qsTr("extended away");
        } else if(type == TelepathyTypes.ConnectionPresenceTypeOffline) {
            return qsTr("offline");
        } else {
            return "";
        }
    }

    Loader {
        id: contextLoader
    }

    AccountContentFactory {
        id: accountFactory
    }

    Component {
        id: contactsScreenContent
        ContactsScreenContent {
            id: contactsScreenItem
            anchors.fill: parent
        }
    }

    Component {
        id: accountScreenContent
        AccountScreenContent {
            id: accountScreenItem
            anchors.fill: parent
        }
    }

    Component {
        id: messageScreenContent
        MessageScreenContent {
            id: messageScreenItem
            anchors.fill: parent
        }
    }

    Component {
        id: incomingCallContent
        IncomingCall {
            id: incomingCallDialog
        }
    }

    Component {
        id: contactPickerContent
        ContactPickerContent {
            id: contactPickerItem
            anchors.fill: parent
        }
    }

    Component {
        id: confirmationDialogContent
        ConfirmationDialog {
            id: confirmationDialogItem
        }
    }

    Loader {
            id: dialogLoader
            width: scene.width
            height: scene.height
    }
}

