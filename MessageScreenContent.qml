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
    id: messageScreenPage
    anchors.fill: parent
    enableCustomActionMenu: true
    actionMenuOpen: messageContentMenuLoader.item == null || messageContentMenuLoader.item.visible

    property string contactId: window.currentContactId
    property string contactName: (window.contactItem != undefined? window.contactItem.data(AccountsModel.AliasRole) : "")
    property bool modelLoaded: (conversationView.model != undefined)
    property int accountStatus: 0
    property bool isMessageScreen: true

    Component.onCompleted: {
        initPage();
    }

    Component.onDestruction: {
        if(window.chatAgent != undefined && !window.chatAgent.isConference) {
            accountsModel.disconnectConversationModel(window.currentAccountId,
                                                      contactId);
        } else {
            accountsModel.disconnectGroupConversationModel(window.currentAccountId,
                                                           window.chatAgent.channelPath);
        }
    }

    onActionMenuIconClicked: {
        if (messageContentMenuLoader.item == null) {
            messageContentMenuLoader.sourceComponent = messageContentMenuComponent;
        }

        messageContentMenuLoader.item.setPosition( mouseX, mouseY);
        messageContentMenuLoader.item.show();
    }

    onActivating: {
        console.log("MessageScreenContent onActivating");
        notificationManager.chatActive = true;
        window.currentScreen = "chat"; // i18n ok
        var videoWindow = messageScreenPage.getVideoWindow();
        if (videoWindow != null) {
            videoWindow.activate();
        }

        if (window.chatAgent != undefined) {
            window.chatAgent.requestChatState(TelepathyTypes.ChannelChatStateActive);
        }
    }

    onDeactivating: {
        console.log("MessageScreenContent onDeactivating");
        notificationManager.chatActive = false;
        var videoWindow = messageScreenPage.getVideoWindow();
        if (videoWindow != null) {
            videoWindow.deactivate();
        }
        window.chatAgent.requestChatState(TelepathyTypes.ChannelChatStateInactive);
    }

    // small trick to reload the data() role values when the item changes
    Connections {
        target: window.contactItem
        onChanged: window.contactItem = window.contactItem
    }

    Connections {
        target: window.accountItem

        onChanged: {
            accountStatus = window.accountItem.data(AccountsModel.ConnectionStatusRole);
        }
    }

    Connections {
        target: accountsModel
        onChatReady:  {
            if (accountId == window.currentAccountId && contactId == window.currentContactId) {
                setupDataFromChatAgent();
            }
        }

        onGroupChatReady:  {
            if (accountId == window.currentAccountId && channelPath == window.chatAgent.channelPath) {
                setupDataFromChatAgent();
            }
        }

        onNetworkStatusChanged: {
            showInfoBar();
        }
    }

    Connections {
        id: callAgentConnections
        target: null
        onCallStatusChanged: {
            // Several sounds might play at once here. Should be prioritize, make a queue, or let them all play ?
            if (window.callAgent.callStatus == CallAgent.CallStatusNoCall) {
                window.fullScreen = false;
                var videoWindow = messageScreenPage.getVideoWindow();
                videoWindow.opacity = 0;
                messageScreenPage.unloadVideoWindow();
            }
            // activate call ringing
            if(window.callAgent.callStatus == CallAgent.CallStatusRinging) {
                window.playOutgoingCallSound();
            } else {
                window.stopLoopedSound();
            }
            // connection established
            if (window.callAgent.callStatus == CallAgent.CallStatusTalking) {
                window.playConnectedCallSound();
            }
            if (window.callAgent.callStatus == CallAgent.CallStatusHangingUp) {
                window.playHangUpCallSound();
            }
            if (window.callAgent.error) {
                window.playErrorSound();
                // do not clear error on purpose, so other components do not miss it;
                // we might see the error twice, and cannot really tell if it is the same error or not,
                // but we don't really care as we'd just play the error sound twice,
                // which is not a bad idea if the error state persists anyway
            }
        }
    }

    Connections {
        target: window
        onSearch: {
            // only search if the message screen is active
            if (notificationManager.chatActive) {
                conversationView.model.searchByString(needle);
                searchHeader.searchActive = (needle != "");
            }
        }
    }

    Connections {
        id: conversationModelConnections
        target: null

        onCurrentRowMatchChanged: {
            conversationView.positionViewAtIndex(conversationView.model.currentRowMatch, ListView.Center);
        }

        onBackFetchable: {
            if (conversationView.model.canFetchMoreBack()) {
                historyFeeder.running = true;
            }
        }
        onBackFetched: {
            historyFeeder.fetching = false;
            if (historyFeeder.oldIndex != -1) {
                conversationView.positionViewAtIndex(historyFeeder.oldIndex + numItems, ListView.Beginning);
            } else {
                conversationView.positionViewAtIndex(0, ListView.End);
            }

            if (!conversationView.model.canFetchMoreBack()) {
                historyFeeder.running = false;
            }
        }
    }

    Item {
        id: pageContent
        parent: messageScreenPage
        anchors.fill: parent
        // if the messages roll over the main bar, uncomment this line to
        // force clipping
        clip: true

        Image {
            anchors.fill: parent
            source: "image://themedimage/widgets/common/backgrounds/global-background-texture"
        }

        SearchHeader {
            id: searchHeader
            searchActive: window.showToolBarSearch
            searching: conversationView.model != undefined ? conversationView.model.searching : false
            olderActive : conversationView.model != undefined ? conversationView.model.olderActive : false
            newerActive : conversationView.model != undefined ? conversationView.model.newerActive : false
            numMatchesFound: conversationView.model != undefined ? conversationView.model.numMatchesFound : 0
            onOlderClicked: {
                conversationView.model.olderMatch();
            }
            onNewerClicked: {
                conversationView.model.newerMatch();
            }
        }

        InfoBar {
            id: infoBar

            anchors {
                top: searchHeader.bottom
                left: parent.left
                right: parent.right
            }
        }

        Component {
            id: sectionDateDelegate
            Item {
                width: conversationView.width
                height: 50

                Text {
                    id: dateText
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: theme_fontColorHighlight
                    font.pixelSize: theme_fontPixelSizeLarge
                    //verticalAlignment: Text.AlignVCenter
                    //horizontalAlignment: Text.AlignHCenter
                    text: section
                }

                Image {
                    anchors.top: dateText.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "image://themedimage/widgets/common/dividers/divider-horizontal-single"
                }
            }
        }

        ListView {
            id: conversationView
            anchors {
                top: infoBar.bottom
                left: parent.left
                right: parent.right
                bottom: window.fullScreen ? imToolBar.top : textBar.top
                topMargin: 10 - window.contentVerticalShift
                leftMargin: 10
                rightMargin: 10
                bottomMargin: 10
            }

            cacheBuffer: 600
            delegate: MessageDelegate { }
            highlightFollowsCurrentItem: true
            currentIndex: count - 1
            clip: true

            section.property : "dateString"
            section.criteria : ViewSection.FullString
            section.delegate : sectionDateDelegate

            interactive: contentHeight > height

            onCountChanged: {
                window.playIncomingMessageSound();
            }
        }

        /*
          Timer used to feed history from the logger at the beginning of the view
        */
        Timer {
            id: historyFeeder
            interval: 1000
            running: false
            repeat: true

            property bool fetching : false
            property int oldIndex : -1

            onTriggered: {
                if (conversationView.atYBeginning) {
                    if (!fetching && conversationView.model.canFetchMoreBack()) {
                        fetching = true;
                        oldIndex = 0;
                        conversationView.model.fetchMoreBack();
                    }
                }
            }

            onFetchingChanged: {
                showInfoBar();
            }
        }

        Image {
            id: textBar
            source: "image://themedimage/widgets/common/action-bar/action-bar-background"
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: imToolBar.top
            height: textBox.height
            visible: !window.fullScreen

            Item {
                id: textBox
                anchors.left: parent.left
                anchors.right: sendMessageButton.left
                anchors.rightMargin: 5

                height: textEdit.height + 2 * textEdit.anchors.margins

                TextField {
                    id: textEdit
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 10
                    textFormat: Text.RichText
                    font.pixelSize: theme_fontPixelSizeLarge
                    height: contentHeight + 2 * anchors.margins
                    enabled: (conversationView.model != undefined? true : false)
                    Keys.onEnterPressed: {
                        if(parseChatText(textEdit.text) != "") {
                            conversationView.model.sendMessage(parseChatText(textEdit.text));
                            textEdit.text = "";
                        }
                    }
                    Keys.onReturnPressed: {
                        if(parseChatText(textEdit.text) != "") {
                            conversationView.model.sendMessage(parseChatText(textEdit.text));
                            textEdit.text = "";
                        }
                    }

                    function isEmpty() {
                        var parsedText = messageScreenPage.parseChatText(textEdit.text);
                        return (parsedText.trim() == "");
                    }

                    Keys.onPressed: {
                        window.chatAgent.requestChatState(TelepathyTypes.ChannelChatStateComposing);
                        stateTimer.restart();
                    }

                    Timer {
                        id: stateTimer
                        interval: 10000
                        onTriggered: {
                            window.chatAgent.requestChatState(TelepathyTypes.ChannelChatStatePaused);
                        }
                    }
                }
            }

            Button {
                id: sendMessageButton
                visible: !window.fullScreen
                anchors {
                    margins: 10
                    right: parent.right
                    verticalCenter: textBox.verticalCenter
                    topMargin: 0
                }
                height: 40
                text: Constants.messageSend
                textColor: theme_buttonFontColor

                onClicked: {
                    if(!textEdit.isEmpty()) {
                        conversationView.model.sendMessage(parseChatText(textEdit.text));
                        textEdit.text = "";
                    }
                }
            }
        }

        Component {
            id: videoWindowComponent
            VideoWindow {
                id: videoWindow
                pageContentItem: pageContent
                toolBarHeight: imToolBar.height
                parent: messageScreenPage
            }
        }

        Loader {
            id: videoWindowLoader
            property variant caller: null
            onStatusChanged: {
                if (videoWindowLoader.status == Loader.Ready) {
                    var c = caller;
                    caller = null;
                    if (c != null) {
                        c.onVideoWindowLoaded(videoWindowLoader.item);
                    }
                }
            }
        }

        IMToolBar {
            id: imToolBar
            parent: pageContent

            onChatTextEnterPressed: {
                if(textEdit.text != "") {
                    conversationView.model.sendMessage(parseChatText(textEdit.text));
                    textEdit.text = "";
                }
            }

            onSmileyClicked: {
                // save the cursor position
                var position = textEdit.cursorPosition;
                textEdit.text = textEdit.text + "<img src=\"" + sourceName + "\" >";

                // give the focus back to the text editor
                textEdit.focus = true;
                textEdit.cursorPosition = position + 1;
            }
        }
    }

    Loader {
        id: messageContentMenuLoader
    }

    TopItem {
        id: messageTopItem
    }

    Component {
        id: messageContentMenuComponent
        ContextMenu {
            id: messageContentMenu

            width: 200
            forceFingerMode: 2

            content: MessageContentMenu {
                currentPage: messageScreenPage;
            }
        }
    }

    function showInfoBar()
    {
        var text = "";
        if (!networkOnline) {
            text = Constants.noNetworkText;
        } else  if (accountStatus != TelepathyTypes.ConnectionStatusConnected) {
            // check first whether the account is offline or just connecting
            text = window.accountStatusMessage(accountStatus);
        } else if (!modelLoaded) {
            text = Constants.messageOpeningChat;
        } else if (historyFeeder.fetching) {
            text = Constants.messageLoadingHistory;
        }

        // assign and show/hide as necessary
        infoBar.text = text;
        if (text == "") {
            infoBar.hide();
        } else {
            infoBar.show();
        }
    }

    function parseChatText(message)
    {
        var parsedMessage;

        // first remove the head
        var index = message.indexOf("</head>");
        parsedMessage = message.substr(index + 8, message.length);

        // remove the body tag
        index = parsedMessage.indexOf(">");
        parsedMessage = parsedMessage.substr(index + 1, message.length);

        // remove the end body tag
        index = parsedMessage.indexOf("</body>");
        parsedMessage = parsedMessage.substr(0, index);

        // remove paragraph tag
        index = parsedMessage.indexOf(">");
        parsedMessage = parsedMessage.substr(index + 1, message.length);

        //remove end paragraph tag
        index = parsedMessage.indexOf("</p>");
        parsedMessage = parsedMessage.substr(0, index);

        //recurse as long as there is an image to replace

        while(parsedMessage.indexOf("<img") > -1) {
            var imgIndex = parsedMessage.indexOf("<img");
            var srcIndex = parsedMessage.indexOf("src");
            var endIndex = parsedMessage.indexOf("/>");

            var emoticonName = parsedMessage.substr(srcIndex + 5, parsedMessage.length);
            emoticonName = emoticonName.substr(0, emoticonName.indexOf("/>") - 6);

            var nameIndex = emoticonName.indexOf("emote-");
            emoticonName = emoticonName.substr(nameIndex + 6, emoticonName.length);

            //replace the image name with ascii chars
            var asciiEmo;
            if(emoticonName == "angry") {
                asciiEmo = ":-&";
            } else if (emoticonName == "confused") {
                asciiEmo = ":-S";
            } else if (emoticonName == "embarressed") {
                asciiEmo = ":-[";
            } else if (emoticonName == "happy") {
                asciiEmo = ":-)";
            } else if (emoticonName == "love") {
                asciiEmo = "<3";
            } else if (emoticonName == "sad") {
                asciiEmo = ":'(";
            } else if (emoticonName == "star") {
                asciiEmo = "(*)";
            } else if (emoticonName == "tired") {
                asciiEmo = "|-(";
            } else if (emoticonName == "wink") {
                asciiEmo = ";-)";
            }

            parsedMessage = parsedMessage.substr(0, imgIndex) + asciiEmo + parsedMessage.substr(endIndex + 2);
        }

        while(parsedMessage.indexOf("&lt;") > -1) {
            var ltIndex = parsedMessage.indexOf("&lt;");
            parsedMessage = parsedMessage.substr(0, ltIndex) + "<" + parsedMessage.substr(ltIndex + 4);
        }

        while(parsedMessage.indexOf("&gt;") > -1) {
            var ltIndex = parsedMessage.indexOf("&gt;");
            parsedMessage = parsedMessage.substr(0, ltIndex) + ">" + parsedMessage.substr(ltIndex + 4);
        }

        while(parsedMessage.indexOf("&amp;") > -1) {
            var ltIndex = parsedMessage.indexOf("&amp;");
            parsedMessage = parsedMessage.substr(0, ltIndex) + "&" + parsedMessage.substr(ltIndex + 5);
        }

        while(parsedMessage.indexOf("&quot;") > -1) {
            var ltIndex = parsedMessage.indexOf("&quot;");
            parsedMessage = parsedMessage.substr(0, ltIndex) + "\"" + parsedMessage.substr(ltIndex + 6);
        }

        return parsedMessage;
    }

    function hideActionMenu()
    {
        if (messageContentMenuLoader.item != null) {
            messageContentMenuLoader.item.hide();
        }
    }

    function closeConversation()
    {
        // assuming we need to end the chat session when close is pressed
        if (window.chatAgent.isConference) {
            accountsModel.endChat(window.currentAccountId, window.chatAgent.channelPath);
        } else {
            accountsModel.endChat(window.currentAccountId, contactId);
        }

        if (window.callAgent != undefined) {
            window.callAgent.endCall();
        }

        window.popPage();
    }

    function loadVideoWindow(caller)
    {
        if (videoWindowLoader.item == null) {
            videoWindowLoader.caller = caller;
            videoWindowLoader.sourceComponent = videoWindowComponent;
        }
        else {
            caller.onVideoWindowLoaded(videoWindowLoader.item);
        }
    }

    function unloadVideoWindow()
    {
        videoWindowLoader.sourceComponent = undefined;
    }

    function getVideoWindow() {
        if (videoWindowLoader.item == null) {
            console.log("WARNING: videoWindowLoader is NULL");
        }
        return videoWindowLoader.item;
    }

    function setupDataFromChatAgent()
    {
        if(window.chatAgent != undefined && window.chatAgent.existsChat) {
            if (window.chatAgent.isConference) {
                pageTitle = Constants.messageGroupConversation;
                conversationView.model = accountsModel.groupConversationModel(window.currentAccountId,
                                                                              window.chatAgent.channelPath);
            } else {
                pageTitle = Constants.messageChatWith.arg(window.contactItem.data(AccountsModel.AliasRole));
                conversationView.model = accountsModel.conversationModel(window.currentAccountId,
                                                                         window.currentContactId);
                if (conversationView.model != undefined) {
                    window.fileTransferAgent.setModel(conversationView.model);
                }
            }

            if (conversationView.model != undefined) {
                window.chatAgent.requestChatState(TelepathyTypes.ChannelChatStateActive);
                conversationView.positionViewAtIndex(conversationView.count - 1, ListView.End);
                conversationModelConnections.target = conversationView.model;
                textEdit.focus = true;
                showInfoBar();
            }
        }
    }

    function initPage()
    {
        showInfoBar();

        if (window.chatAgent != undefined) {
            setupDataFromChatAgent();
        }

        if (window.callAgent != undefined) {
            callAgentConnections.target = window.callAgent;
            var status = window.callAgent.callStatus;
            if (status != CallAgent.CallStatusNoCall) {
                messageScreenPage.loadVideoWindow(messageScreenPage);
            }
            window.callAgent.resetMissedCalls();
        }
    }

    function onVideoWindowLoaded(videoWindow) {
        videoWindow.opacity = 1;
        window.callAgent.setOutgoingVideo(videoWindow.cameraWindowSmall ? videoWindow.videoOutgoing : videoWindow.videoIncoming);
        window.callAgent.onOrientationChanged(window.orientation);
        window.callAgent.setIncomingVideo(videoWindow.cameraWindowSmall ? videoWindow.videoIncoming : videoWindow.videoOutgoing);
    }
}
