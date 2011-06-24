import Qt 4.7
import MeeGo.Ux.Components.Common 0.1

AppPage {
    id: accountSetupPage
    anchors.fill: parent
    pageTitle: addAccountButton.text

    Flickable {
        id: accountSetupArea

        anchors.fill: parent
        clip: true

        flickableDirection: "VerticalFlick"
        contentHeight: accountSetupItem.height

        AccountSetupContent {
            id: accountSetupItem
            anchors.left: parent.left
            anchors.right: parent.right
        }
    }
}
