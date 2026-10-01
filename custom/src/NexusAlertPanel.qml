import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var alertManager
    signal closeRequested()

    color: "#F50B0F14"
    z: 6000

    function severityColor(s) {
        if (s === "CRITICAL") return "#D95151"
        if (s === "WARNING") return "#D6A84A"
        return "#4F8FB8"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 12

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 2
                Label { text: "NEXUS ALERT CENTER"; color: "#F6FAFC"; font.pixelSize: 20; font.bold: true }
                Label {
                    text: qsTr("%1 active · %2 unacknowledged").arg(alertManager.activeCount).arg(alertManager.unacknowledgedCount)
                    color: "#98A5B0"; font.pixelSize: 10
                }
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                implicitWidth: 120
                implicitHeight: 38
                radius: 8
                color: "#10171E"
                border.width: 1
                border.color: root.severityColor(alertManager.highestSeverity)
                Label {
                    anchors.centerIn: parent
                    text: alertManager.highestSeverity
                    color: root.severityColor(text)
                    font.bold: true
                }
            }

            Button {
                text: qsTr("ACK ALL")
                enabled: alertManager.unacknowledgedCount > 0
                onClicked: alertManager.acknowledgeAll()
            }
            Button {
                text: qsTr("CLEAR HISTORY")
                onClicked: alertManager.clearInactiveHistory()
            }
            Button {
                text: qsTr("RETURN")
                onClicked: root.closeRequested()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: alertManager.failsafeActive ? 52 : 0
            visible: alertManager.failsafeActive
            radius: 9
            color: "#291316"
            border.color: "#D95151"
            border.width: 1
            Label {
                anchors.centerIn: parent
                text: qsTr("FAILSAFE ACTIVE · Aircraft-level failsafe remains controlled by the autopilot")
                color: "#FFB1B1"
                font.bold: true
            }
        }

        ListView {
            id: list
            objectName: "nexusAlertHistory"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 7
            model: alertManager

            delegate: Rectangle {
                required property int index
                required property string alertId
                required property var timestamp
                required property string severity
                required property string source
                required property string title
                required property string message
                required property bool active
                required property bool acknowledged

                width: list.width
                height: 84
                radius: 9
                color: active ? "#131B22" : "#0F151B"
                opacity: acknowledged ? 0.62 : 1.0
                border.width: 1
                border.color: root.severityColor(severity)

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    Rectangle {
                        Layout.preferredWidth: 6
                        Layout.fillHeight: true
                        radius: 3
                        color: root.severityColor(severity)
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: severity; color: root.severityColor(severity); font.pixelSize: 9; font.bold: true }
                            Label { text: source; color: "#7F8C98"; font.pixelSize: 9 }
                            Label { text: active ? qsTr("ACTIVE") : qsTr("CLEARED"); color: active ? "#D6A84A" : "#73818D"; font.pixelSize: 9 }
                            Item { Layout.fillWidth: true }
                            Label { text: Qt.formatDateTime(timestamp, "hh:mm:ss"); color: "#73818D"; font.pixelSize: 9 }
                        }
                        Label { text: title; color: "#EDF3F7"; font.pixelSize: 12; font.bold: true }
                        Label { Layout.fillWidth: true; text: message; color: "#B8C3CC"; font.pixelSize: 10; elide: Text.ElideRight }
                    }

                    Button {
                        text: acknowledged ? qsTr("ACKED") : qsTr("ACK")
                        enabled: active && !acknowledged
                        onClicked: alertManager.acknowledge(index)
                    }
                }
            }
        }
    }
}
