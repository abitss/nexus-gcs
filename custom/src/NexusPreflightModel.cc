#include "NexusPreflightModel.h"

#include <QtCore/QStringList>

#include "HealthAndArmingCheckReport.h"
#include "MultiVehicleManager.h"
#include "NexusAlertManager.h"
#include "NexusHealthModel.h"
#include "SettingsManager.h"
#include "Vehicle.h"
#include "VideoManager.h"
#include "VideoSettings.h"

NexusPreflightModel::NexusPreflightModel(NexusHealthModel *health,
                                         NexusAlertManager *alerts,
                                         QObject *parent)
    : QObject(parent)
    , _health(health)
    , _alerts(alerts)
{
    auto *manager = MultiVehicleManager::instance();
    connect(manager, &MultiVehicleManager::activeVehicleChanged,
            this, &NexusPreflightModel::_activeVehicleChanged);

    _timer.setInterval(400);
    _timer.setTimerType(Qt::CoarseTimer);
    connect(&_timer, &QTimer::timeout, this, &NexusPreflightModel::refresh);
    _timer.start();

    _vehicle = manager->activeVehicle();
    refresh();
}

QString NexusPreflightModel::_stateFromHealth(const QString &healthState, bool unknownIsWarning)
{
    if (healthState == QStringLiteral("CRITICAL")) return QStringLiteral("BLOCKED");
    if (healthState == QStringLiteral("DEGRADED")) return QStringLiteral("WARNING");
    if (healthState == QStringLiteral("NOMINAL")) return QStringLiteral("GO");
    return unknownIsWarning ? QStringLiteral("WARNING") : QStringLiteral("GO");
}

void NexusPreflightModel::_activeVehicleChanged(Vehicle *vehicle)
{
    _vehicle = vehicle;
    refresh();
}

void NexusPreflightModel::updateMission(bool hasMission,
                                        bool valid,
                                        bool verified,
                                        const QString &state,
                                        const QString &detail)
{
    _hasMission = hasMission;
    _missionValid = valid;
    _missionVerified = verified;
    _missionValidationState = state;
    _missionValidationDetail = detail;
    refresh();
}

