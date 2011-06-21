/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import Qt 4.7

Item {
    // presence icons
    property string availablePresenceIcon : "image://themedimage/icons/status/status-available"
    property string busyPresenceIcon : "image://themedimage/icons/status/status-busy"
    property string awayPresenceIcon : "image://themedimage/icons/status/status-idle"
    property string extendedAwayPresenceIcon : "image://themedimage/icons/status/status-idle"
    property string hiddenPresenceIcon : ""
    property string offlinePresenceIcon : ""
    property string unknownPresenceIcon : ""
    property string errorPresenceIcon : ""

    // sounds
    property string incomingCallSound : "/usr/share/sounds/meego/stereo/ring-1.wav"
    property string outgoingCallSound : "/usr/share/sounds/meego/stereo/ring-4.wav"
    property string hangUpCallSound : "/usr/share/sounds/meego/stereo/disconnect.wav"
    property string incomingMessageSound : "/usr/share/sounds/meego/stereo/chat-fg.wav"
    property string errorSound : "/usr/share/sounds/meego/stereo/error.wav"
    property string connectedCallSound : "/usr/share/sounds/meego/stereo/connect.wav"
    property string recordingStartSound : "/usr/share/sounds/meego/stereo/rec-start.wav"
    property string recordingStopSound : "/usr/share/sounds/meego/stereo/rec-stop.wav"
}
