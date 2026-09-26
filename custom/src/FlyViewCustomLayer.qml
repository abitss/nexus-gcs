import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.FlyView

Item {
    id: root

    property var parentToolInsets
    property var totalToolInsets: toolInsets
    property var mapControl

    readonly property var activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    readonly property var guidedController: globals.guidedControllerFlyView
    readonly property var primaryBattery: activeVehicle && activeVehicle.batteries && activeVehicle.batteries.count > 0
                                          ? activeVehicle.batteries.get(0)
                                          : null

    readonly property bool linkLost: activeVehicle
                                     ? activeVehicle.vehicleLinkManager.communicationLost
                                     : false
    readonly property int gpsLock: activeVehicle ? Number(activeVehicle.gps.lock.rawValue) : 0
    readonly property int gpsSatCount: activeVehicle ? Number(activeVehicle.gps.count.rawValue) : 0
    readonly property bool gpsHealthy: activeVehicle && gpsLock >= 3
    readonly property real batteryPercent: primaryBattery ? Number(primaryBattery.percentRemaining.rawValue) : NaN
    readonly property bool batteryKnown: !isNaN(batteryPercent)
    readonly property bool batteryWarning: batteryKnown && batteryPercent < 25
    readonly property bool batteryCritical: batteryKnown && batteryPercent < 15

    readonly property string connectionText: !activeVehicle ? qsTr("DISCONNECTED")
                                                            : (linkLost ? qsTr("LINK LOST") : qsTr("CONNECTED"))
    readonly property string vehicleText: activeVehicle ? qsTr("UAV-%1").arg(activeVehicle.id) : qsTr("NO VEHICLE")
    readonly property string modeText: activeVehicle ? activeVehicle.flightMode : "--"
    readonly property string gpsText: !activeVehicle ? "--"
                                                     : (gpsHealthy ? qsTr("%1 SAT").arg(gpsSatCount)
                                                                   : qsTr("NO FIX"))
    readonly property string linkText: !activeVehicle ? "--"
                                                      : (linkLost ? qsTr("LOST") : activeVehicle.vehicleLinkManager.primaryLinkName)
    readonly property string batteryText: batteryKnown ? qsTr("%1%").arg(Math.round(batteryPercent)) : "--"

    readonly property string flightStateText: {
        if (!activeVehicle) {
            return qsTr("WAITING FOR VEHICLE")
        }
        if (linkLost) {
            return qsTr("COMMUNICATION LOST")
        }
        if (!activeVehicle.armed) {
            return qsTr("PREFLIGHT")
        }
        if (activeVehicle.armed && !activeVehicle.flying) {
            return qsTr("ARMED")
        }
        return activeVehicle.flightMode
    }

    readonly property string alertText: {
        if (!activeVehicle) {
            return qsTr("No active vehicle. Connect a UAV to begin.")
        }
        if (linkLost) {
            return qsTr("Vehicle communication lost. Telemetry may be stale.")
        }
        if (batteryCritical) {
            return qsTr("Aircraft battery critically low.")
        }
        if (batteryWarning) {
            return qsTr("Aircraft battery low.")
        }
        if (!gpsHealthy) {
            return qsTr("Navigation degraded: no 3D GPS lock.")
        }
        return ""
    }

    readonly property string alertSeverity: {
        if (!activeVehicle) return "INFO"
        if (linkLost || batteryCritical) return "CRITICAL"
        if (batteryWarning || !gpsHealthy) return "WARNING"
        return ""
    }

    readonly property color alertAccent: alertSeverity === "CRITICAL" ? "#E25555"
                                                : alertSeverity === "WARNING" ? "#D6A84A"
                                                : "#4F8FB8"

    function factValue(fact, decimals) {
        if (!fact || isNaN(Number(fact.value))) {
            return "--"
        }
        return Number(fact.value).toFixed(decimals)
    }

    function factUnits(fact) {
        return fact && fact.units ? fact.units : ""
    }

    QGCToolInsets {
        id: toolInsets

        leftEdgeTopInset: parentToolInsets.leftEdgeTopInset
        leftEdgeCenterInset: parentToolInsets.leftEdgeCenterInset
        leftEdgeBottomInset: parentToolInsets.leftEdgeBottomInset
        rightEdgeTopInset: parentToolInsets.rightEdgeTopInset
        rightEdgeCenterInset: parentToolInsets.rightEdgeCenterInset
        rightEdgeBottomInset: parentToolInsets.rightEdgeBottomInset

        topEdgeLeftInset: Math.max(parentToolInsets.topEdgeLeftInset, topChromeBottom)
        topEdgeCenterInset: Math.max(parentToolInsets.topEdgeCenterInset, topChromeBottom)
        topEdgeRightInset: Math.max(parentToolInsets.topEdgeRightInset, topChromeBottom)

        bottomEdgeLeftInset: Math.max(parentToolInsets.bottomEdgeLeftInset, bottomChromeInset)
        bottomEdgeCenterInset: Math.max(parentToolInsets.bottomEdgeCenterInset, bottomChromeInset)
        bottomEdgeRightInset: Math.max(parentToolInsets.bottomEdgeRightInset, bottomChromeInset)
    }

    readonly property real chromeMargin: 10
    readonly property real topChromeBottom: alertBanner.visible
                                            ? alertBanner.y + alertBanner.height + chromeMargin
                                            : statusRibbon.y + statusRibbon.height + chromeMargin
    readonly property real bottomChromeInset: parent.height - bottomChrome.y + chromeMargin

    Rectangle {
        id: statusRibbon

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: parentToolInsets.topEdgeCenterInset + chromeMargin
        anchors.leftMargin: chromeMargin
        anchors.rightMargin: chromeMargin

        height: 52
        radius: 10
        color: "#E60A0E13"
        border.color: "#24313C"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 8

            ColumnLayout {
                Layout.preferredWidth: 110
                spacing: 0

                Label {
                    text: "NEXUS"
                    color: "#F6FAFC"
                    font.pixelSize: 18
                    font.bold: true
                }

                Label {
                    text: vehicleText
                    color: "#7F8D99"
                    font.pixelSize: 9
                    font.bold: true
                }
            }

            NexusStatusChip {
                label: qsTr("LINK")
                value: connectionText
                accentColor: !activeVehicle ? "#4F6575" : (linkLost ? "#D95151" : "#2C9B7F")
                Layout.fillWidth: true
            }

            NexusStatusChip {
                label: qsTr("MODE")
                value: modeText
                accentColor: activeVehicle && activeVehicle.armed ? "#4F8FB8" : "#4F6575"
                Layout.fillWidth: true
            }

            NexusStatusChip {
                label: qsTr("GPS")
                value: gpsText
                accentColor: !activeVehicle ? "#4F6575" : (gpsHealthy ? "#2C9B7F" : "#D6A84A")
                Layout.fillWidth: true
            }

            NexusStatusChip {
                label: qsTr("DATA")
                value: linkText && linkText.length > 0 ? linkText : "--"
                accentColor: linkLost ? "#D95151" : "#4F8FB8"
                Layout.fillWidth: true
            }

            NexusStatusChip {
                label: qsTr("BAT")
                value: batteryText
                accentColor: batteryCritical ? "#D95151"
                                             : (batteryWarning ? "#D6A84A"
                                                               : (batteryKnown ? "#2C9B7F" : "#4F6575"))
                Layout.fillWidth: true
            }
        }
    }

    Rectangle {
        id: alertBanner

        visible: alertText.length > 0
        anchors.top: statusRibbon.bottom
        anchors.left: statusRibbon.left
        anchors.right: statusRibbon.right
        anchors.topMargin: 7

        height: visible ? 34 : 0
        radius: 8
        color: "#E6151A20"
        border.color: alertAccent
        border.width: 1

        Row {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 10

            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: alertAccent
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                text: alertSeverity
                color: alertAccent
                font.pixelSize: 10
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                text: alertText
                color: "#E5EBF0"
                font.pixelSize: 11
                elide: Text.ElideRight
                width: alertBanner.width - 150
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Rectangle {
        id: stateBadge

        anchors.right: parent.right
        anchors.rightMargin: chromeMargin
        anchors.bottom: bottomChrome.top
        anchors.bottomMargin: 8

        implicitWidth: stateText.implicitWidth + 24
        height: 34
        radius: 8
        color: "#D70C1117"
        border.color: linkLost ? "#D95151"
                               : (activeVehicle && activeVehicle.armed ? "#4F8FB8" : "#4F6575")
        border.width: 1

        Label {
            id: stateText
            anchors.centerIn: parent
            text: flightStateText
            color: "#EAF0F5"
            font.pixelSize: 10
            font.bold: true
        }
    }

    Column {
        id: bottomChrome

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: chromeMargin
        anchors.rightMargin: chromeMargin
        anchors.bottomMargin: parentToolInsets.bottomEdgeCenterInset + chromeMargin
        spacing: 6

        Rectangle {
            width: parent.width
            height: 62
            radius: 10
            color: "#E60A0E13"
            border.color: "#24313C"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 6

                NexusMetric {
                    label: qsTr("ALT")
                    value: activeVehicle ? factValue(activeVehicle.altitudeRelative, 1) : "--"
                    units: activeVehicle ? factUnits(activeVehicle.altitudeRelative) : ""
                    Layout.fillWidth: true
                }

                NexusMetric {
                    label: qsTr("GROUND SPEED")
                    value: activeVehicle ? factValue(activeVehicle.groundSpeed, 1) : "--"
                    units: activeVehicle ? factUnits(activeVehicle.groundSpeed) : ""
                    Layout.fillWidth: true
                }

                NexusMetric {
                    label: qsTr("VERT SPEED")
                    value: activeVehicle ? factValue(activeVehicle.climbRate, 1) : "--"
                    units: activeVehicle ? factUnits(activeVehicle.climbRate) : ""
                    Layout.fillWidth: true
                }

                NexusMetric {
                    label: qsTr("HEADING")
                    value: activeVehicle && !isNaN(Number(activeVehicle.heading.rawValue))
                           ? Math.round(Number(activeVehicle.heading.rawValue)).toString()
                           : "--"
                    units: activeVehicle ? "°" : ""
                    Layout.fillWidth: true
                }

                NexusMetric {
                    label: qsTr("HOME")
                    value: activeVehicle ? factValue(activeVehicle.distanceToHome, 0) : "--"
                    units: activeVehicle ? factUnits(activeVehicle.distanceToHome) : ""
                    Layout.fillWidth: true
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 50
            radius: 10
            color: "#EE0B1016"
            border.color: "#24313C"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 7

                Label {
                    text: qsTr("ACTIONS")
                    color: "#7F8C98"
                    font.pixelSize: 9
                    font.bold: true
                    Layout.preferredWidth: 62
                    Layout.alignment: Qt.AlignVCenter
                }

                NexusActionButton {
                    text: qsTr("ARM")
                    primary: true
                    visible: guidedController && guidedController.showArm
                    enabled: visible && !linkLost
                    Layout.fillWidth: true
                    onClicked: guidedController.confirmAction(guidedController.actionArm)
                }

                NexusActionButton {
                    text: qsTr("TAKEOFF")
                    primary: true
                    visible: guidedController && guidedController.showTakeoff
                    enabled: visible && !linkLost
                    Layout.fillWidth: true
                    onClicked: guidedController.confirmAction(guidedController.actionTakeoff)
                }

                NexusActionButton {
                    text: qsTr("HOLD")
                    visible: guidedController && guidedController.showPause
                    enabled: visible && !linkLost
                    Layout.fillWidth: true
                    onClicked: guidedController.confirmAction(guidedController.actionPause)
                }

                NexusActionButton {
                    text: qsTr("RTL")
                    visible: !!activeVehicle
                    enabled: guidedController && guidedController.showRTL && !linkLost
                    Layout.fillWidth: true
                    onClicked: guidedController.confirmAction(guidedController.actionRTL)
                }

                NexusActionButton {
                    text: qsTr("LAND")
                    critical: true
                    visible: guidedController && guidedController.showLand
                    enabled: visible && !linkLost
                    Layout.fillWidth: true
                    onClicked: guidedController.confirmAction(guidedController.actionLand)
                }

                Label {
                    visible: !activeVehicle
                    text: qsTr("Connect a vehicle to enable flight actions")
                    color: "#7F8C98"
                    font.pixelSize: 10
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 48
            radius: 10
            color: "#F0090D12"
            border.color: "#24313C"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 6

                NexusNavItem { text: qsTr("FLIGHT"); active: true; Layout.fillWidth: true }
                NexusNavItem { text: qsTr("PLAN"); Layout.fillWidth: true }
                NexusNavItem { text: qsTr("HEALTH"); Layout.fillWidth: true }
                NexusNavItem { text: qsTr("PAYLOAD"); Layout.fillWidth: true }
                NexusNavItem { text: qsTr("ANALYZE"); Layout.fillWidth: true }
                NexusNavItem { text: qsTr("VEHICLE"); Layout.fillWidth: true }
            }
        }
    }
}
