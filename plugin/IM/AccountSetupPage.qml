import Qt 4.7
import MeeGo.Ux.Components.Common 0.1

AppPage {
    id: accountSetupPage
    anchors.left: parent.left
    anchors.right: parent.right
    height: childrenRect.height

    pageTitle: addAccountButton.text

    AccountSetupContent {
        id: accountSetupItem
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
