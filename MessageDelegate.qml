/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.App.IM 0.1
import MeeGo.Labs.Components 0.1

Item {
    id: mainArea

    width: parent.width
    height: eventItem ? eventMessage.height : childrenRect.height

    anchors.margins: 10

    property bool eventItem: model.type == "event"
    property bool fileTransferItem: model.type == "incoming_file_transfer" ||
                                    model.type == "outgoing_file_transfer"
    property bool messageItem: model.type == "incoming_message" || model.type == "outgoing_message"
    property bool messageSent: model.type == "outgoing_message" || model.type == "outgoing_file_transfer"
    // TODO: check how to add more colors for group chat
    property string color: model.bubbleColor //messageSent ? "white" : "blue"
    property bool expandedMessage: false

    function messageAvatar() {
        var avatar = "";
        if (messageSent) {
            avatar = "image://avatars/" + scene.currentAccountId + // i18n ok
                        "?" + accountContentFactory.avatarSerial;
        } else {
            avatar = model.contactAvatar;
        }

        return avatar;
    }

    Avatar {
        id: avatar
        visible: !eventItem
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        width: 80
        height: 80
        source: messageAvatar()
        //width:  childrenRect.width
        //height: childrenRect.height
    }

    Item {
        id: message
        visible: messageItem
        anchors.top: parent.top
        anchors.left: avatar.right
        anchors.right: parent.right
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        anchors.leftMargin: -17
        anchors.rightMargin: 0
        smooth: true

        height: ((expandedMessage || model.includeSearch) ?
            Math.max(messageHeader.height + messageBody.height + messageBody.anchors.margins,
                     messageTop.height + messageBody.height) :
                 messageHeader.height + 10)

        MouseArea {
            anchors.fill: parent
            onClicked: {
                expandedMessage = true;
            }
        }

        BorderImage {
            id: messageTop
            source: "image://meegotheme/widgets/apps/chat/bubble-" + color + "-top"
            //border.left: int; border.top: int
            //border.right: int; border.bottom: int
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            border.left: 40
            border.right: 20
            border.top: 12

            height: ((expandedMessage || model.includeSearch) ?
                         62 : 10 )
        }

        BorderImage {
            id: messageCenter
            source: "image://meegotheme/widgets/apps/chat/bubble-" + color + "-middle"
            border.left: messageTop.border.left
            border.right: messageTop.border.right

            anchors.top: messageTop.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: messageBottom.top

            height: ((expandedMessage || model.includeSearch) ?
                         messageBody.height : 10 )
        }

        BorderImage {
            id: messageBottom
            source: "image://meegotheme/widgets/apps/chat/bubble-" + color + "-bottom"
            border.left: messageTop.border.left
            border.right: messageTop.border.right

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
        }

        Item {
            id: messageHeader
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            height: childrenRect.height + 10

            PresenceIcon {
                id: presence
                anchors.left: parent.left
                anchors.verticalCenter: contact.verticalCenter
                anchors.margins: 5
                anchors.leftMargin: messageTop.border.left

                status: model.status
            }

            Text {
                id: contact
                anchors.left: presence.right
                anchors.top: parent.top
                anchors.topMargin: 10
                anchors.bottomMargin: 10
                anchors.leftMargin:5
                anchors.right: time.left
                anchors.rightMargin: 10
                text: model.contact

                color: Qt.rgba(0.3,0.3,0.3,1)
                font.weight: Font.Bold
                elide: Text.ElideRight
            }

            Text {
                id: time
                anchors.right: parent.right
                anchors.bottom: contact.bottom
                anchors.rightMargin: messageTop.border.right
                text: fuzzyDateTime.getFuzzy(model.time)

                color: Qt.rgba(0.3,0.3,0.3,1)

                Connections {
                    target: fuzzyDateTimeUpdater
                    onTriggered: {
                        time.text = fuzzyDateTime.getFuzzy(model.time);
                    }
                }
            }
        }

        Text {
            id: messageBody
            anchors.top: messageHeader.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 10
            anchors.leftMargin: messageTop.border.left
            anchors.rightMargin: messageTop.border.right

            text: parseChatText(model.text)
            wrapMode: Text.WordWrap
            textFormat: Text.RichText

            color: theme_fontColorNormal
            visible: (expandedMessage || model.includeSearch)
        }
    }

    Item {
        id: eventMessage
        visible: eventItem
        width: parent.width
        height: childrenRect.height
        anchors.margins: 10

        Text {
            id: eventMessageText
            horizontalAlignment: Text.AlignHCenter
            color: theme_fontColorInactive
            font.pixelSize: theme_fontPixelSizeSmall
            // i18n: the first argument is the event itself, the second one is the fuzzy time
            text: qsTr("%1 - %2").arg(model.text)
                                 .arg(fuzzyDateTime.getFuzzy(model.time))
            wrapMode: Text.WordWrap

            Connections {
                target: fuzzyDateTimeUpdater
                onTriggered: {
                    eventMessageText.text = qsTr("%1 - %2").arg(model.text)
                                                           .arg(fuzzyDateTime.getFuzzy(model.time));
                }
            }
        }
    }

    FileTransferDelegate {
        id: fileTransferMessage
        visible: fileTransferItem
        width: parent.width
    }

    function parseChatText(message) {
        var parsedMessage = message;

        //recurse as long as there is an image to replace
        while(parsedMessage.indexOf(":-{") > -1) {
            var index = parsedMessage.indexOf(":-{");
            var endIndex = index + 3;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-angry.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf(":-&amp;") > -1) {
            var index = parsedMessage.indexOf(":-&amp;");
            var endIndex = index + 7;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-angry.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf(":S") > -1) {
            var index = parsedMessage.indexOf(":S");
            var endIndex = index + 2;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-confused.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf(":-S") > -1) {
            var index = parsedMessage.indexOf(":-S");
            var endIndex = index + 3;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-confused.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf(":-[") > -1) {
            var index = parsedMessage.indexOf(":-[");
            var endIndex = index + 3;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-embarressed.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf(":)") > -1) {
            var index = parsedMessage.indexOf(":)");
            var endIndex = index + 2;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-happy.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf(":-)") > -1) {
            var index = parsedMessage.indexOf(":-)");
            var endIndex = index + 3;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-happy.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf("&lt;3") > -1) {
            var index = parsedMessage.indexOf("&lt;3");
            var endIndex = index + 5;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-love.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf(":(") > -1) {
            var index = parsedMessage.indexOf(":(");
            var endIndex = index + 2;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-sad.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf(":'(") > -1) {
            var index = parsedMessage.indexOf(":'(");
            var endIndex = index + 3;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-sad.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf("(*)") > -1) {
            var index = parsedMessage.indexOf("(*)");
            var endIndex = index + 3;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-star.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf("|-(") > -1) {
            var index = parsedMessage.indexOf("|-(");
            var endIndex = index + 3;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-tired.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf(";)") > -1) {
            var index = parsedMessage.indexOf(";)");
            var endIndex = index + 2;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-wink.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        while(parsedMessage.indexOf(";-)") > -1) {
            var index = parsedMessage.indexOf(";-)");
            var endIndex = index + 3;

            var emoticonSource = "/usr/share/themes/" + theme_name + "/icons/emotes/emote-wink.png"
            var imageTag = imageTagging(emoticonSource);
            parsedMessage = parsedMessage.substr(0, index) + imageTag + parsedMessage.substr(endIndex);
        }

        return parsedMessage;
    }

    function imageTagging(sourceName) {
        return "<img src=\"" + sourceName + "\" >"
    }
}
