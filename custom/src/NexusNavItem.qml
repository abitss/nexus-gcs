import QtQuick
import QtQuick.Controls
import "NexusTokens.js" as T

Rectangle {
    id: root

    property string text: ""
    property bool active: false
    property bool compact: false
    signal clicked()

    implicitWidth: compact ? 78 : 96
    implicitHeight: T.touchMin
    radius: T.radiusSmall
    color: mouseArea.pressed ? T.surfacePressed : (active ? T.successSoft : "transparent")
    border.color: active ? T.success : T.borderSubtle
    border.width: 1

    Behavior on color { ColorAnimation { duration: T.motionFast } }
    Behavior on border.color { ColorAnimation { duration: T.motionFast } }

    Label {
        anchors.centerIn: parent
        width: parent.width - T.space12
        text: root.text
        color: root.active ? T.textPrimary : T.textSecondary
        font.pixelSize: T.textCaption
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
