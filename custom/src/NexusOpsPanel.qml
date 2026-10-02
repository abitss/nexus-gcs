import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property string missionText: "--"
    property real missionProgress: 0
    property string nextWaypointText: "--"
    property string homeText: "--"
    property string homeEtaText: "--"
    property string preflightText: "--"
    property string videoText: "--"
    property string batteryTimeText: "--"
    property color preflightAccent: "#6F7E8C"
    property color videoAccent: "#6F7E8C"

    implicitWidth: 260
    implicitHeight: 190
    radius: 10
    color: "#E60A0E13"
    border.color: "#273540"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 7

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("MISSION")
                color: "#80909D"
                font.pixelSize: 9
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            Label {
                text: root.missionText
                color: "#F1F6F9"
                font.pixelSize: 12
                font.bold: true
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 5
            radius: 2.5
            color: "#1A242D"

            Rectangle {
                width: parent.width * Math.max(0, Math.min(1, root.missionProgress))
                height: parent.height
                radius: parent.radius
                color: "#2C9B7F"
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 6

            Label { text: qsTr("NEXT WP"); color: "#AAB7BF"; font.pixelSize: 9; font.bold: true }
            Label { text: root.nextWaypointText; color: "#E8EEF3"; font.pixelSize: 10; Layout.alignment: Qt.AlignRight }

            Label { text: qsTr("HOME"); color: "#AAB7BF"; font.pixelSize: 9; font.bold: true }
            Label { text: root.homeText; color: "#E8EEF3"; font.pixelSize: 10; Layout.alignment: Qt.AlignRight }

            Label { text: qsTr("HOME ETA"); color: "#AAB7BF"; font.pixelSize: 9; font.bold: true }
            Label { text: root.homeEtaText; color: "#E8EEF3"; font.pixelSize: 10; Layout.alignment: Qt.AlignRight }

            Label { text: qsTr("BAT TIME"); color: "#AAB7BF"; font.pixelSize: 9; font.bold: true }
            Label { text: root.batteryTimeText; color: "#E8EEF3"; font.pixelSize: 10; Layout.alignment: Qt.AlignRight }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#24313C"
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 7

            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: root.preflightAccent
            }

            Label {
                text: qsTr("PREFLIGHT")
                color: "#AAB7BF"
                font.pixelSize: 9
                font.bold: true
            }

            Label {
                text: root.preflightText
                color: "#E8EEF3"
                font.pixelSize: 10
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: root.videoAccent
            }

            Label {
                text: root.videoText
                color: "#E8EEF3"
                font.pixelSize: 10
                font.bold: true
            }
        }
    }
}
