import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning

import QGroundControl
import QGroundControl.Controls
import QGroundControl.FlightMap
import QGroundControl.LogViewer

Rectangle {
    id: root

    property var analyzeModel
    property var reportsModel
    signal closeRequested()

    width: Math.min(940, parent ? parent.width * 0.86 : 940)
    color: "#F70A0F14"
    border.color: "#2B3944"
    border.width: 1
    radius: 12
    z: 6950

    property string pendingLog: ""
    property real durationSeconds: NaN
    property real distanceMeters: NaN
    property real maxAltitudeMeters: NaN
    property real batteryUsedPercent: NaN
    property var warningRows: []
    property var eventRows: []
    property var routePoints: []
    property var reportData: ({})

    function unavailable(value, suffix) {
        return isNaN(value) ? qsTr("UNAVAILABLE") : Number(value).toFixed(1) + (suffix || "")
    }

    function resolveField(candidates) {
        const fields = logParser.plottableFields
        for (let c = 0; c < candidates.length; c++) {
            const wanted = String(candidates[c]).toLowerCase()
            for (let i = 0; i < fields.length; i++) {
                if (String(fields[i]).toLowerCase() === wanted) return String(fields[i])
            }
        }
        for (let c = 0; c < candidates.length; c++) {
            const wanted = String(candidates[c]).toLowerCase()
            for (let i = 0; i < fields.length; i++) {
                if (String(fields[i]).toLowerCase().indexOf(wanted) >= 0) return String(fields[i])
            }
        }
        return ""
    }

    function computeDistance(path) {
        if (!path || path.length < 2) return NaN
        let total = 0
        for (let i = 1; i < path.length; i++) {
            const a = QtPositioning.coordinate(path[i - 1].latitude, path[i - 1].longitude)
            const b = QtPositioning.coordinate(path[i].latitude, path[i].longitude)
            if (!a.isValid || !b.isValid) continue
            total += a.distanceTo(b)
        }
        return total > 0 ? total : NaN
    }

    function maxFromField(fieldName) {
        if (!fieldName) return NaN
        const mm = logParser.fieldMinMax(fieldName)
        return mm && mm.max !== undefined ? Number(mm.max) : NaN
    }

    function batteryUsage(fieldName) {
        if (!fieldName) return NaN
        const points = logParser.fieldSamples(fieldName)
        if (!points || points.length < 2) return NaN
        const start = Number(points[0].y)
        const end = Number(points[points.length - 1].y)
        if (isNaN(start) || isNaN(end)) return NaN
        const used = start - end
        return used >= 0 && used <= 100 ? used : NaN
    }

    function rebuildEvidence() {
        if (!logParser.parseComplete) return

        durationSeconds = logParser.maxTimestamp > logParser.minTimestamp
                ? logParser.maxTimestamp - logParser.minTimestamp : NaN

        routePoints = logParser.gpsPath()
        distanceMeters = reportsModel.routeDistanceMeters(routePoints)

        const altField = resolveField([
            "vehicle_global_position.alt",
            "vehicle_local_position.z",
            "POS.Alt",
            "GPS.Alt"
        ])
        maxAltitudeMeters = altField ? reportsModel.maxSampleValue(logParser.fieldSamples(altField)) : NaN

        const batteryField = resolveField([
            "battery_status.remaining",
            "BAT.RemPct",
            "BAT.Remaining"
        ])
        batteryUsedPercent = batteryField ? reportsModel.batteryUsedPercent(logParser.fieldSamples(batteryField)) : NaN

        eventRows = logParser.events
        const warnings = []
        for (let i = 0; i < eventRows.length; i++) {
            const t = String(eventRows[i].type).toLowerCase()
            if (t === "warning" || t === "error") warnings.push(eventRows[i])
        }
        warningRows = warnings

        if (reportsModel.aircraft.length === 0 && logParser.detectedVehicleType.length > 0) {
            reportsModel.aircraft = logParser.detectedVehicleType
        }

        reportData = reportsModel.buildReportData(
            logParser.startTime && !isNaN(logParser.startTime.getTime())
                ? Qt.formatDateTime(logParser.startTime, Qt.ISODate) : "",
            durationSeconds,
            distanceMeters,
            maxAltitudeMeters,
            batteryUsedPercent,
            warningRows,
            eventRows,
            routePoints
        )

        Qt.callLater(routeMap.fitRoute)
    }

    function loadSelected() {
        logParser.clear()
        pendingLog = ""
        durationSeconds = NaN
        distanceMeters = NaN
        maxAltitudeMeters = NaN
        batteryUsedPercent = NaN
        warningRows = []
        eventRows = []
        routePoints = []
        reportData = ({})

        reportsModel.loadForSource(analyzeModel.selectedPath)
        if (!analyzeModel.selectedFirmwareLog) return
        pendingLog = analyzeModel.selectedPath
        logParser.startParsingAsync(pendingLog)
    }

    LogFileParser { id: logParser }

    Connections {
        target: logParser
        function onParseFileFinished(filePath, ok, errorMessage) {
            if (filePath !== pendingLog) return
            if (!ok) {
                QGroundControl.showMessageDialog(root, qsTr("Reports"), errorMessage)
                return
            }
            root.rebuildEvidence()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("REPORTS"); color: "#F4F8FA"; font.pixelSize: 20; font.bold: true }
            Label { text: qsTr("POST-FLIGHT EVIDENCE"); color: "#A8B5BD"; font.pixelSize: 9; font.bold: true }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("REFRESH REPORT")
                enabled: analyzeModel.selectedFirmwareLog
                onClicked: root.loadSelected()
            }
            NexusIconButton { text: "×"; onClicked: root.closeRequested() }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 58
            radius: 8
            color: analyzeModel.selectedPath.length > 0 ? "#0F171D" : "#1B1610"
            border.color: analyzeModel.selectedPath.length > 0 ? "#2C4757" : "#70572C"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 9
                Label {
                    Layout.fillWidth: true
                    text: analyzeModel.selectedPath.length > 0
                          ? analyzeModel.selectedName + " · " + analyzeModel.selectedType
                          : qsTr("Select a flight in ANALYZE before generating a report.")
                    color: "#DDE5EA"
                    font.pixelSize: 10
                    font.bold: true
                    elide: Text.ElideMiddle
                }
                Label {
                    text: analyzeModel.selectedFirmwareLog ? qsTr("EVIDENCE READY")
                         : (analyzeModel.selectedReplayOnly ? qsTr("TLOG REPLAY ONLY") : qsTr("NO SOURCE"))
                    color: analyzeModel.selectedFirmwareLog ? "#7FC5AD" : "#D3AF5E"
                    font.pixelSize: 9
                    font.bold: true
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            visible: analyzeModel.selectedPath.length > 0

            ColumnLayout {
                width: parent.width
                spacing: 8

                GridLayout {
                    Layout.fillWidth: true
                    columns: root.width >= 760 ? 2 : 1
                    columnSpacing: 8
                    rowSpacing: 8

                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("MISSION ID"); color: "#AAB7BF"; font.pixelSize: 9; font.bold: true }
                        TextField {
                            Layout.fillWidth: true
                            text: reportsModel.missionId
                            placeholderText: qsTr("Enter mission ID")
                            onEditingFinished: {
                                reportsModel.missionId = text.trim()
                                root.rebuildEvidence()
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("OPERATOR"); color: "#AAB7BF"; font.pixelSize: 9; font.bold: true }
                        TextField {
                            Layout.fillWidth: true
                            text: reportsModel.operatorName
                            placeholderText: qsTr("Enter operator name")
                            onEditingFinished: {
                                reportsModel.operatorName = text.trim()
                                root.rebuildEvidence()
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("AIRCRAFT"); color: "#AAB7BF"; font.pixelSize: 9; font.bold: true }
                        TextField {
                            Layout.fillWidth: true
                            text: reportsModel.aircraft
                            placeholderText: qsTr("NOT RECORDED")
                            onEditingFinished: {
                                reportsModel.aircraft = text.trim()
                                root.rebuildEvidence()
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("FIRMWARE"); color: "#AAB7BF"; font.pixelSize: 9; font.bold: true }
                        TextField {
                            Layout.fillWidth: true
                            text: reportsModel.firmware
                            placeholderText: qsTr("NOT RECORDED")
                            onEditingFinished: {
                                reportsModel.firmware = text.trim()
                                root.rebuildEvidence()
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("MISSION COMPLETION"); color: "#AAB7BF"; font.pixelSize: 9; font.bold: true }
                        ComboBox {
                            Layout.fillWidth: true
                            model: [qsTr("NOT RECORDED"), qsTr("COMPLETED"), qsTr("PARTIAL"), qsTr("ABORTED"), qsTr("FAILED")]
                            currentIndex: Math.max(0, model.indexOf(reportsModel.missionCompletion))
                            onActivated: {
                                reportsModel.missionCompletion = currentText
                                root.rebuildEvidence()
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("DATE / TIME"); color: "#AAB7BF"; font.pixelSize: 9; font.bold: true }
                        Label {
                            Layout.fillWidth: true
                            text: logParser.parseComplete && logParser.startTime
                                  ? Qt.formatDateTime(logParser.startTime, Qt.locale().dateTimeFormat(Locale.ShortFormat))
                                  : qsTr("UNAVAILABLE")
                            color: "#DEE5E9"
                            font.pixelSize: 10
                        }
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: root.width >= 760 ? 4 : 2
                    columnSpacing: 8
                    rowSpacing: 8

                    Repeater {
                        model: [
                            { label: qsTr("DURATION"), value: root.unavailable(root.durationSeconds, " s") },
                            { label: qsTr("DISTANCE"), value: root.unavailable(root.distanceMeters, " m") },
                            { label: qsTr("MAX ALTITUDE"), value: root.unavailable(root.maxAltitudeMeters, " m") },
                            { label: qsTr("BATTERY USED"), value: root.unavailable(root.batteryUsedPercent, " %") }
                        ]

                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 68
                            radius: 7
                            color: "#10181E"
                            border.color: "#293740"
                            Column {
                                anchors.centerIn: parent
                                spacing: 3
                                Label { anchors.horizontalCenter: parent.horizontalCenter; text: modelData.label; color: "#7F8E97"; font.pixelSize: 9; font.bold: true }
                                Label { anchors.horizontalCenter: parent.horizontalCenter; text: modelData.value; color: "#EFF4F7"; font.pixelSize: 13; font.bold: true }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 300
                    radius: 8
                    color: "#0D141A"
                    border.color: "#293740"

                    FlightMap {
                        id: routeMap
                        anchors.fill: parent
                        anchors.margins: 1
                        mapName: "NexusReportRoute"
                        allowGCSLocationCenter: false
                        readonly property var route: root.routePoints || []

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
                        visible: routePoints.length < 2
                        text: qsTr("ROUTE MAP UNAVAILABLE")
                        color: "#87949C"
                        font.bold: true
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: summaryColumn.implicitHeight + 20
                    radius: 8
                    color: "#10181E"
                    border.color: "#293740"

                    ColumnLayout {
                        id: summaryColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 10
                        spacing: 6

                        Label { text: qsTr("WARNING / EVENT SUMMARY"); color: "#EFF4F7"; font.pixelSize: 10; font.bold: true }
                        Label {
                            text: qsTr("%1 warning/error(s) · %2 total event(s)").arg(warningRows.length).arg(eventRows.length)
                            color: warningRows.length > 0 ? "#D6A84A" : "#7FC5AD"
                            font.pixelSize: 9
                            font.bold: true
                        }

                        Repeater {
                            model: warningRows.slice(0, 12)
                            Label {
                                Layout.fillWidth: true
                                text: Number(modelData.time).toFixed(1) + "s · " + modelData.description
                                color: "#C7D1D7"
                                font.pixelSize: 9
                                wrapMode: Text.WordWrap
                            }
                        }

                        Label {
                            visible: warningRows.length === 0
                            text: qsTr("No warnings/errors recorded in this log.")
                            color: "#81909A"
                            font.pixelSize: 9
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: exportRow.implicitHeight + 20
                    radius: 8
                    color: "#0D151B"
                    border.color: "#345061"

                    RowLayout {
                        id: exportRow
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 10
                        spacing: 8

                        ColumnLayout {
                            Layout.fillWidth: true
                            Label { text: qsTr("EXPORT PIPELINE"); color: "#DDE6EB"; font.pixelSize: 10; font.bold: true }
                            Label {
                                text: qsTr("Report schema v1.0 is ready for future PDF, CSV and KML exporters. Export buttons remain intentionally disabled until exporter implementations are added and validated.")
                                color: "#AAB7BF"
                                font.pixelSize: 9
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }

                        Button { text: "PDF"; enabled: false }
                        Button { text: "CSV"; enabled: false }
                        Button { text: "KML"; enabled: false }
                    }
                }
            }
        }
    }

    Connections {
        target: analyzeModel
        function onAnalyzeChanged() {
            if (reportsModel.sourcePath !== analyzeModel.selectedPath) root.loadSelected()
        }
    }

    Component.onCompleted: root.loadSelected()
}
