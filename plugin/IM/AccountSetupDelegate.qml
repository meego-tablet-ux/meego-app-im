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

ExpandingBox {
    id: setupDelegate

    detailsComponent: accountFactory.embeddedAccountContent(model.id, setupDelegate)

    property int itemWidth: parent.width
    property int accountStatus: model.statusType
    property variant accountContent: detailsItem.accountContent
    //property bool expanded: false

    expandedHeight: detailsItem.height + expandButton.height

    Image {
        id: serviceIcon
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10
        source: accountFactory.accountIcon(model.icon, model.connectionStatus)
        smooth: true
    }

    Text {
        id: accountTypeName
        anchors.left: serviceIcon.right
        anchors.top: parent.top
        anchors.margins: 10
        text: model.displayName
        elide: Text.ElideRight
        font.weight: Font.Bold
        color: theme_fontColorNormal
    }
}
