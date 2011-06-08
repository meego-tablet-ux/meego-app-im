import Qt 4.7
import TelepathyQML 0.1

ListModel {
    id: statusModel
    ListElement {
        status: "available"; // i18n ok
        type: TelepathyTypes.ConnectionPresenceTypeAvailable
        text: QT_TR_NOOP("Available")
    }
    ListElement {
        status: "away"; // i18n ok
        type: TelepathyTypes.ConnectionPresenceTypeAway
        text: QT_TR_NOOP("Away")
    }
    ListElement {
        status: "busy"; // i18n ok
        type: TelepathyTypes.ConnectionPresenceTypeBusy
        text: QT_TR_NOOP("Busy")
    }
    ListElement {
        status: "invisible"; // i18n ok
        type: TelepathyTypes.ConnectionPresenceTypeHidden
        text: QT_TR_NOOP("Invisible")
    }
    ListElement {
        status: "offline"; // i18n ok
        type: TelepathyTypes.ConnectionPresenceTypeOffline
        text: QT_TR_NOOP("Offline")
    }
}