void NexusPreflightModel::refresh()
{
    _blockedCount = 0;
    _warningCount = 0;

    if (!_vehicle) {
        _vehicleState = QStringLiteral("BLOCKED");
        _vehicleDetail = QStringLiteral("No vehicle connected");
        _datalinkState = QStringLiteral("BLOCKED");
        _datalinkDetail = QStringLiteral("No datalink");

        _sensorsState = QStringLiteral("WARNING");
        _sensorsDetail = QStringLiteral("Sensor health unavailable without a vehicle");
        _navigationState = QStringLiteral("WARNING");
        _navigationDetail = QStringLiteral("Navigation health unavailable without a vehicle");
        _batteryState = QStringLiteral("WARNING");
        _batteryDetail = QStringLiteral("Battery health unavailable without a vehicle");
        _homeState = QStringLiteral("WARNING");
        _homeDetail = QStringLiteral("Home unavailable without a vehicle");
        _geofenceState = QStringLiteral("WARNING");
        _geofenceDetail = QStringLiteral("Geofence unavailable without a vehicle");

        _overallState = QStringLiteral("BLOCKED");
        _overallDetail = QStringLiteral("Connect a vehicle before preflight can pass");
        _blockedCount = 2;
        _warningCount = 5;
        emit preflightChanged();
        return;
    }

    // Vehicle/autopilot authority.
    HealthAndArmingCheckReport *const report = _vehicle->healthAndArmingCheckReport();
    const bool reportSupported = report && report->supported();
    const bool autopilotBlocksArm = reportSupported ? !report->canArm()
                                                    : !_vehicle->prearmError().isEmpty();

    if (autopilotBlocksArm) {
        _vehicleState = QStringLiteral("BLOCKED");
        _vehicleDetail = reportSupported
                       ? QStringLiteral("Autopilot health checks block arming")
                       : _vehicle->prearmError();
    } else {
        _vehicleState = QStringLiteral("GO");
        _vehicleDetail = reportSupported
                       ? QStringLiteral("Autopilot permits arming")
                       : QStringLiteral("Vehicle connected");
    }

    // Sensor health: explicit critical sensor state blocks. Unknown optional
    // sensors stay WARNING, while QGC's arm authority remains decisive.
    const QStringList sensorHealth = {
        _health->imuState(),
        _health->compassState(),
        _health->gyroState(),
        _health->accelerometerState(),
        _health->barometerState(),
    };
    bool sensorCritical = false;
    bool sensorDegraded = false;
    bool sensorUnknown = false;
    for (const QString &state : sensorHealth) {
        sensorCritical |= state == QStringLiteral("CRITICAL");
        sensorDegraded |= state == QStringLiteral("DEGRADED");
        sensorUnknown |= state == QStringLiteral("UNKNOWN");
    }

    if (sensorCritical || autopilotBlocksArm) {
        _sensorsState = QStringLiteral("BLOCKED");
        _sensorsDetail = !_health->sensorFailures().isEmpty()
                       ? QStringLiteral("Sensor failure: %1").arg(_health->sensorFailures().join(QStringLiteral(", ")))
                       : QStringLiteral("Flight-critical sensor check failed");
    } else if (sensorDegraded || sensorUnknown) {
        _sensorsState = QStringLiteral("WARNING");
        _sensorsDetail = sensorUnknown
                       ? QStringLiteral("One or more sensor domains are not reported")
                       : QStringLiteral("One or more sensors are degraded");
    } else {
        _sensorsState = QStringLiteral("GO");
        _sensorsDetail = QStringLiteral("Reported flight sensors nominal");
    }

    // GPS/navigation. GPS is a hard block only when this vehicle requires it.
    const bool gpsRequired = _vehicle->requiresGpsFix();
    if (_health->ekfState() == QStringLiteral("CRITICAL") ||
        (gpsRequired && _health->gpsState() != QStringLiteral("NOMINAL"))) {
        _navigationState = QStringLiteral("BLOCKED");
    } else if (_health->ekfState() == QStringLiteral("DEGRADED") ||
               _health->gpsState() == QStringLiteral("DEGRADED") ||
               _health->gpsState() == QStringLiteral("UNKNOWN") ||
               _health->ekfState() == QStringLiteral("UNKNOWN")) {
        _navigationState = QStringLiteral("WARNING");
    } else {
        _navigationState = QStringLiteral("GO");
    }
    _navigationDetail = QStringLiteral("GPS %1 · EKF %2")
                            .arg(_health->gpsState(), _health->ekfState());

    // Battery.
    _batteryState = _stateFromHealth(_health->batteryState());
    _batteryDetail = _health->batteryDetail();

    // Home is required when GPS/navigation is required.
    if (gpsRequired && _health->homeState() != QStringLiteral("NOMINAL")) {
        _homeState = QStringLiteral("BLOCKED");
    } else {
        _homeState = _stateFromHealth(_health->homeState());
    }
    _homeDetail = _health->homeDetail();

    // Mission. No mission is acceptable for manual flight. A loaded invalid
    // mission is blocked. Valid-but-unverified is a warning.
    if (!_hasMission) {
        _missionState = QStringLiteral("GO");
        _missionDetail = QStringLiteral("No mission loaded; manual flight available");
    } else if (!_missionValid) {
        _missionState = QStringLiteral("BLOCKED");
        _missionDetail = _missionValidationDetail.isEmpty()
                       ? QStringLiteral("Loaded mission is not valid")
                       : _missionValidationDetail;
    } else if (!_missionVerified) {
        _missionState = QStringLiteral("WARNING");
        _missionDetail = QStringLiteral("Mission valid but vehicle readback has not been verified");
    } else {
        _missionState = QStringLiteral("GO");
        _missionDetail = QStringLiteral("Mission valid and vehicle readback verified");
    }

    // Geofence. Critical/breached blocks, unknown is warning.
    _geofenceState = _stateFromHealth(_health->geofenceState());
    _geofenceDetail = _health->geofenceDetail();

    // Datalink.
    _datalinkState = _health->datalinkState() == QStringLiteral("NOMINAL")
                   ? QStringLiteral("GO")
                   : QStringLiteral("BLOCKED");
    _datalinkDetail = _health->datalinkDetail();

    // Payload is optional unless configured. A configured source which is not
    // delivering/decoding video is warning, never a hidden false GO.
    auto *videoManager = VideoManager::instance();
    auto *videoSettings = SettingsManager::instance()->videoSettings();
    const bool payloadConfigured = videoSettings && videoSettings->streamConfigured();
    if (!payloadConfigured) {
        _payloadState = QStringLiteral("GO");
        _payloadDetail = QStringLiteral("Payload video not configured (optional)");
    } else if (videoManager && videoManager->streaming() && videoManager->decoding()) {
        _payloadState = QStringLiteral("GO");
        _payloadDetail = QStringLiteral("Payload video live");
    } else {
        _payloadState = QStringLiteral("WARNING");
        _payloadDetail = QStringLiteral("Payload configured but video is not live");
    }

    // Explicit active failsafe is always a block.
    const bool failsafeBlock = _alerts && _alerts->failsafeActive();

    const QList<QString> states = {
        _vehicleState,
        _sensorsState,
        _navigationState,
        _batteryState,
        _homeState,
        _missionState,
        _geofenceState,
        _datalinkState,
        _payloadState,
    };

    for (const QString &state : states) {
        if (_isBlocked(state)) ++_blockedCount;
        if (_isWarning(state)) ++_warningCount;
    }
    if (failsafeBlock) ++_blockedCount;

    if (_blockedCount > 0) {
        _overallState = QStringLiteral("BLOCKED");
        _overallDetail = failsafeBlock
                       ? QStringLiteral("Active autopilot failsafe. Resolve the condition before flight.")
                       : QStringLiteral("%1 blocking check(s) · %2 warning(s)")
                             .arg(_blockedCount)
                             .arg(_warningCount);
    } else if (_warningCount > 0 ||
               (reportSupported && report->hasWarningsOrErrors())) {
        _overallState = QStringLiteral("WARNING");
        _overallDetail = QStringLiteral("No hard block, but %1 condition(s) require review")
                             .arg(qMax(1, _warningCount));
    } else {
        _overallState = QStringLiteral("GO");
        _overallDetail = QStringLiteral("All required preflight checks passed");
    }

    emit preflightChanged();
}
