#include "NexusAlertManager.h"

#include "MissionManager.h"
#include "MultiVehicleManager.h"
#include "NexusHealthModel.h"
#include "NexusDeviceHealthModel.h"
#include "NexusRecoveryModel.h"
#include "QGCMAVLink.h"
#include "SettingsManager.h"
#include "VideoManager.h"
#include "VideoSettings.h"
#include "Vehicle.h"
#include "VehicleLinkManager.h"

NexusAlertManager::NexusAlertManager(NexusHealthModel *health, NexusDeviceHealthModel *deviceHealth, QObject *parent)
    : QAbstractListModel(parent)
    , _health(health)
    , _deviceHealth(deviceHealth)
{
    auto *manager = MultiVehicleManager::instance();
    connect(manager, &MultiVehicleManager::activeVehicleChanged,
            this, &NexusAlertManager::_activeVehicleChanged);

    _timer.setInterval(500);
    _timer.setTimerType(Qt::CoarseTimer);
    connect(&_timer, &QTimer::timeout, this, &NexusAlertManager::_evaluateConditions);
    _timer.start();

    _setVehicle(manager->activeVehicle());
    _evaluateConditions();
}

void NexusAlertManager::setRecoveryModel(NexusRecoveryModel *recovery)
{
    if (_recovery == recovery) return;
    if (_recovery) disconnect(_recovery, nullptr, this, nullptr);
    _recovery = recovery;
    if (_recovery) {
        connect(_recovery, &NexusRecoveryModel::recoveryChanged,
                this, &NexusAlertManager::_evaluateConditions);
    }
    _evaluateConditions();
}

int NexusAlertManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : _alerts.size();
}

QVariant NexusAlertManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= _alerts.size()) {
        return {};
    }

    const Alert &a = _alerts.at(index.row());
    switch (role) {
    case IdRole: return a.id;
    case TimestampRole: return a.timestamp.toLocalTime();
    case SeverityRole: return a.severity;
    case SourceRole: return a.source;
    case TitleRole: return a.title;
    case MessageRole: return a.message;
    case ActiveRole: return a.active;
    case AcknowledgedRole: return a.acknowledged;
    default: return {};
    }
}

QHash<int, QByteArray> NexusAlertManager::roleNames() const
{
    return {
        {IdRole, "alertId"},
        {TimestampRole, "timestamp"},
        {SeverityRole, "severity"},
        {SourceRole, "source"},
        {TitleRole, "title"},
        {MessageRole, "message"},
        {ActiveRole, "active"},
        {AcknowledgedRole, "acknowledged"},
    };
}

int NexusAlertManager::_severityRank(const QString &severity)
{
    if (severity == QStringLiteral("CRITICAL")) return 3;
    if (severity == QStringLiteral("WARNING")) return 2;
    return 1;
}

int NexusAlertManager::activeCount() const
{
    int count = 0;
    for (const Alert &a : _alerts) {
        if (a.active) ++count;
    }
    return count;
}

int NexusAlertManager::unacknowledgedCount() const
{
    int count = 0;
    for (const Alert &a : _alerts) {
        if (a.active && !a.acknowledged) ++count;
    }
    return count;
}

QString NexusAlertManager::highestSeverity() const
{
    QString result = QStringLiteral("INFO");
    int best = 0;
    for (const Alert &a : _alerts) {
        if (!a.active || a.acknowledged) continue;
        const int rank = _severityRank(a.severity);
        if (rank > best) {
            best = rank;
            result = a.severity;
        }
    }
    return best == 0 ? QStringLiteral("NONE") : result;
}

QString NexusAlertManager::currentTitle() const
{
    int best = 0;
    for (const Alert &a : _alerts) {
        if (!a.active || a.acknowledged) continue;
        const int rank = _severityRank(a.severity);
        if (rank > best) {
            best = rank;
            if (rank == 3) return a.title;
        }
    }
    for (const Alert &a : _alerts) {
        if (a.active && !a.acknowledged && _severityRank(a.severity) == best) return a.title;
    }
    return {};
}

