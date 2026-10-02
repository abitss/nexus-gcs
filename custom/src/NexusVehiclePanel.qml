import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import "NexusTokens.js" as T

Rectangle {
    id: root

    property var vehicleModel
    signal closeRequested()
    signal openPayloadRequested()

    width: parent ? Math.min(620, Math.max(360, parent.width - 20 < 360 ? parent.width - 20 : parent.width * 0.52)) : 560
    color: "#F7081017"
    border.color: "#273540"
    border.width: 1
    radius: 12
    z: 6600

    function accent(state) {
        if (state === "READY" || state === "AVAILABLE") return "#2C9B7F"
        if (state === "SETUP REQUIRED" || state === "INCOMPLETE" || state === "LOADING" || state === "WRITING") return "#D6A84A"
        if (state === "LINK LOST") return "#D95151"
        return "#60717E"
    }

    function openKnown(component) {
        if (!vehicleModel.safeToConfigure) return
        mainWindow.showKnownVehicleComponentConfigPage(component)
        root.closeRequested()
    }

    component SetupCard: Rectangle {
        id: setupCard
        property string title
        property string detail
        property string state
        property bool available: true
        property bool actionEnabled: true
        property string actionText: qsTr("OPEN")
        signal action()

        Layout.fillWidth: true
        implicitHeight: 96
        radius: 9
        color: "#10171E"
        border.color: root.accent(state)

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 4

            RowLayout {
                Layout.fillWidth: true
                Label { text: title; color: "#F1F5F8"; font.pixelSize: 12; font.bold: true }
                Item { Layout.fillWidth: true }
                Label {
                    text: available ? state : qsTr("UNAVAILABLE")
                    color: root.accent(available ? state : "UNAVAILABLE")
                    font.pixelSize: 9
                    font.bold: true
                }
            }

            Label {
                Layout.fillWidth: true
                text: detail
                color: "#AAB7BF"
                font.pixelSize: 9
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }

            Item { Layout.fillHeight: true }

            Button {
                Layout.alignment: Qt.AlignRight
                text: actionText
                enabled: available && actionEnabled
                onClicked: setupCard.action()
            }
        }
    }

    NexusConfirmDialog {
        id: rebootDialog
        heading: qsTr("Reboot flight controller?")
        message: qsTr("The vehicle reports configuration changes that require a reboot. Continue only while the aircraft is safely disarmed on the ground.")
        confirmText: qsTr("REBOOT")
        critical: true
        onConfirmed: vehicleModel.rebootVehicle()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 1
                Label { text: qsTr("VEHICLE"); color: "#F6FAFC"; font.pixelSize: 19; font.bold: true }
                Label {
                    text: vehicleModel.connected
                          ? vehicleModel.vehicleIdText + " · " + vehicleModel.vehicleType
                          : qsTr("No active vehicle")
                    color: "#8D9AA5"
                    font.pixelSize: 9
                }
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                implicitWidth: 102
                implicitHeight: 30
                radius: 7
                color: "#0D141A"
                border.color: vehicleModel.connected
                              ? (vehicleModel.linkLost ? "#D95151" : "#2C9B7F")
                              : "#60717E"

                Label {
                    anchors.centerIn: parent
                    text: !vehicleModel.connected ? qsTr("OFFLINE")
                          : (vehicleModel.linkLost ? qsTr("LINK LOST") : qsTr("CONNECTED"))
                    color: parent.border.color
                    font.pixelSize: 9
                    font.bold: true
                }
            }

            NexusIconNexusIconButton { text: "×"; onClicked: root.closeRequested() }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: vehicleModel.rebootRequired ? 76 : 0
            visible: vehicleModel.rebootRequired
            radius: 9
            color: "#251D10"
            border.color: "#D6A84A"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Label { text: qsTr("REBOOT REQUIRED"); color: "#F0C76A"; font.pixelSize: 11; font.bold: true }
                    Label {
                        Layout.fillWidth: true
                        text: vehicleModel.rebootParameters.join(", ")
                        color: "#C5B58C"
                        font.pixelSize: 9
                        elide: Text.ElideRight
                    }
                }

                Button {
                    text: qsTr("REBOOT")
                    enabled: vehicleModel.safeToReboot
                    onClicked: rebootDialog.open()
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 124
            radius: 9
            color: "#10171E"
            border.color: "#273540"

            GridLayout {
                anchors.fill: parent
                anchors.margins: 11
                columns: 4
                columnSpacing: 12
                rowSpacing: 7

                Label { text: qsTr("FIRMWARE"); color: "#9AAAB4"; font.pixelSize: 9; font.bold: true }
                Label { text: qsTr("VERSION"); color: "#9AAAB4"; font.pixelSize: 9; font.bold: true }
                Label { text: qsTr("MODE"); color: "#9AAAB4"; font.pixelSize: 9; font.bold: true }
                Label { text: qsTr("SETUP"); color: "#9AAAB4"; font.pixelSize: 9; font.bold: true }

                Label { text: vehicleModel.firmwareType; color: "#EDF2F5"; font.pixelSize: 10; font.bold: true }
                Label { text: vehicleModel.firmwareVersion; color: "#EDF2F5"; font.pixelSize: 10; font.bold: true }
                Label { text: vehicleModel.flightMode; color: "#EDF2F5"; font.pixelSize: 10; font.bold: true }
                Label { text: vehicleModel.setupState; color: root.accent(vehicleModel.setupState); font.pixelSize: 10; font.bold: true }

                Label { text: qsTr("PARAMETERS"); color: "#9AAAB4"; font.pixelSize: 9; font.bold: true }
                Label { text: qsTr("UID"); color: "#9AAAB4"; font.pixelSize: 9; font.bold: true }
                Label { text: qsTr("GIT"); color: "#9AAAB4"; font.pixelSize: 9; font.bold: true }
                Label { text: qsTr("STATE"); color: "#9AAAB4"; font.pixelSize: 9; font.bold: true }

                Label { text: vehicleModel.parameterState; color: root.accent(vehicleModel.parameterState); font.pixelSize: 10; font.bold: true }
                Label { text: vehicleModel.uidText; color: "#C8D1D8"; font.pixelSize: 9; elide: Text.ElideMiddle; Layout.fillWidth: true }
                Label { text: vehicleModel.firmwareGitHash; color: "#C8D1D8"; font.pixelSize: 9; elide: Text.ElideMiddle; Layout.fillWidth: true }
                Label {
                    text: vehicleModel.flying ? qsTr("FLYING") : (vehicleModel.armed ? qsTr("ARMED") : qsTr("DISARMED"))
                    color: vehicleModel.armed || vehicleModel.flying ? "#D6A84A" : "#2C9B7F"
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }

        Label {
            Layout.fillWidth: true
            visible: vehicleModel.connected && !vehicleModel.safeToConfigure
            text: vehicleModel.linkLost
                  ? qsTr("Configuration locked while vehicle communication is lost.")
                  : qsTr("Configuration locked while the vehicle is armed or flying.")
            color: "#E5B95D"
            font.pixelSize: 9
            wrapMode: Text.WordWrap
        }

        NexusStateView {
            visible: !vehicleModel.connected
            Layout.fillWidth: true
            Layout.fillHeight: true
            state: "empty"
            title: qsTr("No vehicle connected")
            message: qsTr("Connect a Pixhawk or telemetry link to inspect firmware, sensors, power, radio, flight modes, safety and calibration.")
        }

        ScrollView {
            visible: vehicleModel.connected
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            GridLayout {
                width: parent.width
                columns: root.width >= 520 ? 2 : 1
                columnSpacing: 8
                rowSpacing: 8

                SetupCard {
                    title: qsTr("AIRFRAME")
                    detail: qsTr("Vehicle frame and geometry. Opens QGC's firmware-specific vehicle configuration.")
                    state: vehicleModel.airframeState
                    available: vehicleModel.connected
                    actionEnabled: vehicleModel.safeToConfigure
                    onAction: {
                        mainWindow.showVehicleConfig()
                        root.closeRequested()
                    }
                }

                SetupCard {
                    title: qsTr("SENSORS / CALIBRATION")
                    detail: qsTr("IMU, accelerometer, gyro, compass and supported sensor calibration.")
                    state: vehicleModel.sensorsState
                    available: vehicleModel.sensorsAvailable
                    actionEnabled: vehicleModel.safeToConfigure
                    onAction: root.openKnown(AutoPilotPlugin.KnownSensorsVehicleComponent)
                }

                SetupCard {
                    title: qsTr("POWER")
                    detail: qsTr("Battery, power module and firmware-supported power configuration.")
                    state: vehicleModel.powerState
                    available: vehicleModel.powerAvailable
                    actionEnabled: vehicleModel.safeToConfigure
                    onAction: root.openKnown(AutoPilotPlugin.KnownPowerVehicleComponent)
                }

                SetupCard {
                    title: qsTr("RADIO")
                    detail: qsTr("RC calibration and channel mapping when supported by the autopilot.")
                    state: vehicleModel.radioState
                    available: vehicleModel.radioAvailable
                    actionEnabled: vehicleModel.safeToConfigure
                    onAction: root.openKnown(AutoPilotPlugin.KnownRadioVehicleComponent)
                }

                SetupCard {
                    title: qsTr("FLIGHT MODES")
                    detail: qsTr("Review and configure firmware-supported mode assignments.")
                    state: vehicleModel.flightModesState
                    available: vehicleModel.flightModesAvailable
                    actionEnabled: vehicleModel.safeToConfigure
                    onAction: root.openKnown(AutoPilotPlugin.KnownFlightModesVehicleComponent)
                }

                SetupCard {
                    title: qsTr("SAFETY")
                    detail: qsTr("Failsafe, return, geofence and other firmware-specific safety settings.")
                    state: vehicleModel.safetyState
                    available: vehicleModel.safetyAvailable
                    actionEnabled: vehicleModel.safeToConfigure
                    onAction: root.openKnown(AutoPilotPlugin.KnownSafetyVehicleComponent)
                }

                SetupCard {
                    title: qsTr("CAMERA / PAYLOAD")
                    detail: qsTr("Camera stream, recording, zoom, gimbal and local media controls.")
                    state: NexusPayload.streamState
                    available: true
                    actionEnabled: true
                    actionText: qsTr("OPEN PAYLOAD")
                    onAction: root.openPayloadRequested()
                }

                SetupCard {
                    title: qsTr("ADVANCED PARAMETERS")
                    detail: qsTr("Engineering-only parameter access through QGroundControl's existing parameter editor.")
                    state: vehicleModel.parameterState
                    available: vehicleModel.connected
                    actionEnabled: vehicleModel.safeToConfigure
                    actionText: qsTr("PARAMETERS")
                    onAction: {
                        mainWindow.showVehicleConfigParametersPage()
                        root.closeRequested()
                    }
                }
            }
        }
    }
}
