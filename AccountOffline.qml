/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7
import MeeGo.App.IM 0.1
import MeeGo.Components 0.1
import TelepathyQML 0.1

InfoBar {
    id: container
    text: qsTr("Account is offline");

    function setInfoMessage(status)
    {
        if (accountStatus == TelepathyTypes.ConnectionStatusDisconnected) {
            text = qsTr("Account is offline");
        } else if (accountStatus == TelepathyTypes.ConnectionStatusConnecting) {
            text = qsTr("Account is connecting...");
        } else {
            text = "";
        }
    }


}
