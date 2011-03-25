/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.Labs.Components 0.1

Item {
    id: avatar

    property bool active: true
    property alias source: avatarImage.source
    property alias noAvatarImage: noAvatar.source

    width: childrenRect.width
    height: childrenRect.height

    RoundedItem {
        id: avatarMask
        anchors.fill: avatarBorder
        radius: 0 // the item is not rounded anymore
        visible: avatar.source != ""

        Image {
            id: avatarImage

            anchors.fill: parent
            fillMode: Image.PreserveAspectFit

            onStatusChanged: {
                if(status == Image.Error) {
                    avatarMask.visible = false;
                }
            }
        }
    }

    Image {
        id: avatarBorder

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        visible: avatarMask.visible

        width: height

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
        visible: !avatarMask.visible
        width: height
    }
}
