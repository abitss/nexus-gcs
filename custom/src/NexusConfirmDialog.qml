import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "NexusTokens.js" as T

Dialog {
    id: root
    property string heading: ""
    property string message: ""
    property string confirmText: qsTr("CONFIRM")
    property bool critical: false
    signal confirmed()

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape

    background: Rectangle {
        color: T.surface
        radius: T.radiusLarge
        border.color: root.critical ? T.critical : T.border
        border.width: 1
    }

    contentItem: ColumnLayout {
        spacing: T.space12
        implicitWidth: root.parent ? Math.min(400, Math.max(280, root.parent.width - T.space24 * 2)) : 400
        Label {
            Layout.fillWidth: true
            text: root.heading
            color: T.textPrimary
            font.pixelSize: T.textSection
            font.bold: true
            wrapMode: Text.WordWrap
        }
        Label {
            Layout.fillWidth: true
            text: root.message
            color: T.textSecondary
            font.pixelSize: T.textBody
            wrapMode: Text.WordWrap
        }
        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            NexusActionButton { text: qsTr("CANCEL"); onClicked: root.close() }
            NexusActionButton {
                text: root.confirmText
                critical: root.critical
                primary: !root.critical
                onClicked: { root.confirmed(); root.close() }
            }
        }
    }
}
