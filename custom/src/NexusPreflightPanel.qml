import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var preflightModel
    signal closeRequested()

    color: "#F50B0F14"
    z: 7000

    function stateColor(state) {
        if (state === "BLOCKED") return "#D95151"
        if (state === "WARNING") return "#D6A84A"
        return "#2C9B7F"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 12

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 82
            radius: 12
            color: "#111820"
            border.width: 1
            border.color: root.stateColor(preflightModel.overallState)

            RowLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 14

                ColumnLayout {
                    spacing: 2
                    Label { text: "NEXUS PREFLIGHT"; color: "#F6FAFC"; font.pixelSize: 20; font.bold: true }
                    Label { text: preflightModel.overallDetail; color: "#98A5B0"; font.pixelSize: 10 }
                }

                Item { Layout.fillWidth: true }

                Rectangle {
                    implicitWidth: 130
                    implicitHeight: 40
                    radius: 8
                    color: "#0B1016"
                    border.color: root.stateColor(preflightModel.overallState)
                    Label {
                        anchors.centerIn: parent
                        text: preflightModel.overallState
                        color: root.stateColor(text)
                        font.pixelSize: 14
                        font.bold: true
                    }
                }

                Label {
                    text: qsTr("%1 blocked · %2 warning").arg(preflightModel.blockedCount).arg(preflightModel.warningCount)
                    color: "#98A5B0"
                    font.pixelSize: 10
                }

                Button {
                    text: qsTr("RETURN")
                    onClicked: root.closeRequested()
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: root.width >= 1100 ? 3 : 2
            columnSpacing: 10
            rowSpacing: 10

            NexusPreflightCard { title: "VEHICLE"; state: preflightModel.vehicleState; detail: preflightModel.vehicleDetail; Layout.fillWidth: true }
            NexusPreflightCard { title: "SENSORS"; state: preflightModel.sensorsState; detail: preflightModel.sensorsDetail; Layout.fillWidth: true }
            NexusPreflightCard { title: "GPS / NAVIGATION"; state: preflightModel.navigationState; detail: preflightModel.navigationDetail; Layout.fillWidth: true }
            NexusPreflightCard { title: "BATTERY"; state: preflightModel.batteryState; detail: preflightModel.batteryDetail; Layout.fillWidth: true }
            NexusPreflightCard { title: "HOME"; state: preflightModel.homeState; detail: preflightModel.homeDetail; Layout.fillWidth: true }
            NexusPreflightCard { title: "MISSION"; state: preflightModel.missionState; detail: preflightModel.missionDetail; Layout.fillWidth: true }
            NexusPreflightCard { title: "GEOFENCE"; state: preflightModel.geofenceState; detail: preflightModel.geofenceDetail; Layout.fillWidth: true }
            NexusPreflightCard { title: "DATALINK"; state: preflightModel.datalinkState; detail: preflightModel.datalinkDetail; Layout.fillWidth: true }
            NexusPreflightCard { title: "PAYLOAD"; state: preflightModel.payloadState; detail: preflightModel.payloadDetail; Layout.fillWidth: true }
        }
    }
}
