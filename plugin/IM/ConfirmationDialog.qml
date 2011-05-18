import QtQuick 1.0
import MeeGo.Components 0.1
import MeeGo.App.IM 0.1

ModalMessageBox {
    id: container

    property string instanceReason: ""
    property string accountId: ""

    width: 420
    height: 480
    fogMaskVisible: false

    acceptButtonText: qsTr("Yes")
    acceptButtonImage: "image://themedimage/images/btn_blue_up"
    acceptButtonImagePressed: "image://themedimage/images/btn_blue_dn"
    cancelButtonText: qsTr("No")
    cancelButtonImage: "image://themedimage/images/btn_red_up"
    cancelButtonImagePressed: "image://themedimage/images/btn_red_dn"

}
