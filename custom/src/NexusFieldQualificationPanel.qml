import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var model
    signal closeRequested()
    width: Math.min(720, parent ? parent.width * 0.62 : 720)
    color: "#F70A0F14"; radius: 12; border.color: "#2B3944"; z: 7200

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 14; spacing: 10
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Label { text: qsTr("HIL + FIELD QUALIFICATION"); color: "#F6FAFC"; font.pixelSize: 19; font.bold: true }
                Label { text: qsTr("OBSERVE · MEASURE · ABORT ON ANOMALY"); color: "#83919A"; font.pixelSize: 9; font.bold: true }
            }
            Item { Layout.fillWidth: true }
            Button { text: "×"; onClicked: root.closeRequested() }
        }

        RowLayout {
            Button { text: qsTr("START HIL"); enabled: !model.running; onClicked: model.startSession("HIL") }
            Button { text: qsTr("START FIELD"); enabled: !model.running; onClicked: model.startSession("FIELD") }
            Button { text: qsTr("STOP"); enabled: model.running; onClicked: model.stopSession() }
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
                        Label { anchors.horizontalCenter: parent.horizontalCenter; text: modelData[0]; color: "#83919A"; font.pixelSize: 8; font.bold: true }
                        Label { anchors.horizontalCenter: parent.horizontalCenter; text: modelData[1]; color: "#EDF3F6"; font.pixelSize: 11; font.bold: true }
                    }
                }
            }
        }

        Label { text: qsTr("TEST CARDS"); color: "#EEF3F6"; font.bold: true }
        TextArea { id: notes; Layout.fillWidth: true; placeholderText: qsTr("Operator notes / anomaly / expected behavior") }
        RowLayout {
            Button { text: qsTr("PASS TELEMETRY"); onClicked: model.markCard("OUTDOOR_TELEMETRY","PASS",notes.text) }
            Button { text: qsTr("PASS GPS"); onClicked: model.markCard("GPS_BEHAVIOR","PASS",notes.text) }
            Button { text: qsTr("PASS MISSION"); onClicked: model.markCard("MISSION_EXECUTION","PASS",notes.text) }
            Button { text: qsTr("PASS LINK"); onClicked: model.markCard("LINK_DEGRADATION","PASS",notes.text) }
            Button { text: qsTr("PASS FAILSAFE"); onClicked: model.markCard("FAILSAFE_BEHAVIOR","PASS",notes.text) }
        }
        RowLayout {
            Button { text: qsTr("PASS STABILITY"); onClicked: model.markCard("APP_STABILITY","PASS",notes.text) }
            Button { text: qsTr("PASS PERFORMANCE"); onClicked: model.markCard("PERFORMANCE","PASS",notes.text) }
            Button { text: qsTr("PASS THERMAL"); onClicked: model.markCard("THERMAL_BEHAVIOR","PASS",notes.text) }
            Button { text: qsTr("FAIL / ABORT"); onClicked: model.markCard("OPERATOR_ABORT","FAIL",notes.text) }
            Button { text: qsTr("EXPORT"); enabled: !model.running; onClicked: exportPath.text = model.exportDefault() }
        }
        Label { id: exportPath; Layout.fillWidth: true; color: "#83919A"; font.pixelSize: 8; elide: Text.ElideMiddle }
        Label { Layout.fillWidth: true; text: qsTr("Field mode records evidence only. It never induces RF loss or overrides PX4 failsafes."); color: "#D6A84A"; font.pixelSize: 8 }
    }
}
