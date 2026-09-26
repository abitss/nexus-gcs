import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property string label: ""
    property string value: "--"
    property string units: ""
    property color foregroundColor: "#F3F7FA"
    property color mutedColor: "#7F8C98"

    implicitWidth: 118
    implicitHeight: 52
    radius: 8
    color: "#CC0B0F14"
    border.color: "#263441"
    border.width: 1

    Column {
        anchors.centerIn: parent
        spacing: 2

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.label
            color: root.mutedColor
            font.pixelSize: 9
            font.bold: true
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 4

            Label {
                text: root.value
                color: root.foregroundColor
                font.pixelSize: 16
                font.bold: true
            }

            Label {
                visible: root.units.length > 0
                text: root.units
                color: root.mutedColor
                font.pixelSize: 10
                anchors.baseline: parent.children[0].baseline
            }
        }
    }
}
