import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning

import QGroundControl
import QGroundControl.Controls
import QGroundControl.FlightMap
import QGroundControl.LogViewer
import "NexusTokens.js" as T

Rectangle {
    id: root

    property var analyzeModel
    signal closeRequested()

    width: parent ? Math.min(1040, Math.max(360, parent.width - 20)) : 980
    color: "#F70A0F14"
    border.color: "#2B3944"
    border.width: 1
    radius: 12
    z: 6900

    property string pendingLog: ""
    property int selectedTab: 0

    function loadSelected() {
        logParser.clear()
        pendingLog = ""
        if (!analyzeModel.selectedFirmwareLog) return
        pendingLog = analyzeModel.selectedPath
        logParser.startParsingAsync(pendingLog)
    }

    function startReplay() {
        if (!analyzeModel.selectedReplayOnly) return
        const activeVehicle = QGroundControl.multiVehicleManager.activeVehicle
        if (activeVehicle && !activeVehicle.isOfflineEditingVehicle) {
            QGroundControl.showMessageDialog(root, qsTr("Route Replay"),
                qsTr("Close the active vehicle connection before starting telemetry replay."))
            return
        }
        const replayLink = QGroundControl.linkManager.startLogReplay(analyzeModel.selectedPath)
        if (!replayLink) {
            QGroundControl.showMessageDialog(root, qsTr("Route Replay"),
                qsTr("Unable to start replay for the selected telemetry log."))
            return
        }
        mainWindow.showAnalyzeTool()
        root.closeRequested()
    }

    function openDeepAnalysis() {
        mainWindow.showTool(qsTr("Log Viewer"),
                            "qrc:/qml/QGroundControl/AnalyzeView/LogViewer/LogViewerPage.qml",
                            "qrc:/qmlimages/MAVLinkInspector.svg")
        root.closeRequested()
    }

    LogFileParser {
        id: logParser
    }

    Connections {
        target: logParser
        function onParseFileFinished(filePath, ok, errorMessage) {
            if (filePath !== pendingLog) return
            if (!ok) {
                QGroundControl.showMessageDialog(root, qsTr("Analyze"), errorMessage)
                return
            }
            Qt.callLater(routeMap.fitRoute)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("ANALYZE"); color: "#F5F8FA"; font.pixelSize: 20; font.bold: true }
            Label {
                text: qsTr("LOCAL FLIGHT EVIDENCE")
                color: "#A8B5BD"
                font.pixelSize: 9
                font.bold: true
            }
            Item { Layout.fillWidth: true }
            Button { text: qsTr("REFRESH"); onClicked: analyzeModel.refreshHistory() }
            Button { text: qsTr("QGC LOG VIEWER"); onClicked: root.openDeepAnalysis() }
            NexusIconButton { text: "×"; onClicked: root.closeRequested() }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Rectangle {
                Layout.preferredWidth: 280
                Layout.fillHeight: true
                radius: 8
                color: "#0F161C"
                border.color: "#26333D"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 6

                    Label {
                        text: qsTr("FLIGHT HISTORY · %1").arg(analyzeModel.flightCount)
                        color: "#DDE5EA"
                        font.pixelSize: 10
                        font.bold: true
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 4
                        model: analyzeModel.flightHistory

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 72
                            radius: 7
                            color: analyzeModel.selectedPath === modelData.path ? "#172630" : "#121A20"
                            border.color: analyzeModel.selectedPath === modelData.path ? "#4A7489" : "#26333D"

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (analyzeModel.selectFlight(modelData.path)) {
                                        root.loadSelected()
                                    }
                                }
                            }

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 7
                                spacing: 2
                                Label {
                                    Layout.fillWidth: true
                                    text: modelData.name
                                    color: "#EEF3F6"
                                    font.pixelSize: 9
                                    font.bold: true
                                    elide: Text.ElideMiddle
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: modelData.type; color: "#7FA7BB"; font.pixelSize: 9; font.bold: true }
                                    Item { Layout.fillWidth: true }
                                    Label { text: modelData.sizeText; color: "#9AAAB4"; font.pixelSize: 9 }
                                }
                                Label { text: modelData.dateText; color: "#A4B1B9"; font.pixelSize: 9 }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 8
                color: "#0D141A"
                border.color: "#26333D"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 9
                    spacing: 7

                    NexusStateView {
                        visible: analyzeModel.selectedPath.length === 0
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        state: analyzeModel.flightCount > 0 ? "empty" : "empty"
                        title: analyzeModel.flightCount > 0 ? qsTr("Select a flight") : qsTr("No local flight history")
                        message: analyzeModel.flightCount > 0
                                 ? qsTr("Choose a telemetry or firmware log from Flight History to inspect route, timeline and graphs.")
                                 : qsTr("Flight logs stored on this device will appear here after a flight or imported evidence session.")
                        actionText: qsTr("REFRESH")
                        onAction: analyzeModel.refreshHistory()
                    }

                    NexusStateView {
                        visible: pendingLog.length > 0 && analyzeModel.selectedFirmwareLog && !logParser.parseComplete
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        state: "loading"
                        title: qsTr("Parsing flight evidence")
                        message: qsTr("Building route, event timeline and graph data from the selected log.")
                    }

                    ColumnLayout {
                        visible: analyzeModel.selectedPath.length > 0 && !(pendingLog.length > 0 && analyzeModel.selectedFirmwareLog && !logParser.parseComplete)
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 7

                        RowLayout {
                            Layout.fillWidth: true
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 1
                                Label { text: analyzeModel.selectedName; color: "#EEF3F6"; font.pixelSize: 13; font.bold: true; elide: Text.ElideMiddle; Layout.fillWidth: true }
                                Label {
                                    text: analyzeModel.selectedType + " · " + analyzeModel.selectedDate + " · " + analyzeModel.selectedSize
                                    color: "#AAB7BF"
                                    font.pixelSize: 9
                                }
                            }
                            Button {
                                text: qsTr("ROUTE REPLAY")
                                visible: analyzeModel.selectedReplayOnly
                                onClicked: root.startReplay()
                            }
                            Button { text: qsTr("DEEP ANALYSIS"); onClicked: root.openDeepAnalysis() }
                        }

                        Rectangle {
                            visible: analyzeModel.selectedReplayOnly
                            Layout.fillWidth: true
                            Layout.preferredHeight: 58
                            radius: 7
                            color: "#191A12"
                            border.color: "#6F662F"
                            Label {
                                anchors.fill: parent
                                anchors.margins: 8
                                wrapMode: Text.WordWrap
                                verticalAlignment: Text.AlignVCenter
                                text: qsTr("Telemetry .tlog selected. QGC performs authoritative live replay for this format. Static Nexus route/timeline graphs are available for PX4 ULog and DataFlash firmware logs.")
                                color: "#D1C77D"
                                font.pixelSize: 9
                            }
                        }

                        TabBar {
                            id: tabs
                            Layout.fillWidth: true
                            visible: analyzeModel.selectedFirmwareLog && logParser.parseComplete
                            onCurrentIndexChanged: root.selectedTab = currentIndex
                            TabButton { text: qsTr("ROUTE") }
                            TabButton { text: qsTr("TIMELINES") }
                            TabButton { text: qsTr("GRAPHS") }
                        }

                        StackLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            visible: analyzeModel.selectedFirmwareLog && logParser.parseComplete
                            currentIndex: root.selectedTab

                            Item {
                                FlightMap {
                                    id: routeMap
                                    anchors.fill: parent
                                    mapName: "NexusAnalyzeRoute"
                                    allowGCSLocationCenter: false
                                    readonly property var route: logParser.parseComplete ? logParser.gpsPath() : []

                                    function fitRoute() {
                                        const p = route
                                        if (!p || p.length < 2) return
                                        let minLat = p[0].latitude, maxLat = p[0].latitude
                                        let minLon = p[0].longitude, maxLon = p[0].longitude
                                        for (let i = 1; i < p.length; i++) {
                                            minLat = Math.min(minLat, p[i].latitude)
                                            maxLat = Math.max(maxLat, p[i].latitude)
                                            minLon = Math.min(minLon, p[i].longitude)
                                            maxLon = Math.max(maxLon, p[i].longitude)
                                        }
                                        setVisibleRegion(QtPositioning.rectangle(
                                            QtPositioning.coordinate(maxLat, minLon),
                                            QtPositioning.coordinate(minLat, maxLon)))
                                    }

                                    MapPolyline {
                                        line.width: 3
                                        line.color: "#4CA5D0"
                                        path: routeMap.route
                                    }

                                    MapScale {
                                        anchors.left: parent.left
                                        anchors.bottom: parent.bottom
                                        anchors.margins: 8
                                        mapControl: routeMap
                                    }
                                }

                                Label {
                                    anchors.centerIn: parent
                                    visible: routeMap.route.length < 2
                                    text: qsTr("GPS route unavailable in this log")
                                    color: "#8A979F"
                                }
                            }

                            ScrollView {
                                clip: true
                                ColumnLayout {
                                    width: parent.width
                                    spacing: 8

                                    Label {
                                        text: qsTr("TELEMETRY TIMELINE")
                                        color: "#EEF3F6"
                                        font.pixelSize: 11
                                        font.bold: true
                                    }
                                    Label {
                                        text: qsTr("%1 samples · %2 s → %3 s")
                                              .arg(logParser.sampleCount)
                                              .arg(logParser.minTimestamp.toFixed(1))
                                              .arg(logParser.maxTimestamp.toFixed(1))
                                        color: "#82919A"
                                        font.pixelSize: 9
                                    }

                                    Label {
                                        text: qsTr("MODE CHANGES")
                                        color: "#EEF3F6"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                    Repeater {
                                        model: logParser.modeSegments
                                        Rectangle {
                                            Layout.fillWidth: true
                                            implicitHeight: 38
                                            radius: 6
                                            color: "#121A20"
                                            RowLayout {
                                                anchors.fill: parent
                                                anchors.margins: 7
                                                Label { text: modelData.mode; color: "#DCE5EA"; font.pixelSize: 9; font.bold: true }
                                                Item { Layout.fillWidth: true }
                                                Label { text: Number(modelData.start).toFixed(1) + "s → " + Number(modelData.end).toFixed(1) + "s"; color: "#7E8B94"; font.pixelSize: 9 }
                                            }
                                        }
                                    }

                                    Label {
                                        text: qsTr("EVENT TIMELINE")
                                        color: "#EEF3F6"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                    Repeater {
                                        model: logParser.events
                                        Rectangle {
                                            Layout.fillWidth: true
                                            implicitHeight: eventText.implicitHeight + 16
                                            radius: 6
                                            color: "#121A20"
                                            RowLayout {
                                                anchors.fill: parent
                                                anchors.margins: 7
                                                Label { text: Number(modelData.time).toFixed(1) + "s"; color: "#78A8BF"; font.pixelSize: 9; font.bold: true }
                                                Label {
                                                    id: eventText
                                                    Layout.fillWidth: true
                                                    text: modelData.description
                                                    color: "#C9D3D9"
                                                    font.pixelSize: 9
                                                    wrapMode: Text.WordWrap
                                                }
                                                Label { text: modelData.type; color: "#A4B1B9"; font.pixelSize: 9 }
                                            }
                                        }
                                    }
                                }
                            }

                            ScrollView {
                                clip: true

                                GridLayout {
                                    width: parent.width
                                    columns: root.width >= 760 ? 2 : 1
                                    columnSpacing: 8
                                    rowSpacing: 8

                                    NexusAnalyzeGraph {
                                        logParser: logParser
                                        title: qsTr("BATTERY")
                                        unit: "%"
                                        candidateFields: [
                                            "battery_status.remaining",
                                            "battery.Remaining",
                                            "BAT.RemPct",
                                            "BAT.Remaining"
                                        ]
                                    }

                                    NexusAnalyzeGraph {
                                        logParser: logParser
                                        title: qsTr("ALTITUDE")
                                        unit: "m"
                                        candidateFields: [
                                            "vehicle_global_position.alt",
                                            "vehicle_local_position.z",
                                            "POS.Alt",
                                            "GPS.Alt"
                                        ]
                                    }

                                    NexusAnalyzeGraph {
                                        logParser: logParser
                                        title: qsTr("SPEED")
                                        unit: "m/s"
                                        candidateFields: [
                                            "vehicle_gps_position.vel_m_s",
                                            "vehicle_local_position.vx",
                                            "vehicle_global_position.vel_n",
                                            "GPS.Spd",
                                            "NKF1.VN"
                                        ]
                                    }

                                    NexusAnalyzeGraph {
                                        logParser: logParser
                                        title: qsTr("LINK HEALTH")
                                        unit: "%"
                                        candidateFields: [
                                            "telemetry_status.rx_message_lost_rate",
                                            "telemetry_status.tx_buffer_overruns",
                                            "RAD.RSSI",
                                            "RAD.RemRSSI"
                                        ]
                                    }

                                    NexusAnalyzeGraph {
                                        logParser: logParser
                                        title: qsTr("GPS / NAVIGATION")
                                        unit: ""
                                        candidateFields: [
                                            "vehicle_gps_position.satellites_used",
                                            "vehicle_gps_position.fix_type",
                                            "GPS.NSats",
                                            "GPS.Status"
                                        ]
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            visible: analyzeModel.selectedFirmwareLog && logParser.parsing
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Label { text: qsTr("Parsing flight evidence…"); color: "#A3B0B8" }
                            ProgressBar { Layout.fillWidth: true; from: 0; to: 1; value: logParser.parseProgress }
                        }
                    }
                }
            }
        }
    }
}
