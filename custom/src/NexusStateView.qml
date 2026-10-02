import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "NexusTokens.js" as T

Item {
    id: root
    property string state: "empty" // loading | empty | error
    property string title: ""
    property string message: ""
    property string actionText: ""
    property bool actionEnabled: true
    signal action()

    implicitHeight: 180
    implicitWidth: 320

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - T.space24 * 2, 440)
        spacing: T.space12

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            width: 48; height: 48; radius: 24
            color: root.state === "error" ? T.criticalSoft
                 : root.state === "loading" ? T.infoSoft : T.surfaceRaised
            border.color: root.state === "error" ? T.critical
                        : root.state === "loading" ? T.info : T.border

            BusyIndicator {
                anchors.centerIn: parent
                running: root.state === "loading"
                visible: running
                width: 28; height: 28
            }
            Label {
                anchors.centerIn: parent
                visible: root.state !== "loading"
                text: root.state === "error" ? "!" : "—"
                color: root.state === "error" ? T.critical : T.textSecondary
                font.pixelSize: 22; font.bold: true
            }
        }

        Label {
            Layout.fillWidth: true
            text: root.title
            color: T.textPrimary
            font.pixelSize: T.textSection
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
        Label {
            Layout.fillWidth: true
            text: root.message
            color: T.textSecondary
            font.pixelSize: T.textBody
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
        NexusActionButton {
            Layout.alignment: Qt.AlignHCenter
            visible: root.actionText.length > 0
            text: root.actionText
            enabled: root.actionEnabled
            onClicked: root.action()
        }
    }
}
