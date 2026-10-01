#include "NexusRecoveryModel.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QSettings>
#include <QtCore/QDebug>
#include <QtGui/QGuiApplication>

#include "MAVLinkProtocol.h"
#include "MultiVehicleManager.h"
#include "NexusDeviceHealthModel.h"
#include "NexusPayloadModel.h"
#include "NexusPlanVerifier.h"
#include "NexusSecurityModel.h"
#include "Vehicle.h"
#include "VehicleLinkManager.h"

NexusRecoveryModel::NexusRecoveryModel(NexusPayloadModel *payload,
                                       NexusDeviceHealthModel *deviceHealth,
                                       NexusPlanVerifier *planVerifier,
                                       NexusSecurityModel *security,
                                       QObject *parent)
    : QObject(parent)
    , _payload(payload)
    , _deviceHealth(deviceHealth)
    , _planVerifier(planVerifier)
    , _security(security)
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("NexusRecovery"));
    _previousUncleanExit = settings.value(QStringLiteral("running"), false).toBool();
    settings.setValue(QStringLiteral("running"), true);
    settings.setValue(QStringLiteral("lastStartUtc"), QDateTime::currentDateTimeUtc());
    settings.sync();

    if (_previousUncleanExit) {
        _record(QStringLiteral("APP_RESTART"), QStringLiteral("WARNING"),
                QStringLiteral("Previous NEXUS session did not record a clean shutdown. Vehicle state must be re-verified."));
    }

    if (qApp) {
        connect(qApp, &QCoreApplication::aboutToQuit, this, &NexusRecoveryModel::_markCleanExit);
    }
    if (qGuiApp) {
        connect(qGuiApp, &QGuiApplication::applicationStateChanged,
                this, &NexusRecoveryModel::_applicationStateChanged);
        _appState = _applicationStateName(qGuiApp->applicationState());
    }

    auto *manager = MultiVehicleManager::instance();
    connect(manager, &MultiVehicleManager::activeVehicleChanged,
            this, &NexusRecoveryModel::_activeVehicleChanged);
    _setVehicle(manager->activeVehicle());

    if (_payload) {
        connect(_payload, &NexusPayloadModel::payloadChanged,
                this, &NexusRecoveryModel::_payloadChanged);
        _videoLost = _payload->videoLost();
    }

    if (_deviceHealth) {
        connect(_deviceHealth, &NexusDeviceHealthModel::deviceHealthChanged,
                this, &NexusRecoveryModel::_deviceChanged);
        _previousUsbConnected = _deviceHealth->usbConnected();
        _deviceChanged();
    }

    if (_planVerifier) {
        connect(_planVerifier, &NexusPlanVerifier::stateChanged,
                this, &NexusRecoveryModel::_planStateChanged);
        _missionWasBusy = _planVerifier->busy();
    }

    connect(MAVLinkProtocol::instance(), &MAVLinkProtocol::messageReceived,
            this, &NexusRecoveryModel::_mavlinkMessageReceived);
}

NexusRecoveryModel::~NexusRecoveryModel()
{
    _markCleanExit();
}

QString NexusRecoveryModel::overallState() const
{
    if (_fcRebootDetected || _interruptedMission || _storageBlocked) return QStringLiteral("RECOVERY REQUIRED");
    if (_telemetryLost || _usbLost || _udpLost || _videoLost || _permissionRevoked || _previousUncleanExit) {
        return QStringLiteral("DEGRADED");
    }
    return QStringLiteral("NOMINAL");
}

QString NexusRecoveryModel::_applicationStateName(Qt::ApplicationState state)
{
    switch (state) {
    case Qt::ApplicationActive: return QStringLiteral("FOREGROUND");
    case Qt::ApplicationInactive: return QStringLiteral("INACTIVE");
    case Qt::ApplicationSuspended: return QStringLiteral("SUSPENDED");
    case Qt::ApplicationHidden: return QStringLiteral("BACKGROUND");
    }
    return QStringLiteral("UNKNOWN");
}

void NexusRecoveryModel::_applicationStateChanged(Qt::ApplicationState state)
{
    const QString next = _applicationStateName(state);
    if (_appState == next) return;

    _appState = next;
    _record(state == Qt::ApplicationActive ? QStringLiteral("APP_FOREGROUND") : QStringLiteral("APP_BACKGROUND"),
            QStringLiteral("INFO"),
            QStringLiteral("Application state changed to %1.").arg(next));
    emit recoveryChanged();
}

void NexusRecoveryModel::_activeVehicleChanged(Vehicle *vehicle)
{
    _setVehicle(vehicle);
}

