import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var securityModel
    signal closeRequested()

    width: Math.min(660, parent ? parent.width * 0.56 : 660)
    color: "#F70A0F14"
    border.color: "#2B3944"
    border.width: 1
    radius: 12
    z: 7100

    Dialog {
        id: authDialog
        property string role: "ENGINEER"
        modal: true
        title: qsTr("Authenticate %1").arg(role)
        standardButtons: Dialog.Cancel

        ColumnLayout {
            width: Math.min(420, root.width - 80)
            TextField {
                id: passphrase
                Layout.fillWidth: true
                echoMode: TextInput.Password
                placeholderText: qsTr("Offline passphrase")
                onAccepted: authButton.clicked()
            }
            Label {
                Layout.fillWidth: true
                visible: securityModel.lockedOut
                text: qsTr("Locked for %1 seconds").arg(securityModel.lockoutSeconds)
                color: "#E0B85F"
            }
            Label {
                Layout.fillWidth: true
                visible: securityModel.lastError.length > 0
                text: securityModel.lastError
                color: "#E26A6A"
                wrapMode: Text.WordWrap
            }
            Button {
                id: authButton
                Layout.alignment: Qt.AlignRight
                text: qsTr("AUTHENTICATE")
                enabled: passphrase.text.length > 0
                onClicked: {
                    if (securityModel.authenticate(authDialog.role, passphrase.text)) {
                        passphrase.text = ""
                        authDialog.close()
                    }
                }
            }
        }
    }

    Dialog {
        id: engineerCredentialDialog
        modal: true
        title: qsTr("Set Engineer Credential")
        standardButtons: Dialog.Cancel
        ColumnLayout {
            width: Math.min(420, root.width - 80)
            Label {
                Layout.fillWidth: true
                text: qsTr("Admin-only. Configure or rotate the local Engineer passphrase.")
                wrapMode: Text.WordWrap
            }
            TextField {
                id: newEngineerPass
                Layout.fillWidth: true
                echoMode: TextInput.Password
                placeholderText: qsTr("Minimum 10 characters")
            }
            Button {
                Layout.alignment: Qt.AlignRight
                text: qsTr("SAVE ENGINEER CREDENTIAL")
                enabled: securityModel.canAdmin && newEngineerPass.text.length >= 10
                onClicked: {
                    if (securityModel.setEngineerCredential(newEngineerPass.text)) {
                        newEngineerPass.text = ""
                        engineerCredentialDialog.close()
                    }
                }
            }
        }
    }

    Dialog {
        id: bootstrapDialog
        modal: true
        title: qsTr("Bootstrap Local Admin")
        standardButtons: Dialog.Cancel
        ColumnLayout {
            width: Math.min(420, root.width - 80)
            Label {
                Layout.fillWidth: true
                text: qsTr("Create the first offline Admin credential. The passphrase is never stored; Nexus stores only a salted PBKDF2 verifier.")
                wrapMode: Text.WordWrap
            }
            TextField {
                id: bootstrapPass
                Layout.fillWidth: true
                echoMode: TextInput.Password
                placeholderText: qsTr("Minimum 10 characters")
            }
            Button {
                Layout.alignment: Qt.AlignRight
                text: qsTr("CREATE ADMIN")
                enabled: bootstrapPass.text.length >= 10
                onClicked: {
                    if (securityModel.bootstrapAdmin(bootstrapPass.text)) {
                        bootstrapPass.text = ""
                        bootstrapDialog.close()
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
                Label { text: qsTr("SECURITY"); color: "#F6FAFC"; font.pixelSize: 19; font.bold: true }
                Label { text: qsTr("LOCAL TRUST + RELEASE INTEGRITY"); color: "#83919A"; font.pixelSize: 9; font.bold: true }
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                implicitWidth: 100; implicitHeight: 30; radius: 7
                color: "#0D141A"
                border.color: securityModel.currentRole === "OPERATOR" ? "#60717E" : "#2C9B7F"
                Label { anchors.centerIn: parent; text: securityModel.currentRole; color: parent.border.color; font.pixelSize: 9; font.bold: true }
            }
            Button { text: "×"; onClicked: root.closeRequested() }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 86
            radius: 8
            color: "#10171E"
            border.color: "#2B3944"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                ColumnLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("OFFLINE AUTHENTICATION"); color: "#EEF3F6"; font.bold: true }
                    Label {
                        Layout.fillWidth: true
                        text: securityModel.adminConfigured
                              ? qsTr("Local Admin credential configured. Engineer/Admin sessions require offline passphrase authentication.")
                              : qsTr("No local Admin credential exists yet. Bootstrap is required before privileged roles can be secured.")
                        color: "#87949D"; font.pixelSize: 8; wrapMode: Text.WordWrap
                    }
                }
                Button {
                    visible: !securityModel.adminConfigured
                    text: qsTr("BOOTSTRAP ADMIN")
                    onClicked: bootstrapDialog.open()
                }
                Button {
                    visible: securityModel.adminConfigured
                    text: qsTr("ENGINEER")
                    onClicked: { authDialog.role = "ENGINEER"; authDialog.open() }
                }
                Button {
                    visible: securityModel.adminConfigured
                    text: qsTr("ADMIN")
                    onClicked: { authDialog.role = "ADMIN"; authDialog.open() }
                }
                Button {
                    visible: securityModel.canAdmin
                    text: securityModel.engineerConfigured ? qsTr("ROTATE ENGINEER") : qsTr("SET ENGINEER")
                    onClicked: engineerCredentialDialog.open()
                }
                Button {
                    visible: securityModel.authenticated
                    text: qsTr("LOCK")
                    onClicked: securityModel.lock()
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ColumnLayout {
                width: parent.width
                spacing: 8

                Repeater {
                    model: [
                        {title: qsTr("ROLES / PERMISSIONS"), value: qsTr("OPERATOR · ENGINEER · ADMIN"), detail: qsTr("Privileged engineering/admin actions require authenticated local roles.")},
                        {title: qsTr("SECURE LOCAL CONFIG"), value: qsTr("PBKDF2-SHA256"), detail: qsTr("Salted verifier only. No plaintext password/PIN is stored.")},
                        {title: qsTr("AUDIT TRAIL"), value: securityModel.verifyAuditTrail() ? qsTr("CHAIN VALID") : qsTr("VERIFY FAILED"), detail: securityModel.auditPath},
                        {title: qsTr("MISSION FILE VALIDATION"), value: qsTr("QGC PLAN v1"), detail: qsTr("Size, JSON structure, fileType/version and required mission/fence/rally objects checked before trusted import.")},
                        {title: qsTr("UPDATE VERIFICATION"), value: qsTr("SHA-256 + ANDROID SIGNER"), detail: qsTr("Package checksum is verified against trusted manifest; Android update installation additionally enforces the production signing identity.")},
                        {title: qsTr("REPOSITORY SECRETS"), value: qsTr("FORBIDDEN"), detail: qsTr("CI scans for secret patterns, private keys, keystores and release credentials.")},
                        {title: qsTr("DEPENDENCY SCANNING"), value: qsTr("CI ENFORCED"), detail: qsTr("Dependency review and CodeQL analyze pull requests/releases.")},
                        {title: qsTr("RELEASE KEY HYGIENE"), value: qsTr("CI SECRETS ONLY"), detail: qsTr("Production keystore is injected at release time and never committed or uploaded as a build artifact.")}
                    ]

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 76
                        radius: 8
                        color: "#10171E"
                        border.color: "#293740"
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 9
                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: modelData.title; color: "#EDF3F6"; font.pixelSize: 10; font.bold: true }
                                Item { Layout.fillWidth: true }
                                Label { text: modelData.value; color: "#7FC5AD"; font.pixelSize: 8; font.bold: true }
                            }
                            Label { Layout.fillWidth: true; text: modelData.detail; color: "#83919A"; font.pixelSize: 8; wrapMode: Text.WordWrap }
                        }
                    }
                }
            }
        }
    }
}
