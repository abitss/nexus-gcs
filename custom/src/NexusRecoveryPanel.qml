import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "NexusTokens.js" as T

Rectangle {
    id: root
    property var recoveryModel
    signal closeRequested()

    width: parent ? Math.min(740, Math.max(360, parent.width - 20 < 360 ? parent.width - 20 : parent.width * 0.62)) : 700
    color: "#F70A0F14"
    border.color: "#2B3944"
    border.width: 1
    radius: 12
    z: 7150

    component RecoveryCard: Rectangle {
        id: card
        property string title
        property string value
        property string detail
        property string state: "NOMINAL"
        Layout.fillWidth: true
        implicitHeight: 88
        radius: 8
        color: "#10171E"
        border.color: state === "CRITICAL" ? "#D95151"
                    : state === "WARNING" ? "#D6A84A" : "#293740"
        NexusConfirmDialog {
        id: clearRecoveryDialog
        heading: qsTr("Clear recovered event history?")
        message: qsTr("This clears the in-memory Recovery timeline only. Security audit evidence remains intact.")
        confirmText: qsTr("CLEAR EVENTS")
        critical: true
        onConfirmed: recoveryModel.clearRecoveredEvents()
    }

    ColumnLayout {
            anchors.fill: parent
            anchors.margins: 9
            spacing: 3
            RowLayout {
                Layout.fillWidth: true
                Label { text: card.title; color: "#EEF3F6"; font.pixelSize: 10; font.bold: true }
                Item { Layout.fillWidth: true }
                Label {
                    text: card.value
                    color: card.state === "CRITICAL" ? "#E36A6A"
                         : card.state === "WARNING" ? "#E0B85F" : "#7FC5AD"
                    font.pixelSize: 8
                    font.bold: true
                }
            }
            Label { Layout.fillWidth: true; text: card.detail; color: "#83919A"; font.pixelSize: 8; wrapMode: Text.WordWrap }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Label { text: qsTr("CRASH + RECOVERY"); color: "#F6FAFC"; font.pixelSize: 19; font.bold: true }
                Label { text: qsTr("DETECT · PRESERVE · RESTORE · VERIFY"); color: "#83919A"; font.pixelSize: 9; font.bold: true }
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                implicitWidth: 126; implicitHeight: 30; radius: 7
                color: "#0D141A"
                border.color: recoveryModel.overallState === "RECOVERY REQUIRED" ? "#D95151"
                            : recoveryModel.overallState === "DEGRADED" ? "#D6A84A" : "#2C9B7F"
                Label { anchors.centerIn: parent; text: recoveryModel.overallState; color: parent.border.color; font.pixelSize: 8; font.bold: true }
            }
            NexusIconButton { text: "×"; onClicked: root.closeRequested() }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ColumnLayout {
                width: parent.width
                spacing: 8

                GridLayout {
                    Layout.fillWidth: true
                    columns: root.width >= 600 ? 2 : 1
                    columnSpacing: 8
                    rowSpacing: 8

                    RecoveryCard {
                        title: qsTr("APP / PROCESS RESTART")
                        value: recoveryModel.previousUncleanExit ? qsTr("UNCLEAN PREVIOUS EXIT") : qsTr("CLEAN")
                        detail: qsTr("A persisted running marker detects Android/process death where normal shutdown callbacks never ran.")
                        state: recoveryModel.previousUncleanExit ? "WARNING" : "NOMINAL"
                    }

                    RecoveryCard {
                        title: qsTr("FOREGROUND / BACKGROUND")
                        value: recoveryModel.appState
                        detail: qsTr("Android/Qt application lifecycle is tracked without auto-resuming dangerous commands.")
                    }

                    RecoveryCard {
                        title: qsTr("FC REBOOT")
                        value: recoveryModel.fcRebootDetected ? qsTr("DETECTED") : qsTr("NOT DETECTED")
                        detail: qsTr("Uses MAVLink SYSTEM_TIME boot counter rollback when the autopilot reports it. Reconnect alone is not mislabeled as reboot.")
                        state: recoveryModel.fcRebootDetected ? "CRITICAL" : "NOMINAL"
                    }

                    RecoveryCard {
                        title: qsTr("TELEMETRY / UDP")
                        value: recoveryModel.udpLost ? qsTr("UDP LOST")
                             : recoveryModel.telemetryLost ? qsTr("TELEMETRY LOST") : qsTr("CONNECTED")
                        detail: qsTr("Heartbeat loss is classified from the active QGC vehicle link.")
                        state: recoveryModel.telemetryLost ? "CRITICAL" : "NOMINAL"
                    }

                    RecoveryCard {
                        title: qsTr("USB")
                        value: recoveryModel.usbLost ? qsTr("DISCONNECTED") : qsTr("AVAILABLE / NOT REQUIRED")
                        detail: qsTr("Tracks disappearance and re-enumeration of the tablet's serial/USB inventory.")
                        state: recoveryModel.usbLost ? "CRITICAL" : "NOMINAL"
                    }

                    RecoveryCard {
                        title: qsTr("VIDEO")
                        value: recoveryModel.videoLost ? qsTr("LOST") : qsTr("NOMINAL")
                        detail: qsTr("Uses PAYLOAD's real decode-loss state. Video recovery never fabricates frames or stream health.")
                        state: recoveryModel.videoLost ? "WARNING" : "NOMINAL"
                    }

                    RecoveryCard {
                        title: qsTr("MISSION TRANSFER")
                        value: recoveryModel.interruptedMission ? qsTr("REVERIFY REQUIRED") : qsTr("VERIFIED / IDLE")
                        detail: qsTr("Interrupted upload/readback invalidates mission trust. Nexus never silently resumes a half-completed transfer.")
                        state: recoveryModel.interruptedMission ? "CRITICAL" : "NOMINAL"
                    }

                    RecoveryCard {
                        title: qsTr("STORAGE")
                        value: recoveryModel.storageBlocked ? qsTr("FULL / BLOCKED") : qsTr("WRITABLE")
                        detail: qsTr("Zero available local bytes blocks trust in new recordings/log writes and raises recovery-required state.")
                        state: recoveryModel.storageBlocked ? "CRITICAL" : "NOMINAL"
                    }

                    RecoveryCard {
                        title: qsTr("PERMISSIONS")
                        value: recoveryModel.permissionRevoked ? qsTr("REVOKED") : qsTr("AVAILABLE")
                        detail: qsTr("Runtime permission revocation is detected from Device Health and remains explicit until restored.")
                        state: recoveryModel.permissionRevoked ? "WARNING" : "NOMINAL"
                    }

                    RecoveryCard {
                        title: qsTr("CORRUPTED MISSION")
                        value: qsTr("FAIL CLOSED")
                        detail: qsTr("Recovery mission files must pass Security/QGC Plan validation before Nexus will treat them as trusted.")
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Button {
                        text: qsTr("ACK MISSION INTERRUPTION")
                        enabled: recoveryModel.interruptedMission
                        onClicked: recoveryModel.acknowledgeInterruptedMission()
                    }
                    NexusActionButton {
                        text: qsTr("CLEAR RECOVERED EVENTS")
                        onClicked: clearRecoveryDialog.open()
                    }
                    Item { Layout.fillWidth: true }
                }

                Label { text: qsTr("RECOVERY EVENT TIMELINE"); color: "#EEF3F6"; font.pixelSize: 10; font.bold: true }

                Repeater {
                    model: recoveryModel.events
                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: eventText.implicitHeight + 16
                        radius: 7
                        color: "#10171E"
                        border.color: modelData.severity === "CRITICAL" ? "#6E3030"
                                    : modelData.severity === "WARNING" ? "#665127" : "#293740"
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 7
                            Label { text: modelData.time; color: "#72838E"; font.pixelSize: 7 }
                            Label { text: modelData.type; color: "#9FC0D1"; font.pixelSize: 8; font.bold: true }
                            Label {
                                id: eventText
                                Layout.fillWidth: true
                                text: modelData.detail
                                color: "#C9D3D9"
                                font.pixelSize: 8
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }
        }
    }
}