QString NexusAlertManager::currentMessage() const
{
    int best = 0;
    for (const Alert &a : _alerts) {
        if (!a.active || a.acknowledged) continue;
        best = qMax(best, _severityRank(a.severity));
    }
    for (const Alert &a : _alerts) {
        if (a.active && !a.acknowledged && _severityRank(a.severity) == best) return a.message;
    }
    return {};
}

void NexusAlertManager::acknowledge(int row)
{
    if (row < 0 || row >= _alerts.size()) return;
    Alert &a = _alerts[row];
    if (a.acknowledged) return;
    a.acknowledged = true;
    emit dataChanged(index(row), index(row), {AcknowledgedRole});
    _refreshSummary();
}

void NexusAlertManager::acknowledgeById(const QString &id)
{
    for (int row = 0; row < _alerts.size(); ++row) {
        if (_alerts.at(row).id == id && _alerts.at(row).active) {
            acknowledge(row);
            return;
        }
    }
}

void NexusAlertManager::acknowledgeAll()
{
    if (_alerts.isEmpty()) return;
    bool changed = false;
    for (Alert &a : _alerts) {
        if (a.active && !a.acknowledged) {
            a.acknowledged = true;
            changed = true;
        }
    }
    if (changed) {
        emit dataChanged(index(0), index(_alerts.size() - 1), {AcknowledgedRole});
        _refreshSummary();
    }
}

void NexusAlertManager::clearInactiveHistory()
{
    beginResetModel();
    QList<Alert> kept;
    for (const Alert &a : _alerts) {
        if (a.active) kept.append(a);
    }
    _alerts = kept;
    _activeRows.clear();
    for (int i = 0; i < _alerts.size(); ++i) {
        if (_alerts.at(i).active) _activeRows.insert(_alerts.at(i).id, i);
    }
    endResetModel();
    _refreshSummary();
}

void NexusAlertManager::_setVehicle(Vehicle *vehicle)
{
    if (_vehicle) {
        disconnect(_vehicle, nullptr, this, nullptr);
        if (_vehicle->missionManager()) disconnect(_vehicle->missionManager(), nullptr, this, nullptr);
    }

    _vehicle = vehicle;
    _failsafeActive = false;
    _failsafeReason.clear();
    _systemCriticalActive = false;
    _systemCriticalReason.clear();

    if (_vehicle) {
        connect(_vehicle, &Vehicle::mavlinkMessageReceived,
                this, &NexusAlertManager::_mavlinkMessageReceived);
        connect(_vehicle, &Vehicle::mavCommandResult,
                this, &NexusAlertManager::_commandResult);
        connect(_vehicle, &Vehicle::textMessageReceived,
                this, &NexusAlertManager::_textMessage);
        if (_vehicle->missionManager()) {
            connect(_vehicle->missionManager(), &MissionManager::error,
                    this, &NexusAlertManager::_missionError);
        }
    }

    _evaluateConditions();
}

void NexusAlertManager::_activeVehicleChanged(Vehicle *vehicle)
{
    _setVehicle(vehicle);
}

void NexusAlertManager::_setCondition(const QString &id,
                                      bool active,
                                      const QString &severity,
                                      const QString &source,
                                      const QString &title,
                                      const QString &message)
{
    const auto it = _activeRows.constFind(id);

    if (active) {
        if (it != _activeRows.constEnd()) {
            const int row = it.value();
            Alert &a = _alerts[row];
            bool changed = false;
            if (a.severity != severity) { a.severity = severity; changed = true; }
            if (a.message != message) { a.message = message; changed = true; }
            if (a.title != title) { a.title = title; changed = true; }
            if (changed) emit dataChanged(index(row), index(row));
            return;
        }

        const int row = _alerts.size();
        beginInsertRows(QModelIndex(), row, row);
        _alerts.append({id, QDateTime::currentDateTimeUtc(), severity, source, title, message, true, false});
        _activeRows.insert(id, row);
        endInsertRows();
        _trimHistory();
        _refreshSummary();
        return;
    }

    if (it != _activeRows.constEnd()) {
        const int row = it.value();
        Alert &a = _alerts[row];
        a.active = false;
        _activeRows.remove(id);
        emit dataChanged(index(row), index(row), {ActiveRole});
        _refreshSummary();
    }
}