void NexusRecoveryModel::_setVehicle(Vehicle *vehicle)
{
    if (_vehicle && _vehicle->vehicleLinkManager()) {
        disconnect(_vehicle->vehicleLinkManager(), nullptr, this, nullptr);
    }

    _vehicle = vehicle;
    _haveBootCounter = false;
    _lastBootMs = 0;
    _fcRebootDetected = false;

    if (_vehicle && _vehicle->vehicleLinkManager()) {
        connect(_vehicle->vehicleLinkManager(), &VehicleLinkManager::communicationLostChanged,
                this, &NexusRecoveryModel::_communicationLostChanged);
        _telemetryLost = _vehicle->vehicleLinkManager()->communicationLost();
    } else {
        _telemetryLost = false;
    }

    _record(vehicle ? QStringLiteral("VEHICLE_SESSION_CONNECTED") : QStringLiteral("VEHICLE_SESSION_CLOSED"),
            QStringLiteral("INFO"),
            vehicle ? QStringLiteral("Vehicle session established; stale state assumptions discarded.")
                    : QStringLiteral("No active vehicle session."));
    emit recoveryChanged();
}

void NexusRecoveryModel::_communicationLostChanged(bool lost)
{
    if (_telemetryLost == lost) return;
    _telemetryLost = lost;

    const QString primary = _vehicle && _vehicle->vehicleLinkManager()
        ? _vehicle->vehicleLinkManager()->primaryLinkName().toUpper()
        : QString();

    _udpLost = lost && primary.contains(QStringLiteral("UDP"));
    _usbLost = lost && !_previousUsbConnected && !_udpLost;

    if (lost) {
        _record(_udpLost ? QStringLiteral("UDP_LOSS") : QStringLiteral("TELEMETRY_LOSS"),
                QStringLiteral("CRITICAL"),
                _udpLost ? QStringLiteral("Primary UDP telemetry heartbeat timed out.")
                         : QStringLiteral("Vehicle telemetry heartbeat timed out."));

        if (_planVerifier && (_planVerifier->busy() || _missionWasBusy)) {
            _interruptedMission = true;
            _planVerifier->reset();
            _record(QStringLiteral("MISSION_UPLOAD_INTERRUPTED"), QStringLiteral("CRITICAL"),
                    QStringLiteral("Mission transfer was interrupted. Verification was invalidated and must be repeated."));
        }
    } else {
        _udpLost = false;
        _record(QStringLiteral("TELEMETRY_RESTORED"), QStringLiteral("INFO"),
                QStringLiteral("Vehicle telemetry resumed. Fresh vehicle state is required before trusting prior assumptions."));
    }

    emit recoveryChanged();
}

void NexusRecoveryModel::_payloadChanged()
{
    if (!_payload) return;
    const bool lost = _payload->videoLost();
    if (lost != _videoLost) {
        _videoLost = lost;
        _record(lost ? QStringLiteral("VIDEO_LOSS") : QStringLiteral("VIDEO_RESTORED"),
                lost ? QStringLiteral("WARNING") : QStringLiteral("INFO"),
                lost ? QStringLiteral("Configured video stream stopped decoding.")
                     : QStringLiteral("Video decoding resumed."));
        emit recoveryChanged();
    }
}

void NexusRecoveryModel::_deviceChanged()
{
    if (!_deviceHealth) return;

    const bool usbNow = _deviceHealth->usbConnected();
    if (_previousUsbConnected && !usbNow) {
        _usbLost = true;
        _record(QStringLiteral("USB_DISCONNECT"), QStringLiteral("CRITICAL"),
                QStringLiteral("Previously enumerated USB/serial device disappeared."));
    } else if (!_previousUsbConnected && usbNow && _usbLost) {
        _usbLost = false;
        _record(QStringLiteral("USB_RESTORED"), QStringLiteral("INFO"),
                QStringLiteral("USB/serial device re-enumerated."));
    }
    _previousUsbConnected = usbNow;

    const bool full = _deviceHealth->storageTotalBytes() > 0 && _deviceHealth->storageFreeBytes() == 0;
    if (full != _storageBlocked) {
        _storageBlocked = full;
        _record(full ? QStringLiteral("STORAGE_FULL") : QStringLiteral("STORAGE_RECOVERED"),
                full ? QStringLiteral("CRITICAL") : QStringLiteral("INFO"),
                full ? QStringLiteral("Local storage reports no available bytes. Recording/log writes must not be trusted.")
                     : QStringLiteral("Local storage is writable again."));
    }

    const bool permissionHealthy =
        _deviceHealth->cameraPermission() != QStringLiteral("DENIED") &&
        _deviceHealth->locationPermission() != QStringLiteral("DENIED") &&
        _deviceHealth->storagePermission() != QStringLiteral("DENIED");

    if (_previousPermissionHealthy && !permissionHealthy) {
        _permissionRevoked = true;
        _record(QStringLiteral("PERMISSION_REVOKED"), QStringLiteral("WARNING"),
                QStringLiteral("One or more runtime permissions were revoked."));
    } else if (!_previousPermissionHealthy && permissionHealthy) {
        _permissionRevoked = false;
        _record(QStringLiteral("PERMISSION_RESTORED"), QStringLiteral("INFO"),
                QStringLiteral("Required permission state recovered."));
    }
    _previousPermissionHealthy = permissionHealthy;

    emit recoveryChanged();
}

