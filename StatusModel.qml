import Qt 4.7
import TelepathyQML 0.1

ListModel {
    id: statusModel

    Component.onCompleted: {
        statusModel.append({"status": "available", // i18n ok
                            "type": TelepathyTypes.ConnectionPresenceTypeAvailable,
                            "text": qsTr("Available")});
        statusModel.append({"status": "away", // i18n ok
                            "type": TelepathyTypes.ConnectionPresenceTypeAway,
                            "text": qsTr("Away")});
        statusModel.append({"status": "busy", // i18n ok
                            "type": TelepathyTypes.ConnectionPresenceTypeBusy,
                            "text": qsTr("Busy")});
        statusModel.append({"status": "invisible", // i18n ok
                            "type": TelepathyTypes.ConnectionPresenceTypeHidden,
                            "text": qsTr("Invisible")});
        statusModel.append({"status": "offline", // i18n ok
                            "type": TelepathyTypes.ConnectionPresenceTypeOffline,
                            "text": qsTr("Offline")});
    }
}
