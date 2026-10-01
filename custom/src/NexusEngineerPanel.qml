import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls

Rectangle {
    id: root

    property var engineerModel
    signal closeRequested()
    signal openHealthRequested()

    width: Math.min(650, parent ? parent.width * 0.56 : 650)
    color: "#F60A0F14"
    border.color: "#2B3944"
    border.width: 1
    radius: 12
    z: 6800

    readonly property var activeVehicle: QGroundControl.multiVehicleManager.activeVehicle

    Connections {
        target: engineerModel
        function onEngineerChanged() {
            if (!engineerModel.unlocked) {
                QGroundControl.corePlugin.showAdvancedUI = false
            }
        }
    }

    function openParameters() {
        if (!engineerModel.unlocked) return
        mainWindow.showVehicleConfigParametersPage()
        root.closeRequested()
    }

    function openMavlinkInspector() {
        if (!engineerModel.unlocked || !activeVehicle) return
        mainWindow.showTool(qsTr("MAVLink Inspector"),
                            "qrc:/qml/QGroundControl/AnalyzeView/MAVLinkInspector/MAVLinkInspectorPage.qml",
                            "qrc:/qmlimages/MAVLinkInspector.svg")
        root.closeRequested()
    }

    function openVibration() {
        if (!engineerModel.unlocked || !activeVehicle) return
        mainWindow.showTool(qsTr("Vibration"),
                            "qrc:/qml/QGroundControl/AnalyzeView/Vibration/VibrationPage.qml",
                            "qrc:/qmlimages/VibrationPageIcon")
        root.closeRequested()
    }

    function openDeveloperLogs() {
        if (!engineerModel.unlocked) return
        mainWindow.showSettingsTool("App Logging")
        root.closeRequested()
    }

    component ToolCard: Rectangle {
        id: card
        property string title
        property string detail
        property string state: ""
        property bool available: true
        property string buttonText: qsTr("OPEN")
        signal action()

        Layout.fillWidth: true
        implicitHeight: 100
        radius: 9
        color: "#10171E"
        border.color: available ? "#2B3944" : "#202A31"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 3

            RowLayout {
                Layout.fillWidth: true
                Label { text: card.title; color: "#F0F4F7"; font.pixelSize: 11; font.bold: true }
                Item { Layout.fillWidth: true }
                Label { visible: card.state.length > 0; text: card.state; color: "#7FA0B5"; font.pixelSize: 8; font.bold: true }
            }

            Label {
                Layout.fillWidth: true
                text: card.detail
                color: "#85939E"
                font.pixelSize: 9
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }

            Item { Layout.fillHeight: true }

            Button {
                Layout.alignment: Qt.AlignRight
                text: card.buttonText
                enabled: card.available
                onClicked: card.action()
            }
        }
    }

    Dialog {
        id: unlockDialog
        modal: true
        title: qsTr("Enter Engineer Mode")
        standardButtons: Dialog.Cancel

        ColumnLayout {
            width: Math.min(440, root.width - 80)
            spacing: 10

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Engineer Mode exposes low-level vehicle configuration and diagnostic tools. It is a local session gate, not account authentication. Type ENGINEER to continue.")
            }

            TextField {
                id: engineerConfirmation
                Layout.fillWidth: true
                placeholderText: qsTr("Type ENGINEER")
                onAccepted: {
                    if (engineerModel.unlock(text)) {
                        QGroundControl.corePlugin.showAdvancedUI = true
                        unlockDialog.close()
                        text = ""
                    }
                }
            }

            Button {
                Layout.alignment: Qt.AlignRight
                text: qsTr("UNLOCK")
                enabled: engineerConfirmation.text.trim().toUpperCase() === "ENGINEER"
                onClicked: {
                    if (engineerModel.unlock(engineerConfirmation.text)) {
                        QGroundControl.corePlugin.showAdvancedUI = true
                        unlockDialog.close()
                        engineerConfirmation.text = ""
                    }
                }
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
                Label { text: qsTr("ENGINEER MODE"); color: "#F6FAFC"; font.pixelSize: 19; font.bold: true }
                Label {
                    text: engineerModel.unlocked
                          ? qsTr("ENGINEER SESSION · %1 PARAMETERS").arg(engineerModel.parameterCount)
                          : qsTr("PROTECTED ENTRY")
                    color: engineerModel.unlocked ? "#70A8C8" : "#D6A84A"
                    font.pixelSize: 9
                    font.bold: true
                }
            }
            Item { Layout.fillWidth: true }

            Button {
                text: engineerModel.unlocked ? qsTr("LOCK") : qsTr("UNLOCK")
                onClicked: {
                    if (engineerModel.unlocked) {
                        engineerModel.lock()
                        QGroundControl.corePlugin.showAdvancedUI = false
                    } else {
                        unlockDialog.open()
                    }
                }
            }
            Button { text: "×"; onClicked: root.closeRequested() }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 74
            radius: 9
            color: engineerModel.unlocked ? "#0E171C" : "#1F1910"
            border.color: engineerModel.unlocked ? "#3A687E" : "#8A6B2B"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 3
                Label {
                    text: engineerModel.unlocked ? qsTr("ENGINEER ROLE ACTIVE") : qsTr("ENGINEER ROLE LOCKED")
                    color: engineerModel.unlocked ? "#8FC6E1" : "#E2BB63"
                    font.pixelSize: 10
                    font.bold: true
                }
                Label {
                    Layout.fillWidth: true
                    text: !engineerModel.unlocked
                          ? qsTr("Unlock to access parameters, raw telemetry, stream inspection, snapshots and developer evidence.")
                          : (!engineerModel.safeToWrite
                             ? qsTr("Read-only diagnostics remain available. Parameter-changing workflows are unsafe while armed, flying, link-lost, or while writes are pending.")
                             : qsTr("Vehicle is in a safe state for engineering configuration."))
                    color: "#9AA7B0"
                    font.pixelSize: 9
                    wrapMode: Text.WordWrap
                }
            }
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            active: engineerModel.unlocked
            sourceComponent: engineerWorkspace
        }

        Item {
            visible: !engineerModel.unlocked
            Layout.fillWidth: true
            Layout.fillHeight: true

            Column {
                anchors.centerIn: parent
                spacing: 12
                Label { anchors.horizontalCenter: parent.horizontalCenter; text: "ENGINEER"; color: "#566773"; font.pixelSize: 28; font.bold: true }
                Label { anchors.horizontalCenter: parent.horizontalCenter; text: qsTr("Protected engineering workspace"); color: "#7D8B95"; font.pixelSize: 11 }
                Button { anchors.horizontalCenter: parent.horizontalCenter; text: qsTr("UNLOCK ENGINEER MODE"); onClicked: unlockDialog.open() }
            }
        }
    }

    Component {
        id: engineerWorkspace

        ScrollView {
            clip: true

            ColumnLayout {
                width: parent.width
                spacing: 9

                GridLayout {
                    Layout.fillWidth: true
                    columns: root.width >= 560 ? 2 : 1
                    columnSpacing: 8
                    rowSpacing: 8

                    ToolCard {
                        title: qsTr("PARAMETER BROWSER")
                        detail: qsTr("Search, categories, Modified-only, Favorites, read-only filtering and parameter metadata including min/max/default.")
                        state: qsTr("%1 PARAMS").arg(engineerModel.parameterCount)
                        available: !!activeVehicle && engineerModel.safeToWrite
                        onAction: root.openParameters()
                    }

                    ToolCard {
                        title: qsTr("BACKUP / RESTORE")
                        detail: qsTr("Use QGC's parameter Tools menu to save a complete parameter backup or load a file for review/diff before applying.")
                        state: engineerModel.safeToWrite ? qsTr("WRITE SAFE") : qsTr("READ / REVIEW")
                        available: !!activeVehicle && engineerModel.safeToWrite
                        onAction: root.openParameters()
                    }

                    ToolCard {
                        title: qsTr("MAVLINK INSPECTOR / STREAM RATES")
                        detail: qsTr("Inspect raw MAVLink messages, fields, component IDs, message counts, actual rates and supported target-rate controls.")
                        state: activeVehicle ? qsTr("LIVE") : qsTr("NO VEHICLE")
                        available: !!activeVehicle
                        onAction: root.openMavlinkInspector()
                    }

                    ToolCard {
                        title: qsTr("RAW SENSOR DIAGNOSTICS")
                        detail: qsTr("Open QGC vibration/raw telemetry diagnostics. Use MAVLink Inspector for RAW_IMU, SCALED_IMU and HIGHRES_IMU fields.")
                        state: NexusHealth.imuState
                        available: !!activeVehicle
                        onAction: root.openVibration()
                    }

                    ToolCard {
                        title: qsTr("EKF / GPS / IMU")
                        detail: qsTr("EKF: %1 · GPS: %2 · IMU: %3").arg(NexusHealth.ekfDetail).arg(NexusHealth.gpsDetail).arg(NexusHealth.imuDetail)
                        state: NexusHealth.overallState
                        available: !!activeVehicle
                        onAction: root.openHealthRequested()
                    }

                    ToolCard {
                        title: qsTr("SYSTEM MESSAGES")
                        detail: activeVehicle && activeVehicle.messageCount > 0
                                ? qsTr("%1 vehicle message(s). Open to inspect STATUSTEXT/events.").arg(activeVehicle.messageCount)
                                : qsTr("No current vehicle system messages.")
                        state: activeVehicle ? qsTr("%1 MSG").arg(activeVehicle.messageCount) : qsTr("OFFLINE")
                        available: !!activeVehicle
                        buttonText: qsTr("VIEW")
                        onAction: systemMessagesDialog.open()
                    }

                    ToolCard {
                        title: qsTr("DEVELOPER LOGS")
                        detail: qsTr("Open QGC App Logging for categorized runtime logs, filtering, export and logging configuration.")
                        state: qsTr("QGC LOG MANAGER")
                        available: true
                        onAction: root.openDeveloperLogs()
                    }

                    ToolCard {
                        title: qsTr("PARAMETER SNAPSHOTS")
                        detail: qsTr("Capture two in-memory parameter snapshots and compare exact raw values without writing anything to the aircraft.")
                        state: engineerModel.snapshotAValid && engineerModel.snapshotBValid
                               ? qsTr("%1 DIFFERENCES").arg(engineerModel.snapshotDiffCount)
                               : qsTr("CAPTURE A + B")
                        available: !!activeVehicle
                        buttonText: qsTr("SNAPSHOTS")
                        onAction: snapshotDialog.open()
                    }
                }
            }
        }
    }

    Dialog {
        id: systemMessagesDialog
        modal: true
        title: qsTr("Vehicle System Messages")
        standardButtons: Dialog.Close

        ScrollView {
            width: Math.min(620, root.width - 70)
            height: Math.min(500, root.height - 120)
            TextArea {
                readOnly: true
                wrapMode: TextEdit.Wrap
                text: activeVehicle ? activeVehicle.formattedMessages : qsTr("No active vehicle")
            }
        }
    }

    Dialog {
        id: snapshotDialog
        modal: true
        title: qsTr("Compare Parameter Snapshots")
        standardButtons: Dialog.Close

        ColumnLayout {
            width: Math.min(610, root.width - 70)
            height: Math.min(520, root.height - 110)
            spacing: 8

            RowLayout {
                Layout.fillWidth: true
                TextField { id: snapshotALabel; Layout.fillWidth: true; placeholderText: qsTr("Snapshot A label") }
                Button { text: qsTr("CAPTURE A"); enabled: !!activeVehicle; onClicked: engineerModel.captureSnapshotA(snapshotALabel.text) }
            }
            Label { Layout.fillWidth: true; text: engineerModel.snapshotAValid ? engineerModel.snapshotALabel : qsTr("Snapshot A not captured"); color: "#8997A1"; font.pixelSize: 9 }

            RowLayout {
                Layout.fillWidth: true
                TextField { id: snapshotBLabel; Layout.fillWidth: true; placeholderText: qsTr("Snapshot B label") }
                Button { text: qsTr("CAPTURE B"); enabled: !!activeVehicle; onClicked: engineerModel.captureSnapshotB(snapshotBLabel.text) }
            }
            Label { Layout.fillWidth: true; text: engineerModel.snapshotBValid ? engineerModel.snapshotBLabel : qsTr("Snapshot B not captured"); color: "#8997A1"; font.pixelSize: 9 }

            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: engineerModel.snapshotAValid && engineerModel.snapshotBValid
                          ? qsTr("%1 parameter difference(s)").arg(engineerModel.snapshotDiffCount)
                          : qsTr("Capture both snapshots to compare")
                    color: "#E7EDF1"
                    font.bold: true
                }
                Item { Layout.fillWidth: true }
                Button { text: qsTr("COMPARE"); enabled: engineerModel.snapshotAValid && engineerModel.snapshotBValid; onClicked: engineerModel.compareSnapshots() }
                Button { text: qsTr("CLEAR"); onClicked: engineerModel.clearSnapshots() }
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: engineerModel.snapshotDiffSummary
                delegate: Rectangle {
                    width: ListView.view.width
                    height: diffText.implicitHeight + 12
                    color: index % 2 ? "#0E141A" : "#121A21"
                    Label {
                        id: diffText
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.margins: 6
                        text: modelData
                        color: "#C8D2D9"
                        font.pixelSize: 9
                        wrapMode: Text.WrapAnywhere
                    }
                }
            }
        }
    }
}
