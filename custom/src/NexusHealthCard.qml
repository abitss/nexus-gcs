import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "NexusTokens.js" as T

Rectangle {
    id: root

    property string title: ""
    property string state: "UNKNOWN"
    property string detail: "--"

    implicitWidth: 220
    implicitHeight: 116
    radius: T.radiusMedium
    color: T.surface
    border.width: 1
    border.color: state === "CRITICAL" ? "#D95151"
                 : state === "DEGRADED" ? "#D6A84A"
                 : state === "NOMINAL" ? "#2C9B7F"
                 : "#4F6575"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: T.space12
        spacing: T.space6

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: root.title
                color: T.textSecondary
                font.pixelSize: T.textBody
                font.bold: true
            }
            Item { Layout.fillWidth: true }
            Label {
                text: root.state
                color: root.state === "CRITICAL" ? "#FF9C9C"
                       : root.state === "DEGRADED" ? "#F0D49A"
                       : root.state === "NOMINAL" ? "#A8F3D6"
                       : "#91A0AC"
                font.pixelSize: T.textBody
                font.bold: true
            }
        }

        Label {
            Layout.fillWidth: true
            text: root.detail
            color: T.textPrimary
            font.pixelSize: T.textBody
            wrapMode: Text.WordWrap
            maximumLineCount: 3
            elide: Text.ElideRight
        }
    }
}
