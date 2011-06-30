/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import TelepathyQML 0.1

Item {
    height:  12
    width: 12
    property int status: 0
    property variant customizer: protocolsModel.customizerForId(mainChatWindow.accountItem.data(AccountsModel.IconRole));


    onStatusChanged: {
        if (status >= 0) {
            iconImage.source = iconImage.statusIcon(status);
        }
    }

    Image {
        id: iconImage
        height: parent.height
        width: parent.width
        source: (status >= 0 ? statusIcon(parent.status) : "")
        anchors.verticalCenter: parent.verticalCenter
        smooth: true

        function statusIcon(type) {
            var icon;

            if (customizer == undefined) {
                return "";
            }

            switch (type) {
            case TelepathyTypes.ConnectionPresenceTypeAvailable:
                icon = customizer.availablePresenceIcon;
                break;
            case TelepathyTypes.ConnectionPresenceTypeBusy:
                icon = customizer.busyPresenceIcon;
                break;
            case TelepathyTypes.ConnectionPresenceTypeAway:
                icon = customizer.awayPresenceIcon;
                break;
            case TelepathyTypes.ConnectionPresenceTypeExtendedAway:
                icon = customizer.extendedAwayPresenceIcon;
                break;
            case TelepathyTypes.ConnectionPresenceTypeHidden:
                icon = customizer.hiddenPresenceIcon;
                break;
            case TelepathyTypes.ConnectionPresenceTypeOffline:
                icon = customizer.offlinePresenceIcon;
                break;
            case TelepathyTypes.ConnectionPresenceTypeUnknown:
                icon = customizer.unknownPresenceIcon;
                break;
            case TelepathyTypes.ConnectionPresenceTypeError:
            default:
                icon = customizer.errorPresenceIcon;
                break;
            }
            return icon;
        }
    }
}
