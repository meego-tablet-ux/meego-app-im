import Qt 4.7
import MeeGo.App.IM 0.1
import TelepathyQML 0.1

Item {
    id: mainArea

    width: parent.width
    height: childrenRect.height

    Loader {
        width: parent.width
        sourceComponent: model.publishState == TelepathyTypes.PresenceStateAsk?
                             contactRequestComponent : contactComponent
    }

    Component {
        id: contactRequestComponent

        ContactRequestDelegate {
            contactItem: model.item
            aliasName: model.aliasName
            avatar: model.avatar
        }
    }

    Component {
        id: contactComponent

        ContactDelegate {
            presenceMessage: model.presenceMessage
            presenceType: model.presenceType
            contactId: model.id
            aliasName: model.aliasName
            avatar: model.avatar
            avatarList: model.avatarList
            chatOpened: model.chatOpened
            textChat: model.textChat
            audioCall: model.audioCall
            videoCall: model.videoCall
            canBlockContacts: model.canBlockContacts
            blocked: model.blocked
            canReportAbuse: model.canReportAbuse
            messageCount: model.pendingMessages
            missedAudioCalls: model.missedAudioCalls
            missedVideoCalls: model.missedVideoCalls
            openChat: model.chatOpened
        }
    }
}

