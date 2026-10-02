import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import "NexusTokens.js" as T

Rectangle {
    id: root

    property var payloadModel
    property var mapControl
    signal closeRequested()

    width: parent ? Math.min(520, Math.max(360, parent.width - 20 < 360 ? parent.width - 20 : parent.width * 0.42)) : 430
    color: "#F20B1016"
    border.color: "#273540"
    border.width: 1
    radius: 12
    z: 6500

    function accentForStream(state) {
        if (state === "LIVE") return "#2C9B7F"
        if (state === "LOST") return "#D95151"
        if (state === "WAITING") return "#D6A84A"
        return "#4F6575"
    }

    function showMapWithVideoPip() {
        QGroundControl.videoManager.fullScreen = false
        if (mapControl && mapControl.pipState && mapControl.pipState.state !== mapControl.pipState.fullState &&
            mapControl.pipState.pipView && mapControl.pipState.pipView._swapPip) {
            mapControl.pipState.pipView._swapPip()
        }
    }

    function showVideoMain() {
        QGroundControl.videoManager.fullScreen = false
        if (mapControl && mapControl.pipState && mapControl.pipState.state === mapControl.pipState.fullState &&
            mapControl.pipState.pipView && mapControl.pipState.pipView._swapPip) {
            mapControl.pipState.pipView._swapPip()
        }
    }

    function showVideoFullscreen() {
        showVideoMain()
        QGroundControl.videoManager.fullScreen = true
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 1
                Label { text: "PAYLOAD / VIDEO"; color: "#F6FAFC"; font.pixelSize: 18; font.bold: true }
                Label { text: payloadModel.cameraName; color: "#8D9AA5"; font.pixelSize: 9 }
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                implicitWidth: 92
                implicitHeight: 30
                radius: 7
                color: "#0D141A"
                border.color: root.accentForStream(payloadModel.streamState)
                Label {
                    anchors.centerIn: parent
                    text: payloadModel.streamState
                    color: root.accentForStream(text)
                    font.pixelSize: 10
                    font.bold: true
                }
            }
            NexusIconButton { text: "×"; onClicked: root.closeRequested() }
        }

        NexusStateView {
            visible: !payloadModel.configured && !payloadModel.hasCamera
            Layout.fillWidth: true
            Layout.preferredHeight: 180
            state: "empty"
            title: qsTr("No camera or stream source")
            message: qsTr("Configure an EO/FPV stream or connect a supported camera before using live view, recording, zoom or gimbal controls.")
        }

        Rectangle {
            visible: payloadModel.configured || payloadModel.hasCamera
            Layout.fillWidth: true
            Layout.preferredHeight: 74
            radius: 9
            color: "#10171E"
            border.color: root.accentForStream(payloadModel.streamState)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 3
                Label { text: payloadModel.streamDetail; color: "#E5EBF0"; font.pixelSize: 10 }
                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "RES " + payloadModel.resolutionText; color: "#9BA8B2"; font.pixelSize: 9 }
                    Label { text: payloadModel.fpsText; color: "#9BA8B2"; font.pixelSize: 9 }
                    Item { Layout.fillWidth: true }
                    Label { text: payloadModel.latencyState; color: "#9BA8B2"; font.pixelSize: 9; font.bold: true }
                }
                Label { text: payloadModel.latencyDetail; color: "#788792"; font.pixelSize: 9; elide: Text.ElideRight; Layout.fillWidth: true }
            }
        }

        GridLayout {
            visible: payloadModel.configured || payloadModel.hasCamera
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 8
            rowSpacing: 8

            Button {
                text: qsTr("MAP + VIDEO")
                Layout.fillWidth: true
                enabled: payloadModel.configured
                onClicked: root.showMapWithVideoPip()
            }
            Button {
                text: qsTr("VIDEO MAIN")
                Layout.fillWidth: true
                enabled: payloadModel.configured
                onClicked: root.showVideoMain()
            }
            Button {
                text: qsTr("FULLSCREEN")
                Layout.fillWidth: true
                enabled: payloadModel.decoding
                onClicked: root.showVideoFullscreen()
            }
            Button {
                text: qsTr("SNAPSHOT")
                Layout.fillWidth: true
                enabled: payloadModel.decoding || payloadModel.hasCamera
                onClicked: payloadModel.snapshot()
            }
            Button {
                text: payloadModel.recording ? qsTr("STOP RECORDING") : qsTr("RECORD")
                Layout.fillWidth: true
                enabled: payloadModel.configured || payloadModel.hasCamera
                onClicked: payloadModel.toggleRecording()
            }
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 40
                radius: 7
                color: payloadModel.recording ? "#2A1517" : "#10171E"
                border.color: payloadModel.recording ? "#D95151" : "#273540"
                Label {
                    anchors.centerIn: parent
                    text: (payloadModel.recording ? "REC · " : "REC · ") + payloadModel.recordTimeText
                    color: payloadModel.recording ? "#FF9C9C" : "#A7B2BB"
                    font.bold: true
                    font.pixelSize: 10
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: payloadModel.hasZoom ? 72 : 0
            visible: payloadModel.hasZoom
            radius: 9
            color: "#10171E"
            border.color: "#273540"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                Label { text: "ZOOM"; color: "#9BA8B2"; font.pixelSize: 9; font.bold: true }
                Button { text: "WIDE"; onPressed: payloadModel.zoomContinuous(-1); onReleased: payloadModel.zoomStop() }
                Button { text: "−"; onClicked: payloadModel.zoomStep(-1) }
                Button { text: "+"; onClicked: payloadModel.zoomStep(1) }
                Button { text: "TELE"; onPressed: payloadModel.zoomContinuous(1); onReleased: payloadModel.zoomStop() }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: payloadModel.hasGimbal ? 132 : 0
            visible: payloadModel.hasGimbal
            radius: 9
            color: "#10171E"
            border.color: "#273540"

            GridLayout {
                anchors.centerIn: parent
                columns: 3
                rowSpacing: 4
                columnSpacing: 4

                Item { width: 60; height: 34 }
                Button {
                    text: "▲"
                    onPressed: payloadModel.gimbalRate(15, 0)
                    onReleased: payloadModel.gimbalRate(0, 0)
                }
                Item { width: 60; height: 34 }

                Button {
                    text: "◀"
                    onPressed: payloadModel.gimbalRate(0, -20)
                    onReleased: payloadModel.gimbalRate(0, 0)
                }
                Button { text: "CENTER"; onClicked: payloadModel.gimbalCenter() }
                Button {
                    text: "▶"
                    onPressed: payloadModel.gimbalRate(0, 20)
                    onReleased: payloadModel.gimbalRate(0, 0)
                }

                Item { width: 60; height: 34 }
                Button {
                    text: "▼"
                    onPressed: payloadModel.gimbalRate(-15, 0)
                    onReleased: payloadModel.gimbalRate(0, 0)
                }
                Item { width: 60; height: 34 }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 9
            color: "#10171E"
            border.color: "#273540"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5
                Label { text: "LOCAL MEDIA STORAGE"; color: "#9BA8B2"; font.pixelSize: 9; font.bold: true }
                Label { text: "VIDEO · " + payloadModel.localVideoPath; color: "#E5EBF0"; font.pixelSize: 9; elide: Text.ElideMiddle; Layout.fillWidth: true }
                Label { text: "PHOTO · " + payloadModel.localPhotoPath; color: "#E5EBF0"; font.pixelSize: 9; elide: Text.ElideMiddle; Layout.fillWidth: true }
                Item { Layout.fillHeight: true }
                Label {
                    Layout.fillWidth: true
                    text: qsTr("FPS is shown only when reported by the camera stream. Latency status reports pipeline mode/jitter configuration, not an invented end-to-end measurement.")
                    color: "#9AAAB4"
                    font.pixelSize: 9
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
