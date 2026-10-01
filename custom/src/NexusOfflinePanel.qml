import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl

Rectangle {
    id: root

    property var offlineModel
    signal closeRequested()
    signal openPayloadRequested()
    signal openEngineerRequested()

    width: Math.min(640, parent ? parent.width * 0.54 : 640)
    color: "#F60A0F14"
    border.color: "#2A3742"
    border.width: 1
    radius: 12
    z: 6750

    function openMaps() {
        mainWindow.showSettingsTool("Maps")
        root.closeRequested()
    }

    component LocalCard: Rectangle {
        id: card
        property string title
        property string detail
        property string state
        property bool healthy: true
        property string buttonText: ""
        signal action()

        Layout.fillWidth: true
        implicitHeight: 92
        radius: 9
        color: "#10171E"
        border.color: healthy ? "#2B4D43" : "#665127"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 3

            RowLayout {
                Layout.fillWidth: true
                Label { text: card.title; color: "#F0F4F7"; font.pixelSize: 11; font.bold: true }
                Item { Layout.fillWidth: true }
                Label {
                    text: card.state
                    color: card.healthy ? "#78B7A1" : "#D6A84A"
                    font.pixelSize: 8
                    font.bold: true
                }
            }

            Label {
                Layout.fillWidth: true
                text: card.detail
                color: "#85939E"
                font.pixelSize: 8
                elide: Text.ElideMiddle
            }

            Item { Layout.fillHeight: true }

            Button {
                visible: card.buttonText.length > 0
                Layout.alignment: Qt.AlignRight
                text: card.buttonText
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
                Label { text: qsTr("OFFLINE-FIRST"); color: "#F6FAFC"; font.pixelSize: 19; font.bold: true }
                Label {
                    text: qsTr("FIELD STORAGE + MAP CACHE")
                    color: "#82929D"
                    font.pixelSize: 9
                    font.bold: true
                }
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                implicitWidth: 124
                implicitHeight: 30
                radius: 7
                color: "#0D141A"
                border.color: offlineModel.internetAvailable ? "#55717F" : "#2C9B7F"

                Label {
                    anchors.centerIn: parent
                    text: offlineModel.internetAvailable ? qsTr("INTERNET ONLINE") : qsTr("AIR-GAPPED")
                    color: parent.border.color
                    font.pixelSize: 9
                    font.bold: true
                }
            }

            Button { text: "×"; onClicked: root.closeRequested() }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 78
            radius: 9
            color: offlineModel.offlineReady ? "#0E1814" : "#211A10"
            border.color: offlineModel.offlineReady ? "#2C725F" : "#8A6B2B"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 3

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: offlineModel.readinessState
                        color: offlineModel.offlineReady ? "#85CDB5" : "#E0B65B"
                        font.pixelSize: 11
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: qsTr("CLOUD REQUIRED: NO")
                        color: "#86A7B8"
                        font.pixelSize: 8
                        font.bold: true
                    }
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Critical NEXUS vehicle workflows use local files, local links and cached map data. Internet availability is treated as optional.")
                    color: "#93A0A9"
                    font.pixelSize: 9
                    wrapMode: Text.WordWrap
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            GridLayout {
                width: parent.width
                columns: root.width >= 560 ? 2 : 1
                columnSpacing: 8
                rowSpacing: 8

                LocalCard {
                    title: qsTr("OFFLINE MAPS")
                    detail: qsTr("%1 cached set(s) · %2 selected/current cache estimate").arg(offlineModel.offlineMapSetCount).arg(offlineModel.offlineMapCacheSize)
                    state: offlineModel.offlineMapSetCount > 0 ? qsTr("CACHED") : qsTr("DOWNLOAD REQUIRED")
                    healthy: offlineModel.offlineMapSetCount > 0
                    buttonText: qsTr("MANAGE MAP CACHE")
                    onAction: root.openMaps()
                }

                LocalCard {
                    title: qsTr("LOCAL MISSIONS")
                    detail: offlineModel.missionPath
                    state: qsTr("LOCAL")
                    healthy: offlineModel.localStorageReady
                    buttonText: qsTr("OPEN PLAN")
                    onAction: {
                        if (mainWindow.allowViewSwitch()) {
                            mainWindow.showPlanView()
                            root.closeRequested()
                        }
                    }
                }

                LocalCard {
                    title: qsTr("VEHICLE CONFIGURATIONS")
                    detail: qsTr("%1 · settings: %2").arg(offlineModel.parameterPath).arg(offlineModel.settingsPath)
                    state: qsTr("LOCAL")
                    healthy: offlineModel.localStorageReady
                    buttonText: qsTr("ENGINEER")
                    onAction: root.openEngineerRequested()
                }

                LocalCard {
                    title: qsTr("TELEMETRY + LOGS")
                    detail: qsTr("%1 · %2").arg(offlineModel.telemetryPath).arg(offlineModel.logPath)
                    state: qsTr("LOCAL")
                    healthy: offlineModel.localStorageReady
                    buttonText: qsTr("ANALYZE")
                    onAction: {
                        mainWindow.showAnalyzeTool()
                        root.closeRequested()
                    }
                }

                LocalCard {
                    title: qsTr("LOCAL MEDIA")
                    detail: qsTr("%1 · %2").arg(offlineModel.videoPath).arg(offlineModel.photoPath)
                    state: qsTr("LOCAL")
                    healthy: offlineModel.localStorageReady
                    buttonText: qsTr("PAYLOAD")
                    onAction: root.openPayloadRequested()
                }

                LocalCard {
                    title: qsTr("NO-CLOUD DEPENDENCY")
                    detail: qsTr("No Nexus custom module requires login, remote API, cloud sync, analytics, or an internet-hosted control service to run core vehicle workflows.")
                    state: qsTr("ENFORCED")
                    healthy: !offlineModel.cloudRequired
                }

                LocalCard {
                    title: qsTr("AIRPLANE-MODE BEHAVIOR")
                    detail: qsTr("Map cache, missions, vehicle links, health, local configuration, logs and local media are designed to remain available with internet reachability absent.")
                    state: offlineModel.internetAvailable ? qsTr("NETWORK PRESENT") : qsTr("OFFLINE ACTIVE")
                    healthy: offlineModel.offlineReady
                }

                LocalCard {
                    title: qsTr("MAP-CACHE MANAGEMENT")
                    detail: qsTr("Create/download tile sets before deployment. Import/export .qgctiledb sets and delete/rename cached regions from QGC Maps settings.")
                    state: qsTr("%1 SETS").arg(offlineModel.offlineMapSetCount)
                    healthy: true
                    buttonText: qsTr("OPEN")
                    onAction: root.openMaps()
                }
            }
        }
    }
}