void NexusAlertManager::_postEvent(const QString &id,
                                   const QString &severity,
                                   const QString &source,
                                   const QString &title,
                                   const QString &message)
{
    const int row = _alerts.size();
    beginInsertRows(QModelIndex(), row, row);
    _alerts.append({id + QStringLiteral(":%1").arg(QDateTime::currentMSecsSinceEpoch()),
                    QDateTime::currentDateTimeUtc(), severity, source, title, message, false, false});
    endInsertRows();
    _trimHistory();
    _refreshSummary();
}

void NexusAlertManager::_trimHistory()
{
    constexpr int kMaxHistory = 500;
    if (_alerts.size() <= kMaxHistory) return;

    // Never trim active conditions.
    for (int row = 0; row < _alerts.size() && _alerts.size() > kMaxHistory;) {
        if (!_alerts.at(row).active) {
            beginRemoveRows(QModelIndex(), row, row);
            _alerts.removeAt(row);
            endRemoveRows();
            for (auto it = _activeRows.begin(); it != _activeRows.end(); ++it) {
                if (it.value() > row) it.value() -= 1;
            }
        } else {
            ++row;
        }
    }
}

void NexusAlertManager::_refreshSummary()
{
    emit summaryChanged();
}

void NexusAlertManager::_evaluateConditions()
{
    if (_recovery) {
        const QString state = _recovery->overallState();
        _setCondition(QStringLiteral("recovery-state"),
                      state != QStringLiteral("NOMINAL"),
                      state == QStringLiteral("RECOVERY REQUIRED") ? QStringLiteral("CRITICAL") : QStringLiteral("WARNING"),
                      QStringLiteral("RECOVERY"),
                      state == QStringLiteral("RECOVERY REQUIRED")
                          ? QStringLiteral("Recovery Action Required")
                          : QStringLiteral("Recovery Degraded"),
                      _recovery->lastRecoveryEvent());
    } else {
        _setCondition(QStringLiteral("recovery-state"), false, {}, {}, {}, {});
    }
    if (_deviceHealth) {
        _setCondition(QStringLiteral("device-overheat"),
                      _deviceHealth->overheatingWarning(),
                      QStringLiteral("CRITICAL"),
                      QStringLiteral("DEVICE"),
                      QStringLiteral("Ground Station Overheating"),
                      QStringLiteral("Tablet thermal state: %1").arg(_deviceHealth->thermalState()));

        _setCondition(QStringLiteral("device-storage"),
                      _deviceHealth->lowStorageWarning(),
                      QStringLiteral("WARNING"),
                      QStringLiteral("DEVICE"),
                      QStringLiteral("Ground Station Storage Low"),
                      QStringLiteral("Free local storage: %1").arg(_deviceHealth->storageFreeText()));

        _setCondition(QStringLiteral("device-battery"),
                      _deviceHealth->lowBatteryWarning(),
                      QStringLiteral("WARNING"),
                      QStringLiteral("DEVICE"),
                      QStringLiteral("Ground Station Battery Low"),
                      _deviceHealth->batteryPercent() >= 0
                        ? QStringLiteral("Tablet battery: %1% · %2").arg(_deviceHealth->batteryPercent()).arg(_deviceHealth->batteryState())
                        : QStringLiteral("Tablet battery state is degraded"));
    } else {
        _setCondition(QStringLiteral("device-overheat"), false, {}, {}, {}, {});
        _setCondition(QStringLiteral("device-storage"), false, {}, {}, {}, {});
        _setCondition(QStringLiteral("device-battery"), false, {}, {}, {}, {});
    }
    if (!_vehicle || !_health) {
        _setCondition(QStringLiteral("telemetry"), false, {}, {}, {}, {});
        _setCondition(QStringLiteral("battery"), false, {}, {}, {}, {});
        _setCondition(QStringLiteral("gps"), false, {}, {}, {}, {});
        _setCondition(QStringLiteral("nav"), false, {}, {}, {}, {});
        _setCondition(QStringLiteral("home"), false, {}, {}, {}, {});
        _setCondition(QStringLiteral("geofence"), false, {}, {}, {}, {});
        _setCondition(QStringLiteral("failsafe"), false, {}, {}, {}, {});
        _setCondition(QStringLiteral("video-loss"), false, {}, {}, {}, {});
        _setCondition(QStringLiteral("autopilot-state"), false, {}, {}, {}, {});
        return;
    }

    const QString datalink = _health->datalinkState();
    _setCondition(QStringLiteral("telemetry"),
                  datalink == QStringLiteral("CRITICAL"),
                  QStringLiteral("CRITICAL"),
                  QStringLiteral("DATALINK"),
                  QStringLiteral("Telemetry Lost"),
                  _health->datalinkDetail());

    const QString battery = _health->batteryState();
    _setCondition(QStringLiteral("battery"),
                  battery == QStringLiteral("CRITICAL") || battery == QStringLiteral("DEGRADED"),
                  battery == QStringLiteral("CRITICAL") ? QStringLiteral("CRITICAL") : QStringLiteral("WARNING"),
                  QStringLiteral("BATTERY"),
                  battery == QStringLiteral("CRITICAL") ? QStringLiteral("Battery Critical") : QStringLiteral("Battery Warning"),
                  _health->batteryDetail());

    const QString gps = _health->gpsState();
    _setCondition(QStringLiteral("gps"),
                  gps == QStringLiteral("CRITICAL") || gps == QStringLiteral("DEGRADED"),
                  gps == QStringLiteral("CRITICAL") ? QStringLiteral("CRITICAL") : QStringLiteral("WARNING"),
                  QStringLiteral("NAVIGATION"),
                  gps == QStringLiteral("CRITICAL") ? QStringLiteral("GPS Failure") : QStringLiteral("GPS Degraded"),
                  _health->gpsDetail());

    const QString ekf = _health->ekfState();
    _setCondition(QStringLiteral("nav"),
                  ekf == QStringLiteral("CRITICAL") || ekf == QStringLiteral("DEGRADED"),
                  ekf == QStringLiteral("CRITICAL") ? QStringLiteral("CRITICAL") : QStringLiteral("WARNING"),
                  QStringLiteral("ESTIMATOR"),
                  ekf == QStringLiteral("CRITICAL") ? QStringLiteral("Navigation Failure") : QStringLiteral("Navigation Degraded"),
                  _health->ekfDetail());

    const QString home = _health->homeState();
    _setCondition(QStringLiteral("home"),
                  home == QStringLiteral("CRITICAL") || home == QStringLiteral("DEGRADED"),
                  home == QStringLiteral("CRITICAL") ? QStringLiteral("CRITICAL") : QStringLiteral("WARNING"),
                  QStringLiteral("HOME"),
                  QStringLiteral("Home Invalid"),
                  _health->homeDetail());

    const QString fence = _health->geofenceState();
    _setCondition(QStringLiteral("geofence"),
                  fence == QStringLiteral("CRITICAL") || fence == QStringLiteral("DEGRADED"),
                  fence == QStringLiteral("CRITICAL") ? QStringLiteral("CRITICAL") : QStringLiteral("WARNING"),
                  QStringLiteral("GEOFENCE"),
                  fence == QStringLiteral("CRITICAL") ? QStringLiteral("Geofence Breach / Failure") : QStringLiteral("Geofence Warning"),
                  _health->geofenceDetail());

    auto *videoSettings = SettingsManager::instance()->videoSettings();
    auto *videoManager = VideoManager::instance();
    const bool videoConfigured = videoSettings && videoSettings->streamConfigured();
    const bool videoMissing = videoConfigured && (!videoManager || !videoManager->decoding());
    _setCondition(QStringLiteral("video-loss"),
                  videoMissing,
                  QStringLiteral("WARNING"),
                  QStringLiteral("PAYLOAD"),
                  QStringLiteral("Video Stream Unavailable"),
                  QStringLiteral("EO/FPV source is configured but no decoded video is available"));

    _setCondition(QStringLiteral("failsafe"),
                  _failsafeActive,
                  QStringLiteral("CRITICAL"),
                  QStringLiteral("AUTOPILOT"),
                  QStringLiteral("Failsafe Active"),
                  _failsafeReason.isEmpty() ? QStringLiteral("Autopilot explicitly reported an active failsafe")
                                            : _failsafeReason);

    _setCondition(QStringLiteral("autopilot-state"),
                  _systemCriticalActive,
                  QStringLiteral("CRITICAL"),
                  QStringLiteral("AUTOPILOT"),
                  QStringLiteral("Autopilot Critical State"),
                  _systemCriticalReason);
}

