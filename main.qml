/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Components 0.1
//import MeeGo.Ux.Components.Common 0.1
//import MeeGo.Ux.Components.DateTime 0.1
//import MeeGo.Ux.Components.Media 0.1
//import MeeGo.Ux.Kernel 0.1
import MeeGo.App.IM 0.1
import TelepathyQML 0.1
import "constants.js" as Constants

Window {
    id: window

    toolBarTitle: Constants.mainChat
    fullScreen: true
    customActionMenu: true
    automaticBookSwitching: false
    disableToolBarSearch: false

    property int animationDuration: 250
    property alias mainChatWindow: window
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
    property string cmdGroupChatId: ""
    property variant accountFilterModel: [ ]

    property QtObject contactsScreenContent: null
    property string contactsScreenContentString: ""

    property QtObject messageScreenContent: null
    property string messageScreenContentString: ""

    property QtObject contactPickerContent: null
    property string contactPickerContentString: ""

    // this property will be set right before opening the conversation screen
    property string currentContactId: ""
    property string currentGroupChatId: ""
    property string currentScreen: ""

    property bool restoreStateEnabled: false

    signal componentsLoaded

    Component {
       id: accountScreenContent
        AccountScreenContent {
            id: accountScreenItem
            anchors.fill: parent
        }
    }

    FuzzyDateTime {
        id: fuzzyDateTime
    }

    Timer {
        id: fuzzyDateTimeUpdater
        interval: /*1 */ 60 * 1000 // 1 min
        repeat: true
        running: true
    }

    IMDBus {
        id: dbus

        model: accountsModel

        onChatOpenRequested: {
            console.log("Chat open requested for account " + accountId + " and contact " + contactId);
            currentAccountId = accountId;
            startConversation(contactId);
            window.raise();
        }

        onGroupChatOpenRequested: {
            console.log("Chat open requested for account " + accountId + " and group chat id " + groupChatId);
            currentAccountId = accountId;
            startGroupConversation(groupChatId);
            window.raise();
        }
    }

    onCurrentAccountIdChanged: {
        console.log("onCurrentAccountIdChanged " + currentAccountId);
        contactsModel.filterByAccountId(currentAccountId);
        accountItem = accountsModel.accountItemForId(window.currentAccountId);
        currentAccountStatus = accountItem.data(AccountsModel.ConnectionStatusRole);
        currentAccountName = accountItem.data(AccountsModel.DisplayNameRole);
        notificationManager.currentAccount = currentAccountId;
        accountItemConnections.target = accountItem;
        appState.invalidate();
    }

    onCurrentContactIdChanged: {
        notificationManager.currentContact = currentContactId;
        appState.invalidate();
    }

    onCurrentGroupChatIdChanged: {
        notificationManager.currentGroupChat = currentGroupChatId;
        appState.invalidate();
    }

    onCurrentScreenChanged: {
        appState.invalidate();
    }

    onIsActiveWindowChanged: {
        notificationManager.applicationActive = isActiveWindow;
    }

    Component.onCompleted: {
        buildComponentStrings();
        notificationManager.applicationActive = isActiveWindow;
        showAccountsScreen();
    }

    onBookMenuTriggered: {
        if (index >= 0 && index < bookMenuPayload.length) {
            if(bookMenuPayload[index] != "") {
                showAccountsScreen();
                componentsLoaded();
                showContactsScreen(bookMenuPayload[index]);
            } else {
                showAccountsScreen();
                componentsLoaded();
            }
        }
    }

    onSearch: {
        console.log("searching contacts: " + needle);
        contactsModel.filterByString(needle);
    }

    onSearchRetracted: {
        contactsModel.filterByString("");
    }

    Connections {
        target: mainWindow

        // This could conflic with onOpenLastUsedAccount if the startup
        // is fast enough to actually get this call on start, and not having
        // to parse the commands later
        onCall: {
            parseWindowParameters(parameters);

            // only actually do something if the models are loaded
            // and there is actually a command to execute
            if (cmdCommand.length > 0 && accountsModel != undefined) {
                // first return to the main account list then execute the command line parameters
                showAccountsScreen();
                componentsLoaded();
                openPageByCommand();
            }
        }
    }

    Connections {
        target:  null;
        id: accountItemConnections

        onChanged: {
            currentAccountName = accountItem.data(AccountsModel.DisplayNameRole);
            notificationManager.currentAccount = currentAccountId;
        }

        onConnectionStatusChanged: {
            currentAccountStatus = accountItem.data(AccountsModel.ConnectionStatusRole);

            // as the filter is nullified if the account is disconnected
            // when the account gets connected, call the filter again
            if (status == TelepathyTypes.ConnectionStatusConnected) {
                contactsModel.filterByAccountId(currentAccountId);
            }
        }
    }

    Connections {
        // this signal is not created yet, so wait till it is, then the target will be set
        // and the connection will be made then.
        target: null;
        id: contactsModelConnections;
        onOpenLastUsedAccount: {
            // If there are command line parameters,those take precedence over opening the last
            // used account or save/restore
            parseWindowParameters(mainWindow.call);
            if(cmdCommand == "") {
                // restore the state takes precedence over opening the last used account
                if (appState.restoreRequired) {
                    restoreState();
                } else {
                    // only open last used account if there are no unread messages
                    if (!accountsModel.existingUnreadMessages()
                            && accountId != "") {
                        showContactsScreen(accountId);
                    }
                }
            } else {
                openPageByCommand();
            }
            restoreStateEnabled = true;
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
            contactsModelConnections.target = contactsModel;
            accountsModelConnections.target = accountsModel;

            telepathyManager.registerClients();
            reloadFilterModel();
            componentsLoaded();
        }
    }

    Connections {
        // those signals are not created yet, so wait till they are, then the target will be set
        // and the connections will be made then.
        target: null
        id: accountsModelConnections

        onIncomingFileTransferAvailable: {
            if (!notificationManager.chatActive) {
                var oldContactId = currentContactId;
                var oldAccountId = currentAccountId;

                // set the current contact property
                currentContactId = contactId;
                currentAccountId = accountId;
                contactItem = accountsModel.contactItemForId(accountId, contactId);
                callAgent = accountsModel.callAgent(accountId, contactId);
                fileTransferAgent = accountsModel.fileTransferAgent(window.currentAccountId, contactId);
                accountsModel.startChat(window.currentAccountId, contactId);
                chatAgent = accountsModel.chatAgentByKey(window.currentAccountId, contactId);

                window.showMessageScreen();
            }
        }

        onIncomingCallAvailable: {
            window.incomingCallAgent = accountsModel.callAgent(accountId, contactId);
            window.raise();
            window.acceptCall(accountId, contactId);
        }

        onRequestedGroupChatCreated: {
            window.chatAgent = agent;

            window.currentGroupChatId = agent.channelPath;
            window.currentContactId = "";
            window.contactItem = undefined;
            window.callAgent = undefined;

            window.showMessageScreen();

            accountsModel.startGroupChat(window.currentAccountId, window.chatAgent.channelPath)
        }

        onAcceptCallFinished: {
            window.fileTransferAgent = accountsModel.fileTransferAgent(window.currentAccountId, window.currentContactId);

            window.contactItem = accountsModel.contactItemForId(window.currentAccountId, window.currentContactId);

            // and start the conversation
            window.showMessageScreen();
            accountsModel.startChat(window.currentAccountId, window.currentContactId);
            callAgent = agent;
            chatAgent = accountsModel.chatAgentByKey(window.currentAccountId, window.currentContactId);
        }

        onPasswordRequestRequired: {
            window.addPage(accountFactory.componentForAccount(accountId, window));
        }

        onAccountCountChanged: {
            reloadFilterModel();
        }
    }

    function buildComponentStrings()
    {
        // clean it before cosntructing the string
        var contactsComponent = "import Qt 4.7;";
        contactsComponent += "import MeeGo.Ux.Components.Common 0.1;";
        contactsComponent += "Component {";
        contactsComponent += "    ContactsScreenContent {";
        contactsComponent += "        id: contactsScreenItem;";
        contactsComponent += "        anchors.fill: parent;";
        contactsComponent += "}   }";
        contactsScreenContentString = contactsComponent;

        var messageComponent = "import Qt 4.7;";
        messageComponent += "Component {";
        messageComponent += "    MessageScreenContent {";
        messageComponent += "        id: messageScreenItem;";
        messageComponent += "        anchors.fill: parent;";
        messageComponent += "}   }";
        messageScreenContentString = messageComponent;

        var contactPicker = "import Qt 4.7;";
        contactPicker += "Component {";
        contactPicker += "    ContactPickerContent {";
        contactPicker += "        id: contactPickerItem;";
        contactPicker += "        anchors.fill: parent;";
        contactPicker += "}    }";
        contactPickerContentString = contactPicker;
    }

    function showAccountsScreen()
    {
        console.log("showAccountsScreen");

        if (window.currentScreen == "accounts") { // i18n ok
            console.log("accounts screen page was not added, already in");
            return;
        }

        window.switchBook(accountScreenContent);
    }

    function showContactsScreen(id)
    {
        console.log("showContactsScreen " + id);

        if (window.currentScreen == "contacts") { // i18n ok
            console.log("contacts screen page was not added, already in");
            return;
        }

        if (contactsScreenContent == null) {
            contactsScreenContent = Qt.createQmlObject(contactsScreenContentString, window);
        }

        window.currentAccountId = id;
        addPage(contactsScreenContent);
    }

    function showMessageScreen()
    {
        console.log("showMessageScreen");

        if (messageScreenContent == null) {
            messageScreenContent = Qt.createQmlObject(messageScreenContentString, window);
        }

        // reuse the existing message screen if possible
        var currentPage = window.pageStack.currentPage;
        if (currentPage != null && typeof(currentPage.isMessageScreen)!="undefined" && currentPage.isMessageScreen) {
            window.pageStack.currentPage.initPage();
        } else {
            addPage(messageScreenContent);
        }
    }

    function startConversation(contactId)
    {
        // set the current contact property
        currentContactId = contactId;
        contactItem = accountsModel.contactItemForId(window.currentAccountId, window.currentContactId);
        callAgent = accountsModel.callAgent(window.currentAccountId, contactId);
        fileTransferAgent = accountsModel.fileTransferAgent(window.currentAccountId, contactId);
        currentGroupChatId = "";

        // and start the conversation
        accountsModel.startChat(window.currentAccountId, contactId);
        chatAgent = accountsModel.chatAgentByKey(window.currentAccountId, contactId);
        window.showMessageScreen();
    }

    function startGroupConversation(channelPath)
    {
        console.log("window.startGroupConversation: channelPath=" + channelPath);
        window.currentGroupChatId = channelPath;
        window.currentContactId = "";
        window.contactItem = undefined;
        window.callAgent = undefined;

        window.chatAgent = accountsModel.chatAgentByKey(window.currentAccountId, channelPath);

        // and start the conversation
        window.showMessageScreen();
        accountsModel.startGroupChat(window.currentAccountId, window.chatAgent.channelPath)
        console.log("window.startGroupConversation: finished");
    }

    function acceptCall(accountId, contactId)
    {
        // set the current contact property
        window.callAgent = window.incomingCallAgent
        //window.callAgent.useResourcePolicy = false;
        window.callAgent.acceptCall();

        window.currentContactId = contactId;
        window.currentAccountId = accountId;
        window.contactItem = incomingContactItem;
    }

    function startAudioCall(contactId, page)
    {
        // set the current contact property
        currentContactId = contactId;
        contactItem = accountsModel.contactItemForId(window.currentAccountId, window.currentContactId);

        //create the audio call agent
        //the message screen will then get the already created agent
        callAgent = accountsModel.callAgent(window.currentAccountId, contactId);
        fileTransferAgent = accountsModel.fileTransferAgent(window.currentAccountId, contactId);
        accountsModel.startChat(window.currentAccountId, contactId);
        chatAgent = accountsModel.chatAgentByKey(window.currentAccountId, contactId);
        //callAgent.useResourcePolicy = false;
        callAgent.audioCall();

        // and start the conversation
        window.showMessageScreen();
    }

    function startVideoCall(contactId, page)
    {
        // set the current contact property
        currentContactId = contactId;
        contactItem = accountsModel.contactItemForId(window.currentAccountId, window.currentContactId);

        //create the audio call agent
        //the message screen will then get the already created agent
        callAgent = accountsModel.callAgent(window.currentAccountId, contactId);
        fileTransferAgent = accountsModel.fileTransferAgent(window.currentAccountId, contactId);
        accountsModel.startChat(window.currentAccountId, contactId);
        chatAgent = accountsModel.chatAgentByKey(window.currentAccountId, contactId);
        //callAgent.useResourcePolicy = false;
        callAgent.videoCall();

        // and start the conversation
        window.showMessageScreen();
    }

    function pickContacts()
    {
        if (contactPickerContent == null) {
            contactPickerContent = Qt.createQmlObject(contactPickerContentString, window);
        }

        addPage(contactPickerContent);
    }

    function reloadFilterModel()
    {
        // do not do anything if accountsSortedModel is not created yet

        accountFilterModel = [];
        var accountsList = new Array();
        var payload = new Array();
        if (typeof(accountsSortedModel) != "undefined") {
            var numAccounts = accountsSortedModel.length;
            for (var i = 0; i < numAccounts; ++i) {
                var accName = accountsSortedModel.dataByRow(i, AccountsModel.DisplayNameRole);
                if (accName != "") {
                    accountsList[accountsList.length] = accName;
                    payload[payload.length] = accountsSortedModel.dataByRow(i, AccountsModel.IdRole );
                }
            }
        }
        accountsList[accountsList.length] = Constants.mainAccountSwitcher;
        payload[payload.length] = "";

        accountFilterModel = accountsList;

        setBookMenuData(accountFilterModel, payload);
    }

    function presenceStatusText(type)
    {
        if(type == TelepathyTypes.ConnectionPresenceTypeAvailable) {
            return Constants.mainStatusAvailable;
        } else if(type == TelepathyTypes.ConnectionPresenceTypeBusy) {
            return Constants.mainStatusBusy;
        } else if(type == TelepathyTypes.ConnectionPresenceTypeAway) {
            return Constants.mainStatusAway;
        } else if(type == TelepathyTypes.ConnectionPresenceTypeExtendedAway) {
            return Constants.mainStatusExtendedAway;
        } else if(type == TelepathyTypes.ConnectionPresenceTypeOffline) {
            return Constants.mainStatusOffline;
        } else if(type == TelepathyTypes.ConnectionPresenceTypeHidden) {
            return Constants.mainStatusInvisible;
        } else {
            return "";
        }
    }

    function parseWindowParameters(parameters)
    {
        // only parse if not empty
        if (parameters.length == 0) {
            return;
        }

        var cmd = parameters[0];
        var cdata = parameters[1];

        if (cmd == "show-chat" ||
            cmd == "show-group-chat" ||
            cmd == "show-contacts") {
            cmdCommand = cmd;
            var parsedParameter = cdata;
            if (parsedParameter.indexOf("&") > 0) {
                cmdAccountId = parsedParameter.substr(0, parsedParameter.indexOf("&"));
            } else {
                cmdAccountId = cdata;
            }

            //message type
            if (cmd == "show-chat") {
                //also get the contact id to open a chat with
                cmdContactId = parsedParameter.substr(parsedParameter.indexOf("&") + 1, parsedParameter.length - 1);
            } else if (cmd == "show-group-chat") {
                // get the group chat id to open
                cmdGroupChatId = parsedParameter.substr(parsedParameter.indexOf("&") + 1, parsedParameter.length - 1);
            }
        }
    }

    function openPageByCommand()
    {
        if(cmdCommand == "show-chat" ||
           cmdCommand == "show-group-chat" ||
           cmdCommand == "show-contacts") {

            if(cmdCommand == "show-chat") {
                currentAccountId = cmdAccountId;
                currentContactId = cmdContactId;
                contactItem = accountsModel.contactItemForId(currentAccountId, currentContactId);
                startConversation(currentContactId);
            } else if(cmdCommand == "show-group-chat") {
                currentAccountId = cmdAccountId;
                startGroupConversation(cmdGroupChatId);
            } else if(cmdCommand == "show-contacts") {
                showContactsScreen(cmdAccountId);
            }
        }
    }

    function playIncomingMessageSound()
    {
        var customizer = protocolsModel.customizerForId(window.accountItem.data(AccountsModel.IconRole));
        eventResource.play(customizer.incomingMessageSound);
    }

    function playConnectedCallSound()
    {
        var customizer = protocolsModel.customizerForId(window.accountItem.data(AccountsModel.IconRole));
        eventResource.play(customizer.connectedCallSound);
    }

    function playHangUpCallSound()
    {
        var customizer = protocolsModel.customizerForId(window.accountItem.data(AccountsModel.IconRole));
        eventResource.play(customizer.hangUpCallSound);
    }

    function playRecordingStartSound()
    {
        var customizer = protocolsModel.customizerForId(window.accountItem.data(AccountsModel.IconRole));
        eventResource.play(customizer.recordingStartSound);
    }

    function playRecordingStopSound()
    {
        var customizer = protocolsModel.customizerForId(window.accountItem.data(AccountsModel.IconRole));
        eventResource.play(customizer.recordingStopSound);
    }

    function playErrorSound()
    {
        var customizer = protocolsModel.customizerForId(window.accountItem.data(AccountsModel.IconRole));
        eventResource.play(customizer.errorSound);
    }

    function playOutgoingCallSound()
    {
        var customizer = protocolsModel.customizerForId(window.accountItem.data(AccountsModel.IconRole));
        // this is not using resource policy since used only when we have the resources (setting up a call)
        imLoopedSoundPlayer.soundSource = customizer.outgoingCallSound;
        imLoopedSoundPlayer.playSound();
    }

    function stopLoopedSound()
    {
        if (imLoopedSoundPlayer.soundSource != "") {
            imLoopedSoundPlayer.stopSound();
            imLoopedSoundPlayer.soundSource = "";
        }
    }

    IMSound {
        id: imLoopedSoundPlayer
        repeat: true
    }

    ResourceSetManager {
        id: eventResource
        applicationClass: "event"
        //applicationClass: "nopolicy"

        property string soundSource : ""

        Component.onCompleted: {
            console.log("eventResource completed")
            addAudioResource("event");
        }

        onBeginUsage: {
            console.log("eventResource.onBeginUsage" + soundSource);
            imSoundPlayer.soundSource = soundSource;
            imSoundPlayer.playSound();
        }

        onEndUsage: {
            console.log("eventResource.onEndUsage");
            stop();
        }

        function play(source) {
            console.log("eventResource.play " + source);
            soundSource = source;
            acquire();
        }

        function stop() {
            console.log("eventResource.stop");
            release();
            if (imSoundPlayer.soundSource != "") {
                imSoundPlayer.stopSound();
                imSoundPlayer.soundSource = "";
            }
        }
    }

    SaveRestoreState {
        id: appState

        onSaveRequired: {
            if (restoreStateEnabled) {
                setValue("currentAccountId", currentAccountId);
                setValue("currentContactId", currentContactId);
                setValue("currentScreen", currentScreen);
                sync();
            }
        }
    }

    IMSound {
        id: imSoundPlayer
        repeat: false
        onEndOfMedia: {
            console.log("eventResource: end of media");
            eventResource.stop();
        }
    }

    function raise()
    {
        var args = [];
        qApp.raise(args);
    }

    AccountContentFactory {
        id: accountFactory
    }

    ConfirmationDialog {
        id: confirmationDialogItem
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
    }

    property alias photoPicker : photoPickerLoader.item

    function createPhotoPicker() {
        if (photoPickerLoader.item == null) {
            console.log("creating PhotoPicker");
            photoPickerLoader.sourceComponent = photoPickerComponent;
        }
    }

    Component {
        id: photoPickerComponent
        PhotoPicker {
            parent: pageStack.currentPage
        }
    }

    Loader {
        id: photoPickerLoader
    }

    property alias avatarPhotoPicker : avatarPhotoPickerLoader.item

    function createAvatarPhotoPicker() {
        if (avatarPhotoPickerLoader.item == null) {
            console.log("creating AvatarPhotoPicker");
            avatarPhotoPickerLoader.sourceComponent = avatarPhotoPickerComponent;
        }
    }

    Component {
        id: avatarPhotoPickerComponent
        PhotoPicker {
            parent: pageStack.currentPage
        }
    }

    Loader {
        id: avatarPhotoPickerLoader
    }

    property alias videoPicker : videoPickerLoader.item

    function createVideoPicker() {
        if (videoPickerLoader.item == null) {
            console.log("creating VideoPicker");
            videoPickerLoader.sourceComponent = videoPickerComponent;
        }
    }

    Component {
        id: videoPickerComponent
        VideoPicker {
            parent: pageStack.currentPage
        }
    }

    Loader {
        id: videoPickerLoader
    }

    property alias musicPicker : musicPickerLoader.item

    function createMusicPicker() {
        if (musicPickerLoader.item == null) {
            console.log("creating MusicPicker");
            musicPickerLoader.sourceComponent = musicPickerComponent;
        }
    }

    Component {
        id: musicPickerComponent
        MusicPicker {
            selectSongs: true
            parent: pageStack.currentPage
        }
    }

    Loader {
        id: musicPickerLoader
    }

    property QtObject contactsPicker : null

    function createContactsPicker() {
        if (contactsPicker == null) {
            console.log("creating ContactsPicker");
            var sourceCode = "import Qt 4.7;"
                           + "import MeeGo.Labs.Components 0.1 as Labs;"
                           + "Labs.ContactsPicker {"
                           + "  parent: pageStack.currentPage;"
                           + "}";
            contactsPicker = Qt.createQmlObject(sourceCode, pageStack.currentPage);
        }
    }

    property QtObject peopleModel : null

    function createPeopleModel() {
        if (peopleModel == null) {
            console.log("creating PeopleModel");
            var sourceCode = "import Qt 4.7;"
                           + "import MeeGo.App.Contacts 0.1;"
                           + "PeopleModel {}";
            peopleModel = Qt.createQmlObject(sourceCode, window);
        }
    }

    property QtObject appModel : null

    function createAppModel() {
        if (appModel == null) {
            console.log("creating ApplicationsModel");
            var sourceCode = "import Qt 4.7;"
                           + "import MeeGo.Labs.Components 0.1 as Labs;"
                           + "Labs.ApplicationsModel {}";
            appModel = Qt.createQmlObject(sourceCode, window);
        }
    }

    function restoreState()
    {
        var accountId = appState.value("currentAccountId");
        var contactId = appState.value("currentContactId");
        var screen = appState.value("currentScreen");

        if (screen == "accounts" && currentScreen == "") {
            // no need to do anything as it is opened by default
            return;
        }

        // verify parameters needed for contacts and chat screen
        if (accountId == "" || accountsModel.accountItemForId(accountId) == null) {
            return;
        }

        // for either option, the contacts screen has to be shown anyway
        if (screen == "contacts" || screen == "chat") {
            showContactsScreen(accountId);
        }

        if (screen == "chat" && contactId != "") {
            // if contact null, stay on the contacts screen
            contactItem = accountsModel.contactItemForId(currentAccountId, contactId);
            if (contactItem != null) {
                // if contact item is valid, open the contacts and then the message screen
                startConversation(contactId);
            }
        }
    }

    function accountStatusMessage(status)
    {
        var connectionStatusReason = window.accountItem.data(AccountsModel.ConnectionStatusReasonRole)
        if (status == TelepathyTypes.ConnectionStatusDisconnected) {
            switch(connectionStatusReason) {
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
                case TelepathyTypes.ConnectionStatusReasonNoneSpecified:
                default:
                    return Constants.errorLoginTryLater;
            }
        } else if (status == TelepathyTypes.ConnectionStatusConnecting) {
            return Constants.contactScreenAccountConnecting;
        } else {
            return "";
        }
    }
}
