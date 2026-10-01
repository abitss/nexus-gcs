import QtQuick
import QtQuick.Controls

Button {
    id: root

    property bool critical: false
    property bool primary: false

    implicitHeight: 40
    implicitWidth: Math.max(104, contentItem.implicitWidth + 28)

    background: Rectangle {
        radius: 8
        color: !root.enabled ? "#141A20"
                            : root.critical ? "#5B2528"
                            : root.primary ? "#173D35"
                            : "#151E27"
        border.color: !root.enabled ? "#28323B"
                                  : root.critical ? "#D85B60"
                                  : root.primary ? "#2C9B7F"
                                  : "#3B4D5C"
        border.width: 1
    }

    contentItem: Label {
        text: root.text
        color: root.enabled ? "#F2F6F9" : "#687581"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 11
        font.bold: true
    }
}
