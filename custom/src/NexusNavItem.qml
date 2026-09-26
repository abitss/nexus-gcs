import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property string text: ""
    property bool active: false

    implicitWidth: 92
    implicitHeight: 38
    radius: 8
    color: active ? "#1B2B31" : "transparent"
    border.color: active ? "#2C9B7F" : "#25313C"
    border.width: 1

    Label {
        anchors.centerIn: parent
        text: root.text
        color: active ? "#EFFFFA" : "#98A5B0"
        font.pixelSize: 10
        font.bold: true
    }
}
