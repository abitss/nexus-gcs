#include "NexusHealthModel.h"

#include <QtCore/QVariant>
#include <QtCore/QtMath>

#include "BatteryFactGroupListModel.h"
#include "Fact.h"
#include "HealthAndArmingCheckReport.h"
#include "MultiVehicleManager.h"
#include "QGCMAVLink.h"
#include "QmlObjectListModel.h"
#include "SysStatusSensorInfo.h"
#include "Vehicle.h"
#include "VehicleEstimatorStatusFactGroup.h"
#include "VehicleGPSFactGroup.h"
#include "VehicleLinkManager.h"

NexusHealthModel::NexusHealthModel(QObject *parent)
    : QObject(parent)
{
    _timer.setInterval(500);
    _timer.setTimerType(Qt::CoarseTimer);
    connect(&_timer, &QTimer::timeout, this, &NexusHealthModel::refresh);

    auto *manager = MultiVehicleManager::instance();
    connect(manager, &MultiVehicleManager::activeVehicleChanged,
            this, &NexusHealthModel::_activeVehicleChanged);
    _setVehicle(manager->activeVehicle());

    _timer.start();
}

void NexusHealthModel::_setVehicle(Vehicle *vehicle)
{
    if (_vehicle) {
        disconnect(_vehicle, nullptr, this, nullptr);
    }

    _vehicle = vehicle;
    _fenceStatusSeen = false;
    _fenceBreached = false;
    _fenceBreachType = FENCE_BREACH_NONE;

    if (_vehicle) {
        connect(_vehicle, &Vehicle::mavlinkMessageReceived,
                this, &NexusHealthModel::_mavlinkMessageReceived);
        connect(_vehicle, &QObject::destroyed, this, [this]() {
            _vehicle = nullptr;
            refresh();
        });
    }

    refresh();
}

void NexusHealthModel::_activeVehicleChanged(Vehicle *vehicle)
{
    _setVehicle(vehicle);
}

int NexusHealthModel::_severity(const QString &state)
{
    if (state == QStringLiteral("CRITICAL")) return 3;
    if (state == QStringLiteral("DEGRADED")) return 2;
    if (state == QStringLiteral("NOMINAL")) return 1;
    return 0;
}

QString NexusHealthModel::_worstState(const QStringList &states)
{
    QString worst = QStringLiteral("UNKNOWN");
    int maxSeverity = 0;
    for (const QString &state : states) {
        const int s = _severity(state);
        if (s > maxSeverity) {
            maxSeverity = s;
            worst = state;
        }
    }
    return worst;
}

NexusHealthModel::StateDetail NexusHealthModel::_sensorState(MAV_SYS_STATUS_SENSOR sensor) const
{
    if (!_vehicle) {
        return {QStringLiteral("UNKNOWN"), QStringLiteral("No vehicle")};
    }

    auto *info = qobject_cast<SysStatusSensorInfo*>(_vehicle->sysStatusSensorInfo());
    if (!info) {
        return {QStringLiteral("UNKNOWN"), QStringLiteral("SYS_STATUS unavailable")};
    }

    const QString target = QGCMAVLink::mavSysStatusSensorToString(sensor);
    const QStringList names = info->sensorNames();
    const QStringList status = info->sensorStatus();

    for (int i = 0; i < names.size() && i < status.size(); ++i) {
        if (names.at(i) != target) {
            continue;
        }

        if (status.at(i) == QStringLiteral("Error")) {
            return {QStringLiteral("CRITICAL"), QStringLiteral("%1 reports Error").arg(target)};
        }
        if (status.at(i) == QStringLiteral("Normal")) {
            return {QStringLiteral("NOMINAL"), QStringLiteral("%1 healthy").arg(target)};
        }
        if (status.at(i) == QStringLiteral("Disabled")) {
            return {QStringLiteral("DEGRADED"), QStringLiteral("%1 disabled").arg(target)};
        }

        return {QStringLiteral("UNKNOWN"), status.at(i)};
    }

    return {QStringLiteral("UNKNOWN"), QStringLiteral("%1 not reported").arg(target)};
}

