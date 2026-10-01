import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property string title: ""
    property string state: "WARNING"
    property string detail: "--"

    implicitWidth: 240
    implicitHeight: 108
    radius: 10
    color: "#10171E"
    border.width: 1
    border.color: state === "BLOCKED" ? "#D95151"
                 : state === "WARNING" ? "#D6A84A"
                 : "#2C9B7F"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        RowLayout {
            Layout.fillWidth: true
            Label { text: root.title; color: "#B9C4CC"; font.pixelSize: 10; font.bold: true }
            Item { Layout.fillWidth: true }
            Label {
                text: root.state
                color: root.state === "BLOCKED" ? "#FF9C9C"
                       : root.state === "WARNING" ? "#F0D49A"
                       : "#A8F3D6"
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
