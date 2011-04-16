import Qt 4.7

Item {
    id: mainArea
    width: parent.width
    height: label.height + 10

    property alias text: label.text
    property alias textWidth: label.width

    signal clicked()

    BorderImage {
        id: activeBackground
        anchors.fill: parent
        border.left: 5
        border.right: 5
        border.top: 5
        border.bottom: 5

        source: "image://meegotheme/widgets/common/menu/menu-item-active"
        visible: mouseArea.pressed
    }

    Text {
        id: label
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 10
        anchors.left: parent.left
        color: mouseArea.pressed ? theme_fontColorSelected :
                                   theme_contextMenuFontColor
        font.pixelSize: theme_contextMenuFontPixelSize
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        onClicked: {
            mainArea.clicked();
        }
    }
}
