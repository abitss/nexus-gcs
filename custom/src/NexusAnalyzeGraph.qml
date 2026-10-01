import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs

Rectangle {
    id: root

    required property var logParser
    property string title
    property string unit
    property var candidateFields: []
    property string resolvedField: ""
    property bool available: resolvedField.length > 0
    property int maxPoints: 900

    Layout.fillWidth: true
    implicitHeight: 190
    radius: 8
    color: "#0E151B"
    border.color: available ? "#2B3944" : "#202A31"

    function resolveField() {
        resolvedField = ""
        if (!logParser || !logParser.parseComplete) return

        const fields = logParser.plottableFields
        for (let c = 0; c < candidateFields.length; c++) {
            const wanted = String(candidateFields[c]).toLowerCase()
            for (let i = 0; i < fields.length; i++) {
                const field = String(fields[i])
                if (field.toLowerCase() === wanted) {
                    resolvedField = field
                    rebuild()
                    return
                }
            }
        }

        for (let c = 0; c < candidateFields.length; c++) {
            const wanted = String(candidateFields[c]).toLowerCase()
            for (let i = 0; i < fields.length; i++) {
                const field = String(fields[i])
                if (field.toLowerCase().indexOf(wanted) >= 0) {
                    resolvedField = field
                    rebuild()
                    return
                }
            }
        }
    }

    function rebuild() {
        series.clear()
        if (!available) return

        const minT = logParser.minTimestamp
        const maxT = logParser.maxTimestamp
        if (minT < 0 || maxT <= minT) return

        const points = logParser.fieldSamplesFiltered(resolvedField, minT, maxT, maxPoints)
        if (!points || points.length === 0) return

        let minY = Number.MAX_VALUE
        let maxY = -Number.MAX_VALUE
        for (let i = 0; i < points.length; i++) {
            const x = points[i].x - minT
            const y = points[i].y
            series.append(x, y)
            minY = Math.min(minY, y)
            maxY = Math.max(maxY, y)
        }

        xAxis.min = 0
        xAxis.max = Math.max(1, maxT - minT)
        if (minY === maxY) {
            minY -= 1
            maxY += 1
        }
        const pad = Math.max(0.01, (maxY - minY) * 0.08)
        yAxis.min = minY - pad
        yAxis.max = maxY + pad
    }

    Connections {
        target: logParser
        function onParseCompleteChanged() {
            if (logParser.parseComplete) root.resolveField()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 9
        spacing: 4

        RowLayout {
            Layout.fillWidth: true
            Label { text: root.title; color: "#EEF3F6"; font.pixelSize: 10; font.bold: true }
            Item { Layout.fillWidth: true }
            Label {
                text: root.available ? root.resolvedField : qsTr("UNAVAILABLE IN LOG")
                color: root.available ? "#7FA7BB" : "#697680"
                font.pixelSize: 7
                elide: Text.ElideLeft
                Layout.maximumWidth: root.width * 0.58
            }
        }

        GraphsView {
            id: graphView
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: GraphsTheme {
                backgroundVisible: false
                plotAreaBackgroundVisible: false
                grid.mainColor: "#22303A"
                axisX.mainColor: "#60717E"
                axisY.mainColor: "#60717E"
                labelTextColor: "#9CA8B1"
            }

            axisX: ValueAxis {
                id: xAxis
                titleText: qsTr("Elapsed s")
                min: 0
                max: 1
            }

            axisY: ValueAxis {
                id: yAxis
                titleText: root.unit
                min: 0
                max: 1
            }

            LineSeries {
                id: series
                width: 2
            }
        }
    }
}