void NexusRecoveryModel::_planStateChanged()
{
    if (!_planVerifier) return;

    const bool busyNow = _planVerifier->busy();
    if (_missionWasBusy && !busyNow && !_planVerifier->verified() &&
        _planVerifier->state() != QStringLiteral("NOT VERIFIED")) {
        _interruptedMission = true;
        _record(QStringLiteral("MISSION_VERIFY_FAILED"), QStringLiteral("WARNING"),
                QStringLiteral("Mission transfer/verification ended without a verified readback."));
    }
    _missionWasBusy = busyNow;
    emit recoveryChanged();
}

void NexusRecoveryModel::_mavlinkMessageReceived(LinkInterface *, const mavlink_message_t &message)
{
    if (!_vehicle || message.sysid != _vehicle->id()) return;
    if (message.msgid != MAVLINK_MSG_ID_SYSTEM_TIME) return;

    mavlink_system_time_t systemTime{};
    mavlink_msg_system_time_decode(&message, &systemTime);
    if (systemTime.time_boot_ms == 0) return;

    if (_haveBootCounter && bootCounterIndicatesReboot(_lastBootMs, systemTime.time_boot_ms)) {
        _fcRebootDetected = true;
        _interruptedMission = _interruptedMission || (_planVerifier && _planVerifier->busy());
        if (_planVerifier) _planVerifier->reset();
        _record(QStringLiteral("FC_REBOOT"), QStringLiteral("CRITICAL"),
                QStringLiteral("Autopilot boot-time counter reset. Previous mode/home/mission assumptions were invalidated."));
    }

    _lastBootMs = systemTime.time_boot_ms;
    _haveBootCounter = true;
    emit recoveryChanged();
}

bool NexusRecoveryModel::bootCounterIndicatesReboot(quint32 previousMs, quint32 currentMs)
{
    // Ignore small clock/report jitter; a drop greater than five seconds means
    // the FC boot-relative clock materially reset.
    return previousMs > currentMs && (previousMs - currentMs) > 5000U;
}

void NexusRecoveryModel::_record(const QString &type, const QString &severity, const QString &detail)
{
    QVariantMap event{
        {QStringLiteral("time"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("type"), type},
        {QStringLiteral("severity"), severity},
        {QStringLiteral("detail"), detail}
    };
    _events.prepend(event);
    while (_events.size() > 100) _events.removeLast();

    _lastRecoveryEvent = QStringLiteral("%1 · %2").arg(type, detail);
    qInfo().noquote() << "NEXUS_RECOVERY" << type << severity << detail;

    if (_security) {
        _security->recordAudit(QStringLiteral("recovery.%1").arg(type.toLower()),
                               {{QStringLiteral("severity"), severity},
                                {QStringLiteral("detail"), detail}});
    }
}

void NexusRecoveryModel::_markCleanExit()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("NexusRecovery"));
    settings.setValue(QStringLiteral("running"), false);
    settings.setValue(QStringLiteral("lastCleanExitUtc"), QDateTime::currentDateTimeUtc());
    settings.sync();
}

void NexusRecoveryModel::acknowledgeInterruptedMission()
{
    if (!_interruptedMission) return;
    _interruptedMission = false;
    _record(QStringLiteral("MISSION_INTERRUPTION_ACK"), QStringLiteral("INFO"),
            QStringLiteral("Interrupted mission state acknowledged. Mission still requires fresh validation before trust."));
    emit recoveryChanged();
}

void NexusRecoveryModel::clearRecoveredEvents()
{
    _events.clear();
    _previousUncleanExit = false;
    _fcRebootDetected = false;
    if (!_telemetryLost) _udpLost = false;
    if (_deviceHealth && _deviceHealth->usbConnected()) _usbLost = false;
    if (_payload && !_payload->videoLost()) _videoLost = false;
    if (_deviceHealth && _deviceHealth->storageFreeBytes() > 0) _storageBlocked = false;
    _lastRecoveryEvent = QStringLiteral("Recovered event history cleared");
    emit recoveryChanged();
}

bool NexusRecoveryModel::validateRecoveryMission(const QString &path)
{
    if (!_security) return false;
    const bool ok = _security->validateMissionFile(path);
    _record(ok ? QStringLiteral("MISSION_RECOVERY_VALID") : QStringLiteral("MISSION_CORRUPT"),
            ok ? QStringLiteral("INFO") : QStringLiteral("CRITICAL"),
            ok ? QStringLiteral("Recovery mission file passed structural validation.")
               : QStringLiteral("Recovery mission file failed validation and was not trusted."));
    return ok;
}
