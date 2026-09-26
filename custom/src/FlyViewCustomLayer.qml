import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls

Item {
    id: root

    property var parentToolInsets
    property var totalToolInsets: parentToolInsets
    property var mapControl

    readonly property var activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    readonly property var primaryBattery: activeVehicle && activeVehicle.batteries && activeVehicle.batteries.count > 0
                                          ? activeVehicle.batteries.get(0)
                                          : null
    readonly property string vehicleMode: activeVehicle ? activeVehicle.flightMode : qsTr("NO VEHICLE")
    readonly property string batteryText: {
        if (!primaryBattery || isNaN(primaryBattery.percentRemaining.rawValue)) {
            return "--%"
        }
        return Math.round(primaryBattery.percentRemaining.rawValue) + "%"
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 12
        anchors.leftMargin: 12
        width: statusRow.implicitWidth + 28
        height: statusRow.implicitHeight + 18
        radius: 8
        color: "#D90B0F14"
        border.color: activeVehicle ? "#33495C" : "#6B3131"
        border.width: 1

        RowLayout {
            id: statusRow
            anchors.centerIn: parent
            spacing: 12

            Label {
                text: "NEXUS"
                color: "#F4F7FA"
                font.bold: true
            }

            Rectangle {
                width: 1
                height: 18
                color: "#33404D"
            }

            Label {
                text: activeVehicle ? qsTr("CONNECTED") : qsTr("DISCONNECTED")
                color: activeVehicle ? "#79D7A8" : "#FF8C8C"
                font.bold: true
            }

            Label {
                text: vehicleMode
                color: "#D9E2EA"
            }

            Label {
                text: batteryText
                color: "#D9E2EA"
            }
        }
    }
}
