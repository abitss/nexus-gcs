import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var model
    signal closeRequested()
    width: parent ? Math.min(760, Math.max(360, parent.width - 20 < 360 ? parent.width - 20 : parent.width * 0.64)) : 720
    color: "#F70A0F14"; radius: 12; border.color: "#2B3944"; z: 7200

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 14; spacing: 10
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Label { text: qsTr("HIL + FIELD QUALIFICATION"); color: "#F6FAFC"; font.pixelSize: 19; font.bold: true }
                Label { text: qsTr("OBSERVE · MEASURE · ABORT ON ANOMALY"); color: "#AAB7BF"; font.pixelSize: 9; font.bold: true }
            }
            Item { Layout.fillWidth: true }
            NexusIconButton { text: "×"; onClicked: root.closeRequested() }
        }

        RowLayout {
            NexusActionButton { text: qsTr("START HIL"); enabled: !model.running; onClicked: model.startSession("HIL") }
            NexusActionButton { text: qsTr("START FIELD"); enabled: !model.running; onClicked: model.startSession("FIELD") }
            NexusActionButton { text: qsTr("STOP"); enabled: model.running; onClicked: model.stopSession() }
            Item { Layout.fillWidth: true }
            Label { text: model.phase + " · " + model.durationSeconds + " s"; color: "#7FC5AD"; font.bold: true }
        }

        GridLayout {
            Layout.fillWidth: true; columns: 3; columnSpacing: 8; rowSpacing: 8
            Repeater {
                model: [
                    ["HEARTBEAT", model.heartbeatRateHz.toFixed(2) + " Hz"],
                    ["MAVLINK LOSS", model.mavlinkLossPercent.toFixed(2) + "%"],
                    ["GPS", "Fix " + model.gpsFix + " · " + model.satellites + " sats · HDOP " + (isNaN(model.hdop) ? "--" : model.hdop.toFixed(1))],
                    ["MISSION", "Index " + model.missionIndex],
                    ["MODE", model.flightMode],
                    ["THERMAL", isNaN(model.maxDeviceTempC) ? "UNAVAILABLE" : model.maxDeviceTempC.toFixed(1) + " °C"],
                    ["MIN RAM", model.minRamAvailableMb < 0 ? "UNKNOWN" : model.minRamAvailableMb + " MB"],
                    ["MAX UI LAG", model.maxEventLoopLagMs.toFixed(0) + " ms"],
                    ["RECOVERY EVENTS", model.recoveryEventCount]
                ]
                Rectangle {
                    Layout.fillWidth: true; implicitHeight: 66; radius: 8; color: "#10171E"; border.color: "#293740"
                    Column {
                        anchors.centerIn: parent; spacing: 3
                        Label { anchors.horizontalCenter: parent.horizontalCenter; text: modelData[0]; color: "#AAB7BF"; font.pixelSize: 9; font.bold: true }
                        Label { anchors.horizontalCenter: parent.horizontalCenter; text: modelData[1]; color: "#EDF3F6"; font.pixelSize: 11; font.bold: true }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("OPERATIONAL TEST CARDS"); color: "#EEF3F6"; font.bold: true }
            Item { Layout.fillWidth: true }
            Label {
                text: qsTr("PASS only after the card is physically completed and reviewed")
                color: "#D6A84A"
                font.pixelSize: 9
            }
        }

        TextArea {
            id: notes
            Layout.fillWidth: true
            placeholderText: qsTr("Required evidence / operator notes / anomaly / expected behavior")
        }

        GridLayout {
            Layout.fillWidth: true
            columns: root.width >= 680 ? 3 : 2
            columnSpacing: 8
            rowSpacing: 8

            Repeater {
                model: [
                    ["PASS TELEMETRY", "OUTDOOR_TELEMETRY"],
                    ["PASS GPS", "GPS_BEHAVIOR"],
                    ["PASS HOVER", "CONTROLLED_HOVER"],
                    ["PASS MODES", "MODE_TRANSITIONS"],
                    ["PASS MISSION", "MISSION_EXECUTION"],
                    ["PASS LINK", "LINK_DEGRADATION"],
                    ["PASS FAILSAFE", "FAILSAFE_BEHAVIOR"],
                    ["PASS RTL / LAND", "RTL_LAND"],
                    ["PASS RECONNECT", "RECONNECT_RECOVERY"],
                    ["PASS ABORT PATH", "ABORT_PATH"],
                    ["PASS STABILITY", "APP_STABILITY"],
                    ["PASS PERFORMANCE", "PERFORMANCE"],
                    ["PASS THERMAL", "THERMAL_BEHAVIOR"],
                    ["PASS POST-FLIGHT", "POST_FLIGHT_REVIEW"]
                ]

                NexusActionButton {
                    Layout.fillWidth: true
                    text: qsTr(modelData[0])
                    enabled: model.running && model.phase === "FIELD"
                    onClicked: {
                        model.markCard(modelData[1], "PASS", notes.text)
                        notes.clear()
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            NexusActionButton {
                Layout.fillWidth: true
                text: qsTr("FAIL / ABORT")
                enabled: model.running
                critical: true
                onClicked: {
                    model.markCard("OPERATOR_ABORT", "FAIL", notes.text)
                    notes.clear()
                }
            }
            NexusActionButton {
                Layout.fillWidth: true
                text: qsTr("EXPORT EVIDENCE")
                enabled: !model.running
                onClicked: exportPath.text = model.exportDefault()
            }
        }
        Label { id: exportPath; Layout.fillWidth: true; color: "#AAB7BF"; font.pixelSize: 9; elide: Text.ElideMiddle }
        Label { Layout.fillWidth: true; text: qsTr("Field mode records evidence only. It never induces RF loss or overrides PX4 failsafes."); color: "#D6A84A"; font.pixelSize: 9 }
    }
}
