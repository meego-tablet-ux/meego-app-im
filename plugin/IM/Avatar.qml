/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7

Item {
    id: avatar

    property bool active: true
    property alias source: avatarImage.source
    property alias noAvatarImage: noAvatar.source

    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    width: childrenRect.width
    height:  parent.height

    Image {
        id: avatarImage

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        height: parent.height
        width: height

        fillMode: Image.PreserveAspectFit

        visible: (source != ""? true : false)
        onStatusChanged: {
            if(status == Image.Error) {
                avatarImage.visible = false;
            }
        }
    }

    Image {
        id: avatarBorder

        anchors.fill: avatarImage

        visible: avatarImage.visible

        source: (active ?
                     "image://meegotheme/widgets/common/avatar/avatar-shadow" :
                     "image://meegotheme/widgets/common/avatar/avatar-inactive-overlay")
    }

    Image {
        id: noAvatar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        source: "image://meegotheme/widgets/common/avatar/avatar-default"
        visible: !avatarImage.visible
        width: height
        height: parent.height
    }
}
