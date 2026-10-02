import QtQuick
import QtQuick.Controls
import "NexusTokens.js" as T

Button {
    id: root
    implicitWidth: T.touchMin
    implicitHeight: T.touchMin
    padding: 0

    background: Rectangle {
        radius: T.radiusSmall
        color: root.down ? T.surfacePressed : "transparent"
        border.color: root.hovered || root.activeFocus ? T.border : "transparent"
        border.width: 1
        Behavior on color { ColorAnimation { duration: T.motionFast } }
    }

    contentItem: Label {
        text: root.text
        color: root.enabled ? T.textSecondary : T.textMuted
        font.pixelSize: 18
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
