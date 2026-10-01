import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property var deviceModel
    signal closeRequested()

    width: Math.min(620, parent ? parent.width * 0.52 : 620)
    color: "#F60A0F14"
    border.color: "#2B3944"
    border.width: 1
    radius: 12
    z: 7000

    component HealthCard: Rectangle {
        id: card
        property string title
        property string value
        property string detail
        property string state: "NOMINAL"
        property string actionText: ""
        signal action()

        Layout.fillWidth: true
        implicitHeight: 94
        radius: 9
        color: "#10171E"
        border.color: state === "CRITICAL" ? "#D95151"
                    : state === "WARNING" ? "#D6A84A"
                    : "#2B3944"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 3
            RowLayout {
                Layout.fillWidth: true
                Label { text: card.title; color: "#F0F4F7"; font.pixelSize: 10; font.bold: true }
                Item { Layout.fillWidth: true }
                Label {
                    text: card.state
                    color: card.state === "CRITICAL" ? "#E36A6A"
                         : card.state === "WARNING" ? "#E0B85F"
                         : "#78B7A1"
                    font.pixelSize: 8
                    font.bold: true
                }
            }
            Label { text: card.value; color: "#EDF3F6"; font.pixelSize: 14; font.bold: true }
            Label {
                Layout.fillWidth: true
                text: card.detail
                color: "#83919B"
                font.pixelSize: 8
                elide: Text.ElideRight
            }
            Button {
                visible: card.actionText.length > 0
                text: card.actionText
                Layout.alignment: Qt.AlignRight
                onClicked: card.action()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 1
                Label { text: qsTr("DEVICE HEALTH"); color: "#F6FAFC"; font.pixelSize: 19; font.bold: true }
                Label {
                    text: deviceModel.android ? qsTr("ANDROID GROUND STATION") : qsTr("HOST DEVICE")
                    color: "#80909A"
                    font.pixelSize: 9
                    font.bold: true
                }
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                implicitWidth: 96
                implicitHeight: 30
                radius: 7
                color: "#0D141A"
                border.color: deviceModel.overallState === "CRITICAL" ? "#D95151"
                            : deviceModel.overallState === "WARNING" ? "#D6A84A"
                            : "#2C9B7F"
                Label {
                    anchors.centerIn: parent
                    text: deviceModel.overallState
                    color: parent.border.color
                    font.pixelSize: 9
                    font.bold: true
                }
            }
            Button { text: "×"; onClicked: root.closeRequested() }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            GridLayout {
                width: parent.width
                columns: root.width >= 540 ? 2 : 1
                columnSpacing: 8
                rowSpacing: 8

                HealthCard {
                    title: qsTr("TABLET BATTERY")
                    value: deviceModel.batteryPercent >= 0 ? deviceModel.batteryPercent + "%" : qsTr("UNKNOWN")
                    detail: deviceModel.batteryState
                    state: deviceModel.lowBatteryWarning ? "WARNING" : "NOMINAL"
                }

                HealthCard {
                    title: qsTr("TEMPERATURE")
                    value: isNaN(deviceModel.temperatureC) ? qsTr("UNAVAILABLE") : deviceModel.temperatureC.toFixed(1) + " °C"
                    detail: qsTr("Thermal state: %1").arg(deviceModel.thermalState)
                    state: deviceModel.overheatingWarning ? "CRITICAL" : "NOMINAL"
                }

                HealthCard {
                    title: qsTr("STORAGE")
                    value: deviceModel.storageFreeText
                    detail: qsTr("Free local storage")
                    state: deviceModel.lowStorageWarning ? "WARNING" : "NOMINAL"
                }

                HealthCard {
                    title: qsTr("RAM")
                    value: deviceModel.ramAvailableMb >= 0
                           ? qsTr("%1 MB free").arg(deviceModel.ramAvailableMb)
                           : qsTr("UNKNOWN")
                    detail: deviceModel.ramTotalMb > 0 ? qsTr("%1 MB total").arg(deviceModel.ramTotalMb) : qsTr("Total unavailable")
                }

                HealthCard {
                    title: qsTr("USB")
                    value: deviceModel.usbConnected ? qsTr("CONNECTED") : qsTr("NO DEVICE")
                    detail: deviceModel.usbPorts.length > 0 ? deviceModel.usbPorts.join(", ") : qsTr("No enumerated serial/USB device")
                    state: deviceModel.usbConnected ? "NOMINAL" : "WARNING"
                }

                HealthCard {
                    title: qsTr("NETWORK")
                    value: deviceModel.networkState
                    detail: qsTr("Internet reachability only. Core NEXUS operation remains offline-first.")
                }

                HealthCard {
                    title: qsTr("CAMERA PERMISSION")
                    value: deviceModel.cameraPermission
                    detail: qsTr("Needed only for supported local/UVC camera workflows.")
                    state: deviceModel.cameraPermission === "DENIED" ? "WARNING" : "NOMINAL"
                    actionText: deviceModel.cameraPermission === "GRANTED" ? "" : qsTr("REQUEST")
                    onAction: deviceModel.requestCameraPermission()
                }

                HealthCard {
                    title: qsTr("LOCATION PERMISSION")
                    value: deviceModel.locationPermission
                    detail: qsTr("Used for ground-station/device positioning where enabled.")
                    state: deviceModel.locationPermission === "DENIED" ? "WARNING" : "NOMINAL"
                    actionText: deviceModel.locationPermission === "GRANTED" ? "" : qsTr("REQUEST")
                    onAction: deviceModel.requestLocationPermission()
                }

                HealthCard {
                    title: qsTr("STORAGE PERMISSION")
                    value: deviceModel.storagePermission
                    detail: qsTr("Android external-storage access when applicable.")
                    state: deviceModel.storagePermission === "DENIED" ? "WARNING" : "NOMINAL"
                }

                HealthCard {
                    title: qsTr("SCREEN SLEEP")
                    value: deviceModel.keepScreenAwake ? qsTr("BLOCKED DURING OPS") : qsTr("SYSTEM DEFAULT")
                    detail: qsTr("NEXUS keeps the display awake during active vehicle operation, then restores normal device behavior.")
                }
            }
        }
    }
}
