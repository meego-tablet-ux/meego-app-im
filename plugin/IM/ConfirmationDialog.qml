import QtQuick 1.0
import MeeGo.Components 0.1
import MeeGo.App.IM 0.1
import "imconstants.js" as IMConstants

ModalMessageBox {
    id: container

    property string instanceReason: ""
    property string accountId: ""

    width: 400
    height: 300
    fogMaskVisible: false

    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter

    acceptButtonText: IMConstants.confirmationDialogYes
    acceptButtonImage: "image://themedimage/widgets/common/button/button-positive"
    acceptButtonImagePressed: "image://themedimage/widgets/common/button/button-positive-pressed"
    cancelButtonText: IMConstants.confirmationDialogNo
    cancelButtonImage: "image://themedimage/widgets/common/button/button-negative"
    cancelButtonImagePressed: "image://themedimage/widgets/common/button/button-negative-pressed"
}
