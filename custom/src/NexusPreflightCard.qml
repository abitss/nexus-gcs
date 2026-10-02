import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "NexusTokens.js" as T

Rectangle {
    id: root
    property string title: ""
    property string state: "WARNING"
    property string detail: "--"

    implicitWidth: 240
    implicitHeight: 116
    radius: T.radiusMedium
    color: T.surface
    border.width: 1
    border.color: state === "BLOCKED" ? "#D95151"
                 : state === "WARNING" ? "#D6A84A"
                 : "#2C9B7F"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: T.space12
        spacing: T.space6

        RowLayout {
            Layout.fillWidth: true
            Label { text: root.title; color: T.textSecondary; font.pixelSize: T.textBody; font.bold: true }
            Item { Layout.fillWidth: true }
            Label {
                text: root.state
                color: root.state === "BLOCKED" ? "#FF9C9C"
                       : root.state === "WARNING" ? "#F0D49A"
                       : "#A8F3D6"
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
