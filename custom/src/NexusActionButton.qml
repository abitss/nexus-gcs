import QtQuick
import QtQuick.Controls
import "NexusTokens.js" as T

Button {
    id: root

    property bool critical: false
    property bool primary: false

    implicitHeight: T.touchMin
    implicitWidth: Math.max(112, contentItem.implicitWidth + T.space24 * 2)
    leftPadding: T.space16
    rightPadding: T.space16
    topPadding: T.space8
    bottomPadding: T.space8

    background: Rectangle {
        radius: T.radiusSmall
        color: !root.enabled ? "#111920"
             : root.down ? T.surfacePressed
             : root.critical ? T.criticalSoft
             : root.primary ? T.successSoft
             : T.surfaceRaised
        border.color: !root.enabled ? T.borderSubtle
                    : root.critical ? T.critical
                    : root.primary ? T.success
                    : T.border
        border.width: 1

        Behavior on color { ColorAnimation { duration: T.motionFast } }
        Behavior on border.color { ColorAnimation { duration: T.motionFast } }
    }

    contentItem: Label {
        text: root.text
        color: root.enabled ? T.textPrimary : T.textMuted
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: T.textBody
        font.bold: true
        elide: Text.ElideRight
    }
}
