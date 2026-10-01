import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property var healthModel
    signal closeRequested()

    color: "#F50B0F14"
    z: 5000

    function stateColor(state) {
        if (state === "CRITICAL") return "#D95151"
        if (state === "DEGRADED") return "#D6A84A"
        if (state === "NOMINAL") return "#2C9B7F"
        return "#4F6575"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 12

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 76
            radius: 12
            color: "#111820"
            border.width: 1
            border.color: root.stateColor(healthModel ? healthModel.overallState : "UNKNOWN")

            RowLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 14

                ColumnLayout {
                    spacing: 2
                    Label {
                        text: "NEXUS HEALTH"
                        color: "#F6FAFC"
                        font.pixelSize: 20
                        font.bold: true
                    }
                    Label {
                        text: healthModel ? healthModel.overallDetail : "Health model unavailable"
                        color: "#98A5B0"
                        font.pixelSize: 10
                    }
                }

                Item { Layout.fillWidth: true }

                Rectangle {
                    implicitWidth: 148
                    implicitHeight: 38
                    radius: 8
                    color: "#0B1016"
                    border.color: root.stateColor(healthModel ? healthModel.overallState : "UNKNOWN")

                    Label {
                        anchors.centerIn: parent
                        text: healthModel ? healthModel.overallState : "UNKNOWN"
                        color: root.stateColor(text)
                        font.pixelSize: 13
                        font.bold: true
                    }
                }

                Button {
                    text: qsTr("RETURN TO FLIGHT")
                    onClicked: root.closeRequested()
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: root.width >= 1100 ? 4 : 3
            columnSpacing: 10
            rowSpacing: 10

            NexusHealthCard { title: "GPS"; state: healthModel.gpsState; detail: healthModel.gpsDetail; Layout.fillWidth: true }
            NexusHealthCard { title: "EKF / ESTIMATOR"; state: healthModel.ekfState; detail: healthModel.ekfDetail; Layout.fillWidth: true }
            NexusHealthCard { title: "IMU"; state: healthModel.imuState; detail: healthModel.imuDetail; Layout.fillWidth: true }
            NexusHealthCard { title: "COMPASS"; state: healthModel.compassState; detail: healthModel.compassDetail; Layout.fillWidth: true }
            NexusHealthCard { title: "GYRO"; state: healthModel.gyroState; detail: healthModel.gyroDetail; Layout.fillWidth: true }
            NexusHealthCard { title: "ACCELEROMETER"; state: healthModel.accelerometerState; detail: healthModel.accelerometerDetail; Layout.fillWidth: true }
            NexusHealthCard { title: "BAROMETER"; state: healthModel.barometerState; detail: healthModel.barometerDetail; Layout.fillWidth: true }
            NexusHealthCard { title: "BATTERY"; state: healthModel.batteryState; detail: healthModel.batteryDetail; Layout.fillWidth: true }
            NexusHealthCard { title: "DATALINK"; state: healthModel.datalinkState; detail: healthModel.datalinkDetail; Layout.fillWidth: true }
            NexusHealthCard { title: "HOME"; state: healthModel.homeState; detail: healthModel.homeDetail; Layout.fillWidth: true }
            NexusHealthCard { title: "GEOFENCE"; state: healthModel.geofenceState; detail: healthModel.geofenceDetail; Layout.fillWidth: true }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 10
            color: "#10171E"
            border.color: healthModel && healthModel.sensorFailures.length > 0 ? "#D95151" : "#273540"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: qsTr("SENSOR FAILURES")
                        color: "#B9C4CC"
                        font.pixelSize: 10
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: healthModel ? healthModel.sensorFailures.length : 0
                        color: healthModel && healthModel.sensorFailures.length > 0 ? "#FF9C9C" : "#A8F3D6"
                        font.bold: true
                    }
                }

                Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: !healthModel || healthModel.sensorFailures.length === 0
                          ? qsTr("No SYS_STATUS sensor failures reported.")
                          : healthModel.sensorFailures.join("\n")
                    color: "#E5EBF0"
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignTop
                }
            }
        }
    }
}
