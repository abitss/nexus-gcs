import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property string title: ""
    property string state: "UNKNOWN"
    property string detail: "--"

    implicitWidth: 220
    implicitHeight: 112
    radius: 10
    color: "#10171E"
    border.width: 1
    border.color: state === "CRITICAL" ? "#D95151"
                 : state === "DEGRADED" ? "#D6A84A"
                 : state === "NOMINAL" ? "#2C9B7F"
                 : "#4F6575"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: root.title
                color: "#B9C4CC"
                font.pixelSize: 10
                font.bold: true
            }
            Item { Layout.fillWidth: true }
            Label {
                text: root.state
                color: root.state === "CRITICAL" ? "#FF9C9C"
                       : root.state === "DEGRADED" ? "#F0D49A"
                       : root.state === "NOMINAL" ? "#A8F3D6"
                       : "#91A0AC"
                font.pixelSize: 10
                font.bold: true
            }
        }

        Label {
            Layout.fillWidth: true
            text: root.detail
            color: "#E5EBF0"
            font.pixelSize: 10
            wrapMode: Text.WordWrap
            maximumLineCount: 3
            elide: Text.ElideRight
        }
    }
}
