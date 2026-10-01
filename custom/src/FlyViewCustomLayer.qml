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
    readonly property var planMasterController: globals.planMasterControllerFlyView
    readonly property var missionController: planMasterController ? planMasterController.missionController : null
    readonly property var primaryBattery: activeVehicle && activeVehicle.batteries && activeVehicle.batteries.count > 0
                                          ? activeVehicle.batteries.get(0)
                                          : null
    readonly property var healthReport: activeVehicle ? activeVehicle.healthAndArmingCheckReport : null

    readonly property bool linkLost: activeVehicle
                                     ? activeVehicle.vehicleLinkManager.communicationLost
                                     : false
    readonly property int gpsLock: activeVehicle && !linkLost ? Number(activeVehicle.gps.lock.rawValue) : 0
    readonly property int gpsSatCount: activeVehicle && !linkLost ? Number(activeVehicle.gps.count.rawValue) : 0
    readonly property bool gpsHealthy: activeVehicle && !linkLost && gpsLock >= 3

    readonly property real batteryPercent: primaryBattery && !linkLost
                                           ? Number(primaryBattery.percentRemaining.rawValue)
                                           : NaN
    readonly property bool batteryKnown: !isNaN(batteryPercent)
    readonly property bool batteryWarning: batteryKnown && batteryPercent < 25
    readonly property bool batteryCritical: batteryKnown && batteryPercent < 15

    readonly property bool preflightSupported: !!healthReport && healthReport.supported
    readonly property bool preflightBlocked: activeVehicle && !activeVehicle.armed &&
                                             ((preflightSupported && !healthReport.canArm) ||
                                              (!preflightSupported && activeVehicle.prearmError.length > 0))
    readonly property bool preflightWarning: activeVehicle && !activeVehicle.armed &&
                                             preflightSupported && healthReport.canArm &&
                                             healthReport.hasWarningsOrErrors

    readonly property string connectionText: !activeVehicle ? qsTr("DISCONNECTED")
                                                            : (linkLost ? qsTr("LINK LOST") : qsTr("CONNECTED"))
    readonly property string vehicleText: activeVehicle ? qsTr("UAV-%1").arg(activeVehicle.id) : qsTr("NO VEHICLE")
    readonly property string modeText: activeVehicle && !linkLost ? activeVehicle.flightMode : "--"
    readonly property string gpsText: !activeVehicle || linkLost ? "--"
                                                                : (gpsHealthy ? qsTr("%1 SAT").arg(gpsSatCount)
                                                                              : qsTr("NO FIX"))
    readonly property string navText: !activeVehicle || linkLost ? qsTr("UNKNOWN")
                                                                : (gpsHealthy ? qsTr("NOMINAL") : qsTr("DEGRADED"))
    readonly property string linkText: !activeVehicle ? "--"
                                                      : (linkLost ? qsTr("LOST") : activeVehicle.vehicleLinkManager.primaryLinkName)
    readonly property string batteryText: batteryKnown ? qsTr("%1%").arg(Math.round(batteryPercent)) : "--"

    readonly property int missionIndex: missionController && !linkLost ? missionController.currentMissionIndex : -1
    readonly property int missionVisualCount: missionController && missionController.visualItems
                                              ? missionController.visualItems.count
                                              : 0
    readonly property real missionProgress: missionController && !linkLost && !isNaN(Number(missionController.progressPct))
                                                ? Math.max(0, Math.min(1, Number(missionController.progressPct)))
                                                : 0
    readonly property string missionText: missionIndex >= 0
                                          ? qsTr("WP %1").arg(missionIndex)
                                          : qsTr("IDLE")

    function updateCentralPreflightMission() {
        if (!planMasterController) {
            NexusPreflight.updateMission(false, true, false, "NO MISSION", "Manual flight available")
            return
        }
        const status = NexusPlanVerifier.validationStatus(planMasterController)
        const hasMission = !!planMasterController.containsItems
        const valid = !hasMission || !!status.ready
        NexusPreflight.updateMission(hasMission,
                                     valid,
                                     NexusPlanVerifier.verified,
                                     status.state || "INVALID",
                                     status.message || "")
    }

    Timer {
        interval: 400
        running: true
        repeat: true
        onTriggered: root.updateCentralPreflightMission()
    }

    readonly property string preflightText: NexusPreflight.overallState
    readonly property color centralizedPreflightAccent: preflightText === "BLOCKED" ? "#E25555"
                                                     : preflightText === "WARNING" ? "#D6A84A"
                                                     : "#2C9B7F"

    readonly property string legacyPreflightText: {
        if (!activeVehicle || linkLost) return qsTr("UNKNOWN")
        if (activeVehicle.armed) return qsTr("ARMED")
        if (preflightBlocked) return qsTr("BLOCKED")
        if (preflightWarning) return qsTr("WARNING")
        return qsTr("READY")
    }

    readonly property color preflightAccent: centralizedPreflightAccent

    readonly property string videoText: {
        if (!QGroundControl.videoManager.hasVideo) return qsTr("VIDEO OFF")
        if (QGroundControl.videoManager.streaming && QGroundControl.videoManager.decoding) return qsTr("VIDEO LIVE")
        if (QGroundControl.videoManager.streaming) return qsTr("VIDEO WAIT")
        return qsTr("VIDEO READY")
    }

    readonly property color videoAccent: QGroundControl.videoManager.streaming && QGroundControl.videoManager.decoding
                                         ? "#2C9B7F"
                                         : (QGroundControl.videoManager.hasVideo ? "#D6A84A" : "#6F7E8C")

    readonly property string flightStateText: {
        if (!activeVehicle) return qsTr("WAITING FOR VEHICLE")
        if (linkLost) return qsTr("COMMUNICATION LOST")
        if (!activeVehicle.armed) return qsTr("PREFLIGHT")
        if (activeVehicle.armed && !activeVehicle.flying) return qsTr("ARMED")
        return activeVehicle.flightMode
    }

    readonly property string centralizedAlertText: NexusAlerts.currentMessage
    readonly property string centralizedAlertSeverity: NexusAlerts.highestSeverity

    readonly property string alertText: {
        if (centralizedAlertText.length > 0) return centralizedAlertText
        if (!activeVehicle) return qsTr("No active vehicle. Connect a UAV to begin.")
        if (linkLost) return qsTr("Vehicle communication lost. Live telemetry is hidden until the link recovers.")
        if (batteryCritical) return qsTr("Aircraft battery critically low.")
        if (preflightBlocked) {
            if (!preflightSupported && activeVehicle.prearmError.length > 0) return activeVehicle.prearmError
            return qsTr("Preflight checks are blocking arming.")
        }
        if (batteryWarning) return qsTr("Aircraft battery low.")
        if (!gpsHealthy) return qsTr("Navigation degraded: no 3D GPS lock.")
        if (preflightWarning) return qsTr("Preflight checks contain warnings.")
        return ""
    }

    readonly property string alertSeverity: {
        if (centralizedAlertSeverity !== "NONE") return centralizedAlertSeverity
        if (!activeVehicle) return "INFO"
        if (linkLost || batteryCritical || preflightBlocked) return "CRITICAL"
        if (batteryWarning || !gpsHealthy || preflightWarning) return "WARNING"
        return ""
    }

    readonly property color alertAccent: alertSeverity === "CRITICAL" ? "#E25555"
                                               : alertSeverity === "WARNING" ? "#D6A84A"
                                               : "#4F8FB8"

    property string commandFeedback: ""
    property color commandFeedbackAccent: "#4F8FB8"

    function factValue(fact, decimals) {
        if (!activeVehicle || linkLost || !fact || isNaN(Number(fact.value))) {
            return "--"
        }
        return Number(fact.value).toFixed(decimals)
    }

    function factUnits(fact) {
        return activeVehicle && !linkLost && fact && fact.units ? fact.units : ""
    }

    function factDisplay(fact, decimals) {
        const value = factValue(fact, decimals)
        if (value === "--") return value
        const units = factUnits(fact)
        return units && units.length > 0 ? value + " " + units : value
    }

    function confirmGuidedAction(actionCode) {
        if (!guidedController || !activeVehicle || linkLost) return
        guidedController.confirmAction(actionCode)
    }

    Timer {
        id: feedbackTimer
        interval: 6000
        repeat: false
        onTriggered: root.commandFeedback = ""
    }

    Connections {
        target: activeVehicle
        ignoreUnknownSignals: true

        function onMavCommandResult(vehicleId, targetComponent, command, ackResult, failureCode) {
            if (!root.activeVehicle || vehicleId !== root.activeVehicle.id) return
            root.commandFeedback = ackResult === 0
                                   ? qsTr("VEHICLE ACK · CMD %1 ACCEPTED").arg(command)
                                   : qsTr("VEHICLE ACK · CMD %1 REJECTED (%2)").arg(command).arg(ackResult)
            root.commandFeedbackAccent = ackResult === 0 ? "#2C9B7F" : "#E25555"
            feedbackTimer.restart()
        }
    }

    QGCToolInsets {
        id: toolInsets

        leftEdgeTopInset: parentToolInsets.leftEdgeTopInset
        leftEdgeCenterInset: parentToolInsets.leftEdgeCenterInset
        leftEdgeBottomInset: parentToolInsets.leftEdgeBottomInset

        rightEdgeTopInset: Math.max(parentToolInsets.rightEdgeTopInset,
                                    opsPanel.visible ? opsPanel.width + chromeMargin * 2 : 0)
        rightEdgeCenterInset: Math.max(parentToolInsets.rightEdgeCenterInset,
                                       opsPanel.visible ? opsPanel.width + chromeMargin * 2 : 0)
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
        objectName: "nexusStatusRibbon"

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
                Layout.preferredWidth: 104
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
                objectName: "nexusConnectionChip"
                label: qsTr("LINK")
                value: connectionText
                accentColor: !activeVehicle ? "#4F6575" : (linkLost ? "#D95151" : "#2C9B7F")
                Layout.fillWidth: true
            }

            NexusStatusChip {
                objectName: "nexusModeChip"
                label: qsTr("MODE")
                value: modeText
                accentColor: activeVehicle && !linkLost && activeVehicle.armed ? "#4F8FB8" : "#4F6575"
                Layout.fillWidth: true
            }

            NexusStatusChip {
                objectName: "nexusGpsChip"
                label: qsTr("GPS")
                value: gpsText
                accentColor: !activeVehicle || linkLost ? "#4F6575" : (gpsHealthy ? "#2C9B7F" : "#D6A84A")
                Layout.fillWidth: true
            }

            NexusStatusChip {
                objectName: "nexusNavChip"
                label: qsTr("NAV")
                value: navText
                accentColor: navText === qsTr("NOMINAL") ? "#2C9B7F"
                                                         : (navText === qsTr("DEGRADED") ? "#D6A84A" : "#4F6575")
                Layout.fillWidth: true
            }

            NexusStatusChip {
                objectName: "nexusBatteryChip"
                label: qsTr("BAT")
                value: batteryText
                accentColor: batteryCritical ? "#D95151"
                                             : (batteryWarning ? "#D6A84A"
                                                               : (batteryKnown ? "#2C9B7F" : "#4F6575"))
                Layout.fillWidth: true
            }

            NexusStatusChip {
                objectName: "nexusMissionChip"
                label: qsTr("MISSION")
                value: linkLost ? "--" : missionText
                accentColor: missionIndex >= 0 && !linkLost ? "#4F8FB8" : "#4F6575"
                Layout.fillWidth: true
            }
        }
    }

    Rectangle {
        id: alertBanner
        objectName: "nexusAlertBanner"

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

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: alertPanel.visible = true
        }

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

    NexusOpsPanel {
        id: opsPanel
        objectName: "nexusOpsPanel"

        visible: root.width >= 900
        width: Math.min(280, root.width * 0.23)

        anchors.top: alertBanner.visible ? alertBanner.bottom : statusRibbon.bottom
        anchors.topMargin: chromeMargin
        anchors.right: parent.right
        anchors.rightMargin: chromeMargin

        missionText: root.missionText
        missionProgress: root.missionProgress
        nextWaypointText: activeVehicle ? factDisplay(activeVehicle.distanceToNextWP, 0) : "--"
        homeText: activeVehicle ? factDisplay(activeVehicle.distanceToHome, 0) : "--"
        homeEtaText: activeVehicle ? factDisplay(activeVehicle.timeToHome, 0) : "--"
        batteryTimeText: primaryBattery ? factDisplay(primaryBattery.timeRemaining, 0) : "--"
        preflightText: root.preflightText
        preflightAccent: root.preflightAccent
        videoText: root.videoText
        videoAccent: root.videoAccent
    }

    Rectangle {
        id: feedbackBadge
        objectName: "nexusCommandFeedback"

        visible: commandFeedback.length > 0
        anchors.left: parent.left
        anchors.leftMargin: chromeMargin
        anchors.bottom: bottomChrome.top
        anchors.bottomMargin: 8

        implicitWidth: feedbackText.implicitWidth + 24
        height: 34
        radius: 8
        color: "#E60C1117"
        border.color: commandFeedbackAccent
        border.width: 1

        Label {
            id: feedbackText
            objectName: "nexusCommandFeedbackText"
            anchors.centerIn: parent
            text: commandFeedback
            color: "#EAF0F5"
            font.pixelSize: 10
            font.bold: true
        }
    }

    Rectangle {
        id: preflightBadge
        objectName: "nexusPreflightBadge"

        anchors.right: stateBadge.left
        anchors.rightMargin: 8
        anchors.bottom: bottomChrome.top
        anchors.bottomMargin: 8

        implicitWidth: preflightBadgeText.implicitWidth + 26
        height: 34
        radius: 8
        color: "#D70C1117"
        border.color: centralizedPreflightAccent
        border.width: 1

        Label {
            id: preflightBadgeText
            anchors.centerIn: parent
            text: qsTr("PREFLIGHT · %1").arg(NexusPreflight.overallState)
            color: centralizedPreflightAccent
            font.pixelSize: 10
            font.bold: true
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                alertPanel.visible = false
                healthPanel.visible = false
                preflightPanel.visible = true
            }
        }
    }

    Rectangle {
        id: stateBadge
        objectName: "nexusFlightStateBadge"

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
            objectName: "nexusFlightStateText"
            anchors.centerIn: parent
            text: flightStateText
            color: "#EAF0F5"
            font.pixelSize: 10
            font.bold: true
        }
    }

    NexusPayloadPanel {
        id: payloadPanel
        objectName: "nexusPayloadPanel"
        anchors.top: parent.top
        anchors.bottom: bottomChrome.top
        anchors.right: parent.right
        anchors.topMargin: topChromeBottom
        anchors.bottomMargin: 8
        anchors.rightMargin: 10
        visible: false
        payloadModel: NexusPayload
        mapControl: root.mapControl
        onCloseRequested: visible = false
    }

    NexusVehiclePanel {
        id: vehiclePanel
        objectName: "nexusVehiclePanel"
        anchors.top: parent.top
        anchors.bottom: bottomChrome.top
        anchors.right: parent.right
        anchors.topMargin: topChromeBottom
        anchors.bottomMargin: 8
        anchors.rightMargin: 10
        visible: false
        vehicleModel: NexusVehicle
        onCloseRequested: visible = false
        onOpenPayloadRequested: {
            visible = false
            payloadPanel.visible = true
        }
    }

    NexusEngineerPanel {
        id: engineerPanel
        objectName: "nexusEngineerPanel"
        anchors.top: parent.top
        anchors.bottom: bottomChrome.top
        anchors.right: parent.right
        anchors.topMargin: topChromeBottom
        anchors.bottomMargin: 8
        anchors.rightMargin: 10
        visible: false
        engineerModel: NexusEngineer
        onCloseRequested: visible = false
        onOpenHealthRequested: {
            visible = false
            healthPanel.visible = true
        }
    }

    NexusOfflinePanel {
        id: offlinePanel
        objectName: "nexusOfflinePanel"
        anchors.top: parent.top
        anchors.bottom: bottomChrome.top
        anchors.right: parent.right
        anchors.topMargin: topChromeBottom
        anchors.bottomMargin: 8
        anchors.rightMargin: 10
        visible: false
        offlineModel: NexusOffline
        onCloseRequested: visible = false
        onOpenPayloadRequested: {
            visible = false
            payloadPanel.visible = true
        }
        onOpenEngineerRequested: {
            visible = false
            engineerPanel.visible = true
        }
    }

    NexusAnalyzePanel {
        id: analyzePanel
        objectName: "nexusAnalyzePanel"
        anchors.top: parent.top
        anchors.bottom: bottomChrome.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: topChromeBottom
        anchors.bottomMargin: 8
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        visible: false
        analyzeModel: NexusAnalyze
        onCloseRequested: visible = false
    }

    NexusReportsPanel {
        id: reportsPanel
        objectName: "nexusReportsPanel"
        anchors.top: parent.top
        anchors.bottom: bottomChrome.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: topChromeBottom
        anchors.bottomMargin: 8
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        visible: false
        analyzeModel: NexusAnalyze
        reportsModel: NexusReports
        onCloseRequested: visible = false
    }

    NexusDeviceHealthPanel {
        id: deviceHealthPanel
        objectName: "nexusDeviceHealthPanel"
        anchors.top: parent.top
        anchors.bottom: bottomChrome.top
        anchors.right: parent.right
        anchors.topMargin: topChromeBottom
        anchors.bottomMargin: 8
        anchors.rightMargin: 10
        visible: false
        deviceModel: NexusDeviceHealth
        onCloseRequested: visible = false
    }

    NexusSecurityPanel {
        id: securityPanel
        objectName: "nexusSecurityPanel"
        anchors.top: parent.top
        anchors.bottom: bottomChrome.top
        anchors.right: parent.right
        anchors.topMargin: topChromeBottom
        anchors.bottomMargin: 8
        anchors.rightMargin: 10
        visible: false
        securityModel: NexusSecurity
        onCloseRequested: visible = false
    }

    NexusRecoveryPanel {
        id: recoveryPanel
        objectName: "nexusRecoveryPanel"
        anchors.top: parent.top
        anchors.bottom: bottomChrome.top
        anchors.right: parent.right
        anchors.topMargin: topChromeBottom
        anchors.bottomMargin: 8
        anchors.rightMargin: 10
        visible: recoveryModel.previousUncleanExit
        recoveryModel: NexusRecovery
        onCloseRequested: visible = false
    }

    NexusPreflightPanel {
        id: preflightPanel
        objectName: "nexusPreflightPanel"
        anchors.fill: parent
        visible: false
        preflightModel: NexusPreflight
        onCloseRequested: visible = false
    }

    NexusAlertPanel {
        id: alertPanel
        objectName: "nexusAlertPanel"
        anchors.fill: parent
        visible: false
        alertManager: NexusAlerts
        onCloseRequested: visible = false
    }

    NexusHealthPanel {
        id: healthPanel
        objectName: "nexusHealthPanel"
        anchors.fill: parent
        visible: false
        healthModel: NexusHealth
        onCloseRequested: visible = false
    }

    Column {
        id: bottomChrome
        objectName: "nexusBottomChrome"

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: chromeMargin
        anchors.rightMargin: chromeMargin
        anchors.bottomMargin: parentToolInsets.bottomEdgeCenterInset + chromeMargin
        spacing: 6

        Rectangle {
            id: telemetryBar
            objectName: "nexusTelemetryBar"
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
                    objectName: "nexusAltMetric"
                    label: qsTr("ALT")
                    value: activeVehicle ? factValue(activeVehicle.altitudeRelative, 1) : "--"
                    units: activeVehicle ? factUnits(activeVehicle.altitudeRelative) : ""
                    Layout.fillWidth: true
                }

                NexusMetric {
                    objectName: "nexusGroundSpeedMetric"
                    label: qsTr("GROUND SPEED")
                    value: activeVehicle ? factValue(activeVehicle.groundSpeed, 1) : "--"
                    units: activeVehicle ? factUnits(activeVehicle.groundSpeed) : ""
                    Layout.fillWidth: true
                }

                NexusMetric {
                    objectName: "nexusVertSpeedMetric"
                    label: qsTr("VERT SPEED")
                    value: activeVehicle ? factValue(activeVehicle.climbRate, 1) : "--"
                    units: activeVehicle ? factUnits(activeVehicle.climbRate) : ""
                    Layout.fillWidth: true
                }

                NexusMetric {
                    objectName: "nexusHeadingMetric"
                    label: qsTr("HEADING")
                    value: activeVehicle && !linkLost && !isNaN(Number(activeVehicle.heading.rawValue))
                           ? Math.round(Number(activeVehicle.heading.rawValue)).toString()
                           : "--"
                    units: activeVehicle && !linkLost ? "°" : ""
                    Layout.fillWidth: true
                }

                NexusMetric {
                    objectName: "nexusHomeMetric"
                    label: qsTr("HOME")
                    value: activeVehicle ? factValue(activeVehicle.distanceToHome, 0) : "--"
                    units: activeVehicle ? factUnits(activeVehicle.distanceToHome) : ""
                    Layout.fillWidth: true
                }

                NexusMetric {
                    objectName: "nexusNextWpMetric"
                    label: qsTr("NEXT WP")
                    value: activeVehicle ? factValue(activeVehicle.distanceToNextWP, 0) : "--"
                    units: activeVehicle ? factUnits(activeVehicle.distanceToNextWP) : ""
                    Layout.fillWidth: true
                }
            }
        }

        Rectangle {
            id: actionBar
            objectName: "nexusActionBar"
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
                    objectName: "nexusActionArm"
                    text: qsTr("ARM")
                    primary: true
                    visible: guidedController && guidedController.showArm
                    enabled: visible && !linkLost
                    Layout.fillWidth: true
                    onClicked: root.confirmGuidedAction(guidedController.actionArm)
                }

                NexusActionButton {
                    objectName: "nexusActionTakeoff"
                    text: qsTr("TAKEOFF")
                    primary: true
                    visible: guidedController && guidedController.showTakeoff
                    enabled: visible && !linkLost
                    Layout.fillWidth: true
                    onClicked: root.confirmGuidedAction(guidedController.actionTakeoff)
                }

                NexusActionButton {
                    objectName: "nexusActionHold"
                    text: qsTr("HOLD")
                    visible: guidedController && guidedController.showPause
                    enabled: visible && !linkLost
                    Layout.fillWidth: true
                    onClicked: root.confirmGuidedAction(guidedController.actionPause)
                }

                NexusActionButton {
                    objectName: "nexusActionRTL"
                    text: qsTr("RTL")
                    visible: !!activeVehicle
                    enabled: guidedController && guidedController.showRTL && !linkLost
                    Layout.fillWidth: true
                    onClicked: root.confirmGuidedAction(guidedController.actionRTL)
                }

                NexusActionButton {
                    objectName: "nexusActionLand"
                    text: qsTr("LAND")
                    critical: true
                    visible: guidedController && guidedController.showLand
                    enabled: visible && !linkLost
                    Layout.fillWidth: true
                    onClicked: root.confirmGuidedAction(guidedController.actionLand)
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

                Label {
                    visible: activeVehicle && !linkLost &&
                             !(guidedController && (guidedController.showArm ||
                                                    guidedController.showTakeoff ||
                                                    guidedController.showPause ||
                                                    guidedController.showRTL ||
                                                    guidedController.showLand))
                    text: qsTr("No guided action available in the current vehicle state")
                    color: "#7F8C98"
                    font.pixelSize: 10
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        Rectangle {
            id: navBar
            objectName: "nexusBottomNav"
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

                NexusNavItem { text: qsTr("FLIGHT"); active: !healthPanel.visible && !payloadPanel.visible && !vehiclePanel.visible && !engineerPanel.visible && !offlinePanel.visible && !analyzePanel.visible && !reportsPanel.visible && !deviceHealthPanel.visible && !securityPanel.visible && !recoveryPanel.visible && !alertPanel.visible && !preflightPanel.visible; Layout.fillWidth: true; onClicked: { healthPanel.visible = false; alertPanel.visible = false; preflightPanel.visible = false; payloadPanel.visible = false; vehiclePanel.visible = false; engineerPanel.visible = false; offlinePanel.visible = false; analyzePanel.visible = false; reportsPanel.visible = false; deviceHealthPanel.visible = false; securityPanel.visible = false; recoveryPanel.visible = false } }
                NexusNavItem { text: qsTr("PLAN"); Layout.fillWidth: true; onClicked: { if (mainWindow.allowViewSwitch()) mainWindow.showPlanView() } }
                NexusNavItem { text: qsTr("HEALTH"); active: healthPanel.visible; Layout.fillWidth: true; onClicked: { alertPanel.visible = false; preflightPanel.visible = false; payloadPanel.visible = false; vehiclePanel.visible = false; engineerPanel.visible = false; offlinePanel.visible = false; analyzePanel.visible = false; reportsPanel.visible = false; deviceHealthPanel.visible = false; securityPanel.visible = false; recoveryPanel.visible = false; healthPanel.visible = true } }
                NexusNavItem { text: qsTr("PAYLOAD"); active: payloadPanel.visible; Layout.fillWidth: true; onClicked: { alertPanel.visible = false; preflightPanel.visible = false; healthPanel.visible = false; vehiclePanel.visible = false; engineerPanel.visible = false; offlinePanel.visible = false; analyzePanel.visible = false; reportsPanel.visible = false; deviceHealthPanel.visible = false; securityPanel.visible = false; recoveryPanel.visible = false; payloadPanel.visible = !payloadPanel.visible } }
                NexusNavItem { text: qsTr("ANALYZE"); active: analyzePanel.visible; Layout.fillWidth: true; onClicked: { alertPanel.visible = false; preflightPanel.visible = false; healthPanel.visible = false; payloadPanel.visible = false; vehiclePanel.visible = false; engineerPanel.visible = false; offlinePanel.visible = false; reportsPanel.visible = false; deviceHealthPanel.visible = false; securityPanel.visible = false; recoveryPanel.visible = false; analyzePanel.visible = !analyzePanel.visible } }
                NexusNavItem { text: qsTr("VEHICLE"); active: vehiclePanel.visible; Layout.fillWidth: true; onClicked: { alertPanel.visible = false; preflightPanel.visible = false; healthPanel.visible = false; payloadPanel.visible = false; engineerPanel.visible = false; offlinePanel.visible = false; analyzePanel.visible = false; reportsPanel.visible = false; deviceHealthPanel.visible = false; securityPanel.visible = false; recoveryPanel.visible = false; vehiclePanel.visible = !vehiclePanel.visible } }
                NexusNavItem { text: qsTr("ENGINEER"); active: engineerPanel.visible; Layout.fillWidth: true; onClicked: { alertPanel.visible = false; preflightPanel.visible = false; healthPanel.visible = false; payloadPanel.visible = false; vehiclePanel.visible = false; offlinePanel.visible = false; analyzePanel.visible = false; reportsPanel.visible = false; deviceHealthPanel.visible = false; securityPanel.visible = false; recoveryPanel.visible = false; engineerPanel.visible = !engineerPanel.visible } }
                NexusNavItem { text: qsTr("OFFLINE"); active: offlinePanel.visible; Layout.fillWidth: true; onClicked: { alertPanel.visible = false; preflightPanel.visible = false; healthPanel.visible = false; payloadPanel.visible = false; vehiclePanel.visible = false; engineerPanel.visible = false; analyzePanel.visible = false; reportsPanel.visible = false; deviceHealthPanel.visible = false; securityPanel.visible = false; recoveryPanel.visible = false; offlinePanel.visible = !offlinePanel.visible } }
                NexusNavItem { text: qsTr("REPORTS"); active: reportsPanel.visible; Layout.fillWidth: true; onClicked: { alertPanel.visible = false; preflightPanel.visible = false; healthPanel.visible = false; payloadPanel.visible = false; vehiclePanel.visible = false; engineerPanel.visible = false; offlinePanel.visible = false; analyzePanel.visible = false; deviceHealthPanel.visible = false; securityPanel.visible = false; recoveryPanel.visible = false; reportsPanel.visible = !reportsPanel.visible } }
                NexusNavItem { text: qsTr("DEVICE"); active: deviceHealthPanel.visible; Layout.fillWidth: true; onClicked: { alertPanel.visible = false; preflightPanel.visible = false; healthPanel.visible = false; payloadPanel.visible = false; vehiclePanel.visible = false; engineerPanel.visible = false; offlinePanel.visible = false; analyzePanel.visible = false; reportsPanel.visible = false; securityPanel.visible = false; recoveryPanel.visible = false; deviceHealthPanel.visible = !deviceHealthPanel.visible } }
                NexusNavItem { text: qsTr("SECURITY"); active: securityPanel.visible; Layout.fillWidth: true; onClicked: { alertPanel.visible = false; preflightPanel.visible = false; healthPanel.visible = false; payloadPanel.visible = false; vehiclePanel.visible = false; engineerPanel.visible = false; offlinePanel.visible = false; analyzePanel.visible = false; reportsPanel.visible = false; deviceHealthPanel.visible = false; recoveryPanel.visible = false; securityPanel.visible = !securityPanel.visible } }
                NexusNavItem { text: qsTr("RECOVERY"); active: recoveryPanel.visible; Layout.fillWidth: true; onClicked: { alertPanel.visible = false; preflightPanel.visible = false; healthPanel.visible = false; payloadPanel.visible = false; vehiclePanel.visible = false; engineerPanel.visible = false; offlinePanel.visible = false; analyzePanel.visible = false; reportsPanel.visible = false; deviceHealthPanel.visible = false; securityPanel.visible = false; recoveryPanel.visible = !recoveryPanel.visible } }
            }
        }
    }
}
