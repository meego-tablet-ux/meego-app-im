import QtQuick 1.0
import MeeGo.Labs.Components 0.1
import MeeGo.App.IM 0.1

ModalDialog {
    id: container

    property string mainText: ""
    property string subText: ""
    property string instanceReason: ""

    signal accepted()
    signal rejected()

    onDialogClicked: {
        if (button == 1) {
            container.accepted();
        } else if (button == 2) {
            container.rejected();
        }
        dialogLoader.sourceComponent = undefined;
    }

    dialogWidth: 420
    dialogHeight: 480
    leftButtonText: qsTr("Yes")
    bgSourceUpLeft: "image://meegotheme/images/btn_blue_up"
    bgSourceDnLeft: "image://meegotheme/images/btn_blue_dn"
    rightButtonText: qsTr("No")
    bgSourceUpRight: "image://meegotheme/images/btn_red_up"
    bgSourceDnRight: "image://meegotheme/images/btn_red_dn"
    contentLoader.sourceComponent: Item {
        anchors.fill: contentLoader

        Column {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
            spacing: 10

            Text {
                id: mainText
                anchors.left: parent.left
                anchors.right: parent.right
                text: container.mainText
                wrapMode: Text.WordWrap
            }

            Text {
                id: subText
                anchors.left: parent.left
                anchors.right: parent.right
                text: container.subText
                wrapMode: Text.WordWrap
            }
        }
    }

}
