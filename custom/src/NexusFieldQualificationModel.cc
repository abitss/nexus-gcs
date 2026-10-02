#include "NexusFieldQualificationModel.h"

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QDateTime>
#include <QtCore/QtMath>

#include "MultiVehicleManager.h"
#include "NexusDeviceHealthModel.h"
#include "NexusRecoveryModel.h"
#include "Vehicle.h"
#include "VehicleGPSFactGroup.h"
#include "MissionManager.h"

NexusFieldQualificationModel::NexusFieldQualificationModel(NexusDeviceHealthModel *device, NexusRecoveryModel *recovery, QObject *parent)
    : QObject(parent), _device(device), _recovery(recovery)
{
    connect(MultiVehicleManager::instance(), &MultiVehicleManager::activeVehicleChanged,
            this, &NexusFieldQualificationModel::_activeVehicleChanged);
    _setVehicle(MultiVehicleManager::instance()->activeVehicle());

    _timer.setInterval(1000);
    connect(&_timer, &QTimer::timeout, this, &NexusFieldQualificationModel::_sample);
    _timer.start();
    _loop.start();
}

int NexusFieldQualificationModel::durationSeconds() const
{
    return _running && _session.isValid() ? static_cast<int>(_session.elapsed()/1000) : 0;
}

bool NexusFieldQualificationModel::startSession(const QString &phase)
{
    const QString p=phase.trimmed().toUpper();
    if (p!="HIL" && p!="FIELD") return false;
    _running=true; _phase=p; _session.restart(); _loop.restart();
    _heartbeatCount=0; _heartbeatRateHz=0; _maxEventLoopLagMs=0; _cards.clear();
    _maxDeviceTempC=qQNaN(); _minRamAvailableMb=-1; _recoveryEventCount=0;
    emit changed();
    return true;
}

void NexusFieldQualificationModel::stopSession()
{
    _running=false;
    emit changed();
}

void NexusFieldQualificationModel::markCard(const QString &name, const QString &status, const QString &notes)
{
    QVariantMap row{
        {QStringLiteral("time"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("name"), name.trimmed()},
        {QStringLiteral("status"), status.trimmed().toUpper()},
        {QStringLiteral("notes"), notes.trimmed()}
    };
    _cards.append(row);
    emit changed();
}

QString NexusFieldQualificationModel::exportJson(const QString &path) const
{
    QJsonObject root{
        {QStringLiteral("schemaVersion"), QStringLiteral("1.0")},
        {QStringLiteral("phase"), _phase},
        {QStringLiteral("durationSeconds"), durationSeconds()},
        {QStringLiteral("heartbeatRateHz"), _heartbeatRateHz},
        {QStringLiteral("mavlinkLossPercent"), _mavlinkLossPercent},
        {QStringLiteral("gpsFix"), _gpsFix},
        {QStringLiteral("satellites"), _satellites},
        {QStringLiteral("hdop"), qIsNaN(_hdop) ? QJsonValue() : QJsonValue(_hdop)},
        {QStringLiteral("vdop"), qIsNaN(_vdop) ? QJsonValue() : QJsonValue(_vdop)},
        {QStringLiteral("missionIndex"), _missionIndex},
        {QStringLiteral("flightMode"), _flightMode},
        {QStringLiteral("maxDeviceTempC"), qIsNaN(_maxDeviceTempC) ? QJsonValue() : QJsonValue(_maxDeviceTempC)},
        {QStringLiteral("minRamAvailableMb"), _minRamAvailableMb},
        {QStringLiteral("maxEventLoopLagMs"), _maxEventLoopLagMs},
        {QStringLiteral("recoveryEventCount"), _recoveryEventCount},
        {QStringLiteral("cards"), QJsonArray::fromVariantList(_cards)}
    };
    QFile f(path);
    if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate)) return QString();
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return path;
}

QString NexusFieldQualificationModel::exportDefault() const
{
    const QString dir=QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
        .filePath(QStringLiteral("validation"));
    QDir().mkpath(dir);
    const QString stamp=QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    return exportJson(QDir(dir).filePath(QStringLiteral("nexus-%1-%2.json").arg(_phase.toLower(),stamp)));
}

void NexusFieldQualificationModel::_activeVehicleChanged(Vehicle *vehicle){ _setVehicle(vehicle); }

void NexusFieldQualificationModel::_setVehicle(Vehicle *vehicle)
{
    if(_vehicle) disconnect(_vehicle,nullptr,this,nullptr);
    _vehicle=vehicle;
    if(_vehicle) connect(_vehicle,&Vehicle::mavlinkMessageReceived,this,&NexusFieldQualificationModel::_message);
}

void NexusFieldQualificationModel::_message(const mavlink_message_t &message)
{
    if(!_running || !_vehicle || message.sysid!=_vehicle->id()) return;
    if(message.msgid==MAVLINK_MSG_ID_HEARTBEAT) ++_heartbeatCount;
}

void NexusFieldQualificationModel::_sample()
{
    const qint64 elapsed=_loop.restart();
    const double lag=qMax<qint64>(0,elapsed-1000);
    _maxEventLoopLagMs=qMax(_maxEventLoopLagMs,lag);

    if(_running && _session.elapsed()>0) _heartbeatRateHz=_heartbeatCount/(_session.elapsed()/1000.0);

    if(_vehicle){
        _mavlinkLossPercent=_vehicle->mavlinkLossPercent();
        _flightMode=_vehicle->flightMode();
        if(_vehicle->missionManager()) _missionIndex=_vehicle->missionManager()->currentIndex();
        if(auto *gps=qobject_cast<VehicleGPSFactGroup*>(_vehicle->gpsFactGroup())){
            _gpsFix=gps->lock()->rawValue().toInt();
            _satellites=gps->count()->rawValue().toInt();
            _hdop=gps->hdop()->rawValue().toDouble();
            _vdop=gps->vdop()->rawValue().toDouble();
        }
    }

    if(_device){
        const double t=_device->temperatureC();
        if(qIsFinite(t)) _maxDeviceTempC=qIsNaN(_maxDeviceTempC)?t:qMax(_maxDeviceTempC,t);
        const int ram=_device->ramAvailableMb();
        if(ram>=0) _minRamAvailableMb=_minRamAvailableMb<0?ram:qMin(_minRamAvailableMb,ram);
    }
    if(_recovery) _recoveryEventCount=_recovery->events().size();

    emit changed();
}
