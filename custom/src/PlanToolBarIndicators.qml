import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import QGroundControl
import QGroundControl.Controls
import QGroundControl.FactControls

RowLayout {
    required property var planMasterController
    property bool showRallyPointsHelp: false

    signal toolbarButtonClicked()

    id: root
    spacing: Math.max(5, ScreenTools.defaultFontPixelWidth * 0.7)

    property var _planMasterController: planMasterController
    property var _missionController: _planMasterController.missionController
    property var _geoFenceController: _planMasterController.geoFenceController
    property var _rallyPointController: _planMasterController.rallyPointController
    property bool _controllerOffline: _planMasterController.offline
    property var _saveDirty: _planMasterController.dirtyForSave
    property var _uploadDirty: _planMasterController.dirtyForUpload
    property var _syncInProgress: _planMasterController.syncInProgress
    property var _visualItems: _missionController.visualItems
    property bool _hasPlanItems: _planMasterController.containsItems

    property var _validation: NexusPlanVerifier.validationStatus(_planMasterController)
    property string _validationState: _validation.state || "INVALID"
    property string _validationMessage: _validation.message || ""
    property bool _validationReady: !!_validation.ready

    function refreshValidation() {
        _validation = NexusPlanVerifier.validationStatus(_planMasterController)
    }

    function _uploadClicked() {
        _planMasterController.upload()
    }

    function _verifyUploadClicked() {
        refreshValidation()
        if (!_validationReady || _controllerOffline) {
            QGroundControl.showMessageDialog(root, qsTr("Mission Validation"), _validationMessage)
            return
        }
        NexusPlanVerifier.verifyUpload(_planMasterController)
    }

    function _downloadClicked() {
        if (_saveDirty) {
            QGroundControl.showMessageDialog(root, qsTr("Download"),
                                         qsTr("You have unsaved changes. Downloading from the Vehicle will lose these changes. Are you sure?"),
                                         Dialog.Yes | Dialog.Cancel,
                                         function() { _planMasterController.loadFromVehicle() })
        } else {
            _planMasterController.loadFromVehicle()
        }
    }

    function _openButtonClicked() {
        let planSafeOnDisk = !_saveDirty && _planMasterController.currentPlanFile !== ""
        let unsentChanges = _uploadDirty && !_controllerOffline && !planSafeOnDisk
        if (_saveDirty || unsentChanges) {
            let msg
            if (_saveDirty && unsentChanges) {
                msg = qsTr("You have unsaved/unsent changes. Loading a new Plan will lose these changes. Are you sure?")
            } else if (_saveDirty) {
                msg = qsTr("You have unsaved changes. Loading a new Plan will lose these changes. Are you sure?")
            } else {
                msg = qsTr("You have unsent changes. Loading a new Plan will lose these changes. Are you sure?")
            }
            QGroundControl.showMessageDialog(root, qsTr("Open Plan"), msg,
                                        Dialog.Yes | Dialog.Cancel,
                                        function() { _planMasterController.loadFromSelectedFile() })
        } else {
            _planMasterController.loadFromSelectedFile()
        }
    }

    function _saveButtonClicked() {
        if (_planMasterController.currentPlanFile === "") {
            _planMasterController.saveToSelectedFile()
        } else {
            _planMasterController.saveToCurrent()
        }
    }

    function _saveAsKMLClicked() {
        if (_visualItems.count > 1) {
            _planMasterController.saveKmlToSelectedFile()
        }
    }

    function _storageClearButtonClicked() {
        QGroundControl.showMessageDialog(root, qsTr("Clear"),
                                     qsTr("Are you sure you want to remove all the items from the plan editor?"),
                                     Dialog.Yes | Dialog.Cancel,
                                     function() { _planMasterController.removeAll(); NexusPlanVerifier.reset(); })
    }

    function _vehicleClearButtonClicked() {
        QGroundControl.showMessageDialog(root, qsTr("Clear"),
                                     qsTr("Are you sure you want to remove the plan from the vehicle and the plan editor?"),
                                     Dialog.Yes | Dialog.Cancel,
                                     function() {
                                        _planMasterController.removeAllFromVehicle()
                                        NexusPlanVerifier.reset()
                                     })
    }

    function _clearClicked() {
        if (_planMasterController.offline) {
            _storageClearButtonClicked()
        } else {
            _vehicleClearButtonClicked()
        }
    }

    Timer {
        interval: 400
        running: root.visible
        repeat: true
        onTriggered: root.refreshValidation()
    }

    Connections {
        target: NexusPlanVerifier
        function onStateChanged() { root.refreshValidation() }
    }

    QGCPalette { id: qgcPal }

    Rectangle {
        objectName: "nexusPlanReadyChip"
        Layout.preferredWidth: 126
        Layout.preferredHeight: 34
        radius: 7
        color: "#D70B1016"
        border.width: 1
        border.color: NexusPlanVerifier.verified ? "#2C9B7F"
                     : (_validationReady ? "#4F8FB8" : "#D6A84A")

        Column {
            anchors.centerIn: parent
            spacing: 0

            QGCLabel {
                anchors.horizontalCenter: parent.horizontalCenter
                text: NexusPlanVerifier.busy ? NexusPlanVerifier.state : _validationState
                color: NexusPlanVerifier.verified ? "#A8F3D6"
                       : (_validationReady ? "#C7E6FA" : "#F0D49A")
                font.pixelSize: 10
                font.bold: true
            }

            QGCLabel {
                anchors.horizontalCenter: parent.horizontalCenter
                text: _missionController.visualItems ? qsTr("%1 items · %2 m")
                        .arg(Math.max(0, _missionController.visualItems.count - 1))
                        .arg(Math.round(_missionController.missionTotalDistance)) : "--"
                color: "#83919D"
                font.pixelSize: 8
            }
        }
    }

    QGCButton {
        objectName: "planToolbar_openButton"
        text: qsTr("Open")
        iconSource: "/qmlimages/Plan.svg"
        enabled: !_syncInProgress
        onClicked: { toolbarButtonClicked(); _openButtonClicked() }
    }

    QGCButton {
        objectName: "planToolbar_saveButton"
        text: qsTr("Save")
        iconSource: "/res/SaveToDisk.svg"
        enabled: !_syncInProgress && _hasPlanItems
        primary: _saveDirty
        onClicked: { toolbarButtonClicked(); _saveButtonClicked() }
    }

    QGCButton {
        id: uploadButton
        objectName: "planToolbar_uploadButton"
        text: qsTr("Upload")
        iconSource: "/res/UploadToVehicle.svg"
        enabled: !_syncInProgress && _hasPlanItems && !_controllerOffline
        visible: !_syncInProgress
        primary: _uploadDirty && !_controllerOffline
        onClicked: { toolbarButtonClicked(); _uploadClicked() }
    }

    QGCButton {
        objectName: "nexusPlanVerifyButton"
        text: NexusPlanVerifier.busy ? NexusPlanVerifier.state : qsTr("Verify")
        enabled: !NexusPlanVerifier.busy && !_syncInProgress && !_controllerOffline && _hasPlanItems
        primary: _validationReady && !NexusPlanVerifier.verified
        onClicked: { toolbarButtonClicked(); _verifyUploadClicked() }

        ToolTip.visible: hovered
        ToolTip.text: NexusPlanVerifier.verified
                      ? qsTr("Vehicle readback verified · %1").arg(NexusPlanVerifier.fingerprint)
                      : _validationMessage
    }

    QGCButton {
        objectName: "nexusPlanDownloadButton"
        text: qsTr("Download")
        iconSource: "/res/DownloadFromVehicle.svg"
        enabled: !_syncInProgress && !_controllerOffline
        visible: !_syncInProgress
        onClicked: { toolbarButtonClicked(); _downloadClicked() }
    }

    QGCButton {
        objectName: "planToolbar_clearButton"
        text: qsTr("Clear")
        iconSource: "/res/TrashCan.svg"
        enabled: !_syncInProgress
        onClicked: { toolbarButtonClicked(); _clearClicked() }
    }

    QGCButton {
        objectName: "planToolbar_hamburgerButton"
        iconSource: "qrc:/qmlimages/Hamburger.svg"

        onClicked: {
            let position = Qt.point(width, height / 2)
            position = mapToItem(globals.parent, position)
            var dropPanel = hamburgerDropPanelComponent.createObject(mainWindow, { clickRect: Qt.rect(position.x, position.y, 0, 0) })
            dropPanel.open()
        }
    }

    QGCLabel {
        text: qsTr("Click in map to add rally points")
        visible: root.showRallyPointsHelp
        Layout.alignment: Qt.AlignVCenter
    }

    Component {
        id: hamburgerDropPanelComponent

        DropPanel {
            id: dropPanel

            sourceComponent: Component {
                ColumnLayout {
                    spacing: ScreenTools.defaultFontPixelHeight / 2

                    QGCLabel {
                        Layout.fillWidth: true
                        text: root._validationMessage
                        wrapMode: Text.WordWrap
                        color: qgcPal.text
                    }

                    QGCButton {
                        objectName: "planToolbar_saveAsButton"
                        Layout.fillWidth: true
                        text: qsTr("Save as...")
                        enabled: !_syncInProgress && _hasPlanItems
                        onClicked: {
                            dropPanel.close()
                            _planMasterController.saveToSelectedFile()
                        }
                    }

                    QGCButton {
                        Layout.fillWidth: true
                        text: qsTr("Save as KML")
                        enabled: !_syncInProgress && _hasPlanItems
                        onClicked: {
                            dropPanel.close()
                            _saveAsKMLClicked()
                        }
                    }
                }
            }
        }
    }
}