void NexusHealthModel::_mavlinkMessageReceived(const mavlink_message_t &message)
{
    if (message.msgid != MAVLINK_MSG_ID_FENCE_STATUS) {
        return;
    }

    mavlink_fence_status_t fence{};
    mavlink_msg_fence_status_decode(&message, &fence);

    _fenceStatusSeen = true;
    _fenceBreached = fence.breach_status == 1;
    _fenceBreachType = static_cast<FENCE_BREACH>(fence.breach_type);
    refresh();
}

void NexusHealthModel::refresh()
{
    if (!_vehicle) {
        _overallState = QStringLiteral("DEGRADED");
        _overallDetail = QStringLiteral("No active vehicle");

        _gpsState = _ekfState = _imuState = _compassState =
        _gyroState = _accelerometerState = _barometerState =
        _batteryState = _datalinkState = _homeState = _geofenceState =
            QStringLiteral("UNKNOWN");

        _gpsDetail = _ekfDetail = _imuDetail = _compassDetail =
        _gyroDetail = _accelerometerDetail = _barometerDetail =
        _batteryDetail = _datalinkDetail = _homeDetail = _geofenceDetail =
            QStringLiteral("No vehicle");

        _sensorFailures.clear();
        emit healthChanged();
        return;
    }

    const bool linkLost = _vehicle->vehicleLinkManager()->communicationLost();
    const bool aircraftActive = _vehicle->armed() || _vehicle->flying();

    // Datalink
    if (linkLost) {
        _datalinkState = QStringLiteral("CRITICAL");
        _datalinkDetail = QStringLiteral("Vehicle communication lost");
    } else {
        _datalinkState = QStringLiteral("NOMINAL");
        const QString linkName = _vehicle->vehicleLinkManager()->primaryLinkName();
        _datalinkDetail = linkName.isEmpty() ? QStringLiteral("Connected") : linkName;
    }

    // GPS
    auto *gps = qobject_cast<VehicleGPSFactGroup*>(_vehicle->gpsFactGroup());
    if (!gps || linkLost) {
        _gpsState = QStringLiteral("UNKNOWN");
        _gpsDetail = linkLost ? QStringLiteral("Live GPS hidden while link is lost")
                              : QStringLiteral("GPS telemetry unavailable");
    } else {
        const int lock = gps->lock()->rawValue().toInt();
        const int sats = gps->count()->rawValue().toInt();
        const double hdop = gps->hdop()->rawValue().toDouble();
        const StateDetail gpsSensor = _sensorState(MAV_SYS_STATUS_SENSOR_GPS);

        if (gpsSensor.state == QStringLiteral("CRITICAL")) {
            _gpsState = QStringLiteral("CRITICAL");
        } else if (lock >= 3) {
            _gpsState = QStringLiteral("NOMINAL");
        } else {
            _gpsState = aircraftActive && _vehicle->requiresGpsFix()
                      ? QStringLiteral("CRITICAL")
                      : QStringLiteral("DEGRADED");
        }

        _gpsDetail = QStringLiteral("Fix %1 · %2 sat").arg(lock).arg(sats);
        if (qIsFinite(hdop)) {
            _gpsDetail += QStringLiteral(" · HDOP %1").arg(hdop, 0, 'f', 1);
        }
    }

    // EKF / estimator
    auto *estimator = qobject_cast<VehicleEstimatorStatusFactGroup*>(_vehicle->estimatorStatusFactGroup());
    if (!estimator || linkLost) {
        _ekfState = QStringLiteral("UNKNOWN");
        _ekfDetail = linkLost ? QStringLiteral("Live estimator state hidden while link is lost")
                              : QStringLiteral("Estimator status unavailable");
    } else {
        const bool attitude = estimator->goodAttitudeEstimate()->rawValue().toBool();
        const bool hVel = estimator->goodHorizVelEstimate()->rawValue().toBool();
        const bool vVel = estimator->goodVertVelEstimate()->rawValue().toBool();
        const bool hPos = estimator->goodHorizPosAbsEstimate()->rawValue().toBool() ||
                          estimator->goodHorizPosRelEstimate()->rawValue().toBool();
        const bool vPos = estimator->goodVertPosAbsEstimate()->rawValue().toBool();
        const bool glitch = estimator->gpsGlitch()->rawValue().toBool();
        const bool accelError = estimator->accelError()->rawValue().toBool();

        const bool coreGood = attitude && hVel && vVel && hPos && vPos && !glitch && !accelError;
        _ekfState = coreGood ? QStringLiteral("NOMINAL")
                             : (aircraftActive ? QStringLiteral("CRITICAL")
                                               : QStringLiteral("DEGRADED"));

        _ekfDetail = QStringLiteral("ATT %1 · HVEL %2 · VVEL %3 · HPOS %4 · VPOS %5")
                         .arg(attitude ? "OK" : "BAD")
                         .arg(hVel ? "OK" : "BAD")
                         .arg(vVel ? "OK" : "BAD")
                         .arg(hPos ? "OK" : "BAD")
                         .arg(vPos ? "OK" : "BAD");
        if (glitch) _ekfDetail += QStringLiteral(" · GPS GLITCH");
        if (accelError) _ekfDetail += QStringLiteral(" · ACCEL ERROR");
    }

    // SYS_STATUS sensor domains
    const StateDetail gyro = _sensorState(MAV_SYS_STATUS_SENSOR_3D_GYRO);
    const StateDetail accel = _sensorState(MAV_SYS_STATUS_SENSOR_3D_ACCEL);
    const StateDetail mag = _sensorState(MAV_SYS_STATUS_SENSOR_3D_MAG);
    const StateDetail baro = _sensorState(MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE);

    _gyroState = gyro.state;
    _gyroDetail = gyro.detail;
    _accelerometerState = accel.state;
    _accelerometerDetail = accel.detail;
    _compassState = mag.state;
    _compassDetail = mag.detail;
    _barometerState = baro.state;
    _barometerDetail = baro.detail;

    _imuState = _worstState({_gyroState, _accelerometerState});
    if (_imuState == QStringLiteral("UNKNOWN") &&
        (_gyroState == QStringLiteral("NOMINAL") || _accelerometerState == QStringLiteral("NOMINAL"))) {
        _imuState = QStringLiteral("DEGRADED");
    }
    _imuDetail = QStringLiteral("Gyro %1 · Accel %2").arg(_gyroState, _accelerometerState);

    // Battery
    if (!_vehicle->batteries() || _vehicle->batteries()->count() == 0 || linkLost) {
        _batteryState = QStringLiteral("UNKNOWN");
        _batteryDetail = linkLost ? QStringLiteral("Live battery hidden while link is lost")
                                  : QStringLiteral("Battery telemetry unavailable");
    } else {
        auto *battery = qobject_cast<BatteryFactGroup*>(_vehicle->batteries()->get(0));
        if (!battery) {
            _batteryState = QStringLiteral("UNKNOWN");
            _batteryDetail = QStringLiteral("Battery telemetry unavailable");
        } else {
            const double pct = battery->percentRemaining()->rawValue().toDouble();
            const int chargeState = battery->chargeState()->rawValue().toInt();

            if (chargeState == MAV_BATTERY_CHARGE_STATE_CRITICAL ||
                chargeState == MAV_BATTERY_CHARGE_STATE_EMERGENCY ||
                chargeState == MAV_BATTERY_CHARGE_STATE_FAILED ||
                chargeState == MAV_BATTERY_CHARGE_STATE_UNHEALTHY ||
                (qIsFinite(pct) && pct < 15.0)) {
                _batteryState = QStringLiteral("CRITICAL");
            } else if (chargeState == MAV_BATTERY_CHARGE_STATE_LOW ||
                       (qIsFinite(pct) && pct < 25.0)) {
                _batteryState = QStringLiteral("DEGRADED");
            } else if (qIsFinite(pct)) {
                _batteryState = QStringLiteral("NOMINAL");
            } else {
                _batteryState = QStringLiteral("UNKNOWN");
            }

            const double voltage = battery->voltage()->rawValue().toDouble();
            _batteryDetail = qIsFinite(pct)
                           ? QStringLiteral("%1%").arg(qRound(pct))
                           : QStringLiteral("Percent unknown");
            if (qIsFinite(voltage)) {
                _batteryDetail += QStringLiteral(" · %1 V").arg(voltage, 0, 'f', 1);
            }
        }
    }

    // Home
    if (linkLost) {
        _homeState = QStringLiteral("UNKNOWN");
        _homeDetail = QStringLiteral("Live Home validity hidden while link is lost");
    } else if (_vehicle->homePosition().isValid()) {
        _homeState = QStringLiteral("NOMINAL");
        _homeDetail = QStringLiteral("Home position valid");
    } else {
        _homeState = aircraftActive ? QStringLiteral("CRITICAL")
                                    : QStringLiteral("DEGRADED");
        _homeDetail = QStringLiteral("Home position invalid");
    }

    // Geofence: explicit FENCE_STATUS wins, then SYS_STATUS GeoFence health.
    const StateDetail fenceSensor = _sensorState(MAV_SYS_STATUS_GEOFENCE);
    if (linkLost) {
        _geofenceState = QStringLiteral("UNKNOWN");
        _geofenceDetail = QStringLiteral("Fence status hidden while link is lost");
    } else if (_fenceStatusSeen && _fenceBreached) {
        _geofenceState = QStringLiteral("CRITICAL");
        switch (_fenceBreachType) {
        case FENCE_BREACH_MINALT: _geofenceDetail = QStringLiteral("Minimum-altitude fence breached"); break;
        case FENCE_BREACH_MAXALT: _geofenceDetail = QStringLiteral("Maximum-altitude fence breached"); break;
        case FENCE_BREACH_BOUNDARY: _geofenceDetail = QStringLiteral("Boundary fence breached"); break;
        default: _geofenceDetail = QStringLiteral("Geofence breached"); break;
        }
    } else if (fenceSensor.state == QStringLiteral("CRITICAL")) {
        _geofenceState = QStringLiteral("CRITICAL");
        _geofenceDetail = fenceSensor.detail;
    } else if (_fenceStatusSeen || fenceSensor.state == QStringLiteral("NOMINAL")) {
        _geofenceState = QStringLiteral("NOMINAL");
        _geofenceDetail = QStringLiteral("No fence breach reported");
    } else if (fenceSensor.state == QStringLiteral("DEGRADED")) {
        _geofenceState = QStringLiteral("DEGRADED");
        _geofenceDetail = fenceSensor.detail;
    } else {
        _geofenceState = QStringLiteral("UNKNOWN");
        _geofenceDetail = QStringLiteral("Geofence status not reported");
    }

    // Sensor failure list from authoritative SYS_STATUS.
    _sensorFailures.clear();
    if (!linkLost) {
        auto *info = qobject_cast<SysStatusSensorInfo*>(_vehicle->sysStatusSensorInfo());
        if (info) {
            const QStringList names = info->sensorNames();
            const QStringList status = info->sensorStatus();
            for (int i = 0; i < names.size() && i < status.size(); ++i) {
                if (status.at(i) == QStringLiteral("Error")) {
                    _sensorFailures.append(names.at(i));
                }
            }
        }
    }

    // Consolidated operator state. UNKNOWN optional domains never masquerade as
    // failures; CRITICAL/DEGRADED only come from an authoritative condition.
    QStringList states = {
        _gpsState,
        _ekfState,
        _imuState,
        _compassState,
        _gyroState,
        _accelerometerState,
        _barometerState,
        _batteryState,
        _datalinkState,
        _homeState,
        _geofenceState,
    };

    _overallState = _worstState(states);
    if (_overallState == QStringLiteral("UNKNOWN")) {
        _overallState = QStringLiteral("DEGRADED");
    }

    if (!_sensorFailures.isEmpty()) {
        _overallState = QStringLiteral("CRITICAL");
        _overallDetail = QStringLiteral("%1 sensor failure(s): %2")
                             .arg(_sensorFailures.size())
                             .arg(_sensorFailures.join(QStringLiteral(", ")));
    } else if (_overallState == QStringLiteral("CRITICAL")) {
        _overallDetail = QStringLiteral("One or more flight-critical systems require immediate attention");
    } else if (_overallState == QStringLiteral("DEGRADED")) {
        _overallDetail = QStringLiteral("Flight system available with degraded or incomplete health");
    } else {
        _overallDetail = QStringLiteral("All reported flight-critical systems nominal");
    }

    emit healthChanged();
}