void NexusAlertManager::_mavlinkMessageReceived(const mavlink_message_t &message)
{
    if (message.msgid != MAVLINK_MSG_ID_HEARTBEAT) return;

    mavlink_heartbeat_t heartbeat{};
    mavlink_msg_heartbeat_decode(&message, &heartbeat);

    _systemCriticalActive = heartbeat.system_status == MAV_STATE_CRITICAL ||
                            heartbeat.system_status == MAV_STATE_EMERGENCY;
    if (_systemCriticalActive) {
        _systemCriticalReason = heartbeat.system_status == MAV_STATE_EMERGENCY
                              ? QStringLiteral("Autopilot system state: EMERGENCY")
                              : QStringLiteral("Autopilot system state: CRITICAL");
    } else {
        _systemCriticalReason.clear();
    }
    _evaluateConditions();
}

void NexusAlertManager::_commandResult(int vehicleId, int targetComponent, int command, int ackResult, int failureCode)
{
    Q_UNUSED(targetComponent)
    if (!_vehicle || vehicleId != _vehicle->id()) return;

    if (ackResult == MAV_RESULT_ACCEPTED && failureCode == Vehicle::MavCmdResultCommandResultOnly) {
        _postEvent(QStringLiteral("command-accepted"),
                   QStringLiteral("INFO"),
                   QStringLiteral("COMMAND"),
                   QStringLiteral("Command Accepted"),
                   QStringLiteral("MAV_CMD %1 acknowledged by vehicle").arg(command));
        return;
    }

    _postEvent(QStringLiteral("command-rejected"),
               QStringLiteral("CRITICAL"),
               QStringLiteral("COMMAND"),
               QStringLiteral("Command Rejected"),
               QStringLiteral("MAV_CMD %1 result %2 · failure %3")
                   .arg(command)
                   .arg(QGCMAVLink::mavResultToString(static_cast<uint8_t>(ackResult)))
                   .arg(failureCode));
}

