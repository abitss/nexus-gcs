import QtQuick
import QtQuick.Controls
import "NexusTokens.js" as T

Rectangle {
    id: root

    property string label: ""
    property string value: "--"
    property string units: ""
    property color foregroundColor: T.textPrimary
    property color mutedColor: T.textMuted

    implicitWidth: 120
    implicitHeight: 56
    radius: T.radiusSmall
    color: "#E60A141B"
    border.color: T.borderSubtle
    border.width: 1

    Column {
        anchors.centerIn: parent
        spacing: T.space2

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.label
            color: root.mutedColor
            font.pixelSize: T.textCaption
            font.bold: true
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: T.space4

            Label {
                text: root.value
                color: root.foregroundColor
                font.pixelSize: T.textMetric
                font.bold: true
            }

            Label {
                visible: root.units.length > 0
                text: root.units
                color: root.mutedColor
                font.pixelSize: T.textCaption
                anchors.baseline: parent.children[0].baseline
            }
        }
    }
}
