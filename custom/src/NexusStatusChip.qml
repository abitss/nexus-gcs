import QtQuick
import QtQuick.Controls
import "NexusTokens.js" as T

Rectangle {
    id: root

    property string label: ""
    property string value: "--"
    property color accentColor: T.textMuted
    property color foregroundColor: T.textPrimary
    property color mutedColor: T.textSecondary

    implicitHeight: 40
    implicitWidth: Math.max(104, content.implicitWidth + T.space24)
    radius: T.radiusSmall
    color: "#E0121D25"
    border.color: accentColor
    border.width: 1

    Row {
        id: content
        anchors.centerIn: parent
        spacing: T.space8

        Label {
            text: root.label
            color: root.mutedColor
            font.pixelSize: T.textCaption
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }

        Label {
            text: root.value
            color: root.foregroundColor
            font.pixelSize: T.textBody
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