void NexusAlertManager::_textMessage(int sysid, int componentid, int severity, const QString &text, const QString &description)
{
    Q_UNUSED(sysid)
    Q_UNUSED(componentid)

    const QString combined = description.isEmpty() ? text : QStringLiteral("%1 · %2").arg(text, description);
    const QString lower = combined.toLower();

    if (lower.contains(QStringLiteral("failsafe")) ||
        lower.contains(QStringLiteral("fail-safe")) ||
        lower.contains(QStringLiteral("emergency"))) {
        const bool cleared = lower.contains(QStringLiteral("clear")) ||
                             lower.contains(QStringLiteral("recover")) ||
                             lower.contains(QStringLiteral("resolved"));
        _failsafeActive = !cleared;
        _failsafeReason = cleared ? QString() : combined;
        _evaluateConditions();
    }

    QString level;
    if (severity <= MAV_SEVERITY_ERROR) level = QStringLiteral("CRITICAL");
    else if (severity <= MAV_SEVERITY_WARNING) level = QStringLiteral("WARNING");
    else level = QStringLiteral("INFO");

    if (level != QStringLiteral("INFO") || lower.contains(QStringLiteral("failsafe"))) {
        _postEvent(QStringLiteral("autopilot-message"),
                   level,
                   QStringLiteral("AUTOPILOT"),
                   QStringLiteral("Autopilot Message"),
                   combined);
    }
}

void NexusAlertManager::_missionError(int errorCode, const QString &errorMsg)
{
    _postEvent(QStringLiteral("mission-failure"),
               QStringLiteral("CRITICAL"),
               QStringLiteral("MISSION"),
               QStringLiteral("Mission Failure"),
               QStringLiteral("Code %1 · %2").arg(errorCode).arg(errorMsg));
}
