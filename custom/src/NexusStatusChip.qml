import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property string label: ""
    property string value: "--"
    property color accentColor: "#6F7E8C"
    property color foregroundColor: "#EAF0F5"
    property color mutedColor: "#82909D"

    implicitHeight: 34
    implicitWidth: Math.max(94, content.implicitWidth + 22)
    radius: 7
    color: "#C9121820"
    border.color: accentColor
    border.width: 1

    Row {
        id: content
        anchors.centerIn: parent
        spacing: 7

        Label {
            text: root.label
            color: root.mutedColor
            font.pixelSize: 10
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }

        Label {
            text: root.value
            color: root.foregroundColor
            font.pixelSize: 12
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
