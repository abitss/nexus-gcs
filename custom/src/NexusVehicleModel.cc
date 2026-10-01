#include "NexusVehicleModel.h"

#include <QtCore/QStringList>

#include "AutoPilotPlugin.h"
#include "Fact.h"
#include "MultiVehicleManager.h"
#include "ParameterManager.h"
#include "Vehicle.h"
#include "VehicleComponent.h"
#include "VehicleLinkManager.h"

NexusVehicleModel::NexusVehicleModel(QObject *parent)
    : QObject(parent)
{
    auto *manager = MultiVehicleManager::instance();
    connect(manager, &MultiVehicleManager::activeVehicleChanged,
            this, &NexusVehicleModel::_activeVehicleChanged);

    _timer.setInterval(400);
    _timer.setTimerType(Qt::CoarseTimer);
    connect(&_timer, &QTimer::timeout, this, &NexusVehicleModel::refresh);
    _timer.start();

    _setVehicle(manager->activeVehicle());
}

bool NexusVehicleModel::linkLost() const
{
    return _vehicle && _vehicle->vehicleLinkManager()
        ? _vehicle->vehicleLinkManager()->communicationLost()
        : false;
}

bool NexusVehicleModel::armed() const
{
    return _vehicle && _vehicle->armed();
}

bool NexusVehicleModel::flying() const
{
    return _vehicle && _vehicle->flying();
}

bool NexusVehicleModel::safeToConfigure() const
{
    return _vehicle && !linkLost() && !armed() && !flying();
}

bool NexusVehicleModel::safeToReboot() const
{
    return safeToConfigure() && !_parameterManager.isNull() && !_parameterManager->pendingWrites();
}

bool NexusVehicleModel::parametersReady() const
{
    return _parameterManager && _parameterManager->parametersReady();
}

bool NexusVehicleModel::pendingWrites() const
{
    return _parameterManager && _parameterManager->pendingWrites();
}

QStringList NexusVehicleModel::rebootParameters() const
{
    QStringList list(_rebootParameters.begin(), _rebootParameters.end());
    list.sort(Qt::CaseInsensitive);
    return list;
}

QString NexusVehicleModel::vehicleIdText() const
{
    return _vehicle ? QStringLiteral("UAV-%1").arg(_vehicle->id()) : QStringLiteral("--");
}

QString NexusVehicleModel::uidText() const
{
    if (!_vehicle) return QStringLiteral("--");
    const QString uid = _vehicle->vehicleUIDStr();
    return uid.isEmpty() ? QStringLiteral("--") : uid;
}

QString NexusVehicleModel::vehicleType() const
{
    return _vehicle ? _vehicle->vehicleTypeString() : QStringLiteral("NO VEHICLE");
}

QString NexusVehicleModel::firmwareType() const
{
    return _vehicle ? _vehicle->firmwareTypeString() : QStringLiteral("--");
}

QString NexusVehicleModel::firmwareVersion() const
{
    if (!_vehicle || _vehicle->firmwareMajorVersion() < 0) {
        return QStringLiteral("UNKNOWN");
    }

    QString version = QStringLiteral("%1.%2.%3")
                          .arg(_vehicle->firmwareMajorVersion())
                          .arg(_vehicle->firmwareMinorVersion())
                          .arg(_vehicle->firmwarePatchVersion());
    const QString type = _vehicle->firmwareVersionTypeString();
    if (!type.isEmpty()) {
        version += type.startsWith(QLatin1Char(' ')) ? type : QStringLiteral(" ") + type;
    }
    return version;
}

QString NexusVehicleModel::firmwareGitHash() const
{
    if (!_vehicle) return QStringLiteral("--");
    const QString hash = _vehicle->gitHash();
    return hash.isEmpty() ? QStringLiteral("--") : hash;
}

QString NexusVehicleModel::flightMode() const
{
    if (!_vehicle || linkLost()) return QStringLiteral("--");
    return _vehicle->flightMode();
}

QString NexusVehicleModel::setupState() const
{
    if (!_vehicle || !_vehicle->autopilotPlugin()) return QStringLiteral("UNAVAILABLE");
    return _vehicle->autopilotPlugin()->setupComplete()
        ? QStringLiteral("READY")
        : QStringLiteral("SETUP REQUIRED");
}

QString NexusVehicleModel::parameterState() const
{
    if (!_vehicle || !_parameterManager) return QStringLiteral("UNAVAILABLE");
    if (linkLost()) return QStringLiteral("LINK LOST");
    if (_parameterManager->pendingWrites()) return QStringLiteral("WRITING");
    if (!_parameterManager->parametersReady()) return QStringLiteral("LOADING");
    if (_parameterManager->missingParameters()) return QStringLiteral("INCOMPLETE");
    return QStringLiteral("READY");
}

VehicleComponent *NexusVehicleModel::_knownComponent(int knownComponent) const
{
    if (!_vehicle || !_vehicle->autopilotPlugin()) return nullptr;
    return _vehicle->autopilotPlugin()->findKnownVehicleComponent(
        static_cast<AutoPilotPlugin::KnownVehicleComponent>(knownComponent));
}

VehicleComponent *NexusVehicleModel::_airframeComponent() const
{
    if (!_vehicle || !_vehicle->autopilotPlugin()) return nullptr;

    const QVariantList components = _vehicle->autopilotPlugin()->vehicleComponents();
    for (const QVariant &entry : components) {
        auto *component = entry.value<VehicleComponent *>();
        if (!component) continue;
        const QString name = component->name();
        if (name.contains(QStringLiteral("airframe"), Qt::CaseInsensitive) ||
            name.contains(QStringLiteral("frame"), Qt::CaseInsensitive)) {
            return component;
        }
    }
    return nullptr;
}

QString NexusVehicleModel::_componentState(VehicleComponent *component)
{
    if (!component) return QStringLiteral("UNAVAILABLE");
    if (!component->requiresSetup()) return QStringLiteral("AVAILABLE");
    return component->setupComplete() ? QStringLiteral("READY")
                                      : QStringLiteral("SETUP REQUIRED");
}

bool NexusVehicleModel::airframeAvailable() const { return _airframeComponent() != nullptr; }
QString NexusVehicleModel::airframeState() const { return _componentState(_airframeComponent()); }

bool NexusVehicleModel::sensorsAvailable() const
{
    return _knownComponent(AutoPilotPlugin::KnownSensorsVehicleComponent) != nullptr;
}
QString NexusVehicleModel::sensorsState() const
{
    return _componentState(_knownComponent(AutoPilotPlugin::KnownSensorsVehicleComponent));
}

bool NexusVehicleModel::powerAvailable() const
{
    return _knownComponent(AutoPilotPlugin::KnownPowerVehicleComponent) != nullptr;
}
QString NexusVehicleModel::powerState() const
{
    return _componentState(_knownComponent(AutoPilotPlugin::KnownPowerVehicleComponent));
}

bool NexusVehicleModel::radioAvailable() const
{
    return _knownComponent(AutoPilotPlugin::KnownRadioVehicleComponent) != nullptr;
}
QString NexusVehicleModel::radioState() const
{
    return _componentState(_knownComponent(AutoPilotPlugin::KnownRadioVehicleComponent));
}

bool NexusVehicleModel::flightModesAvailable() const
{
    return _knownComponent(AutoPilotPlugin::KnownFlightModesVehicleComponent) != nullptr;
}
QString NexusVehicleModel::flightModesState() const
{
    return _componentState(_knownComponent(AutoPilotPlugin::KnownFlightModesVehicleComponent));
}

bool NexusVehicleModel::safetyAvailable() const
{
    return _knownComponent(AutoPilotPlugin::KnownSafetyVehicleComponent) != nullptr;
}
QString NexusVehicleModel::safetyState() const
{
    return _componentState(_knownComponent(AutoPilotPlugin::KnownSafetyVehicleComponent));
}

void NexusVehicleModel::_activeVehicleChanged(Vehicle *vehicle)
{
    _setVehicle(vehicle);
}

void NexusVehicleModel::_setVehicle(Vehicle *vehicle)
{
    if (_vehicle) {
        disconnect(_vehicle, nullptr, this, nullptr);
    }
    if (_parameterManager) {
        disconnect(_parameterManager, nullptr, this, nullptr);
    }

    _vehicle = vehicle;
    _parameterManager = vehicle ? vehicle->parameterManager() : nullptr;
    _rebootParameters.clear();
    _observedFacts.clear();

    if (_vehicle) {
        connect(_vehicle, &QObject::destroyed, this, [this]() {
            _vehicle = nullptr;
            _parameterManager = nullptr;
            _rebootParameters.clear();
            _observedFacts.clear();
            emit vehicleChanged();
        });
    }

    if (_parameterManager) {
        connect(_parameterManager, &ParameterManager::factAdded,
                this, &NexusVehicleModel::_attachFact);
        connect(_parameterManager, &ParameterManager::pendingWritesChanged,
                this, &NexusVehicleModel::vehicleChanged);
        connect(_parameterManager, &ParameterManager::parametersReadyChanged,
                this, &NexusVehicleModel::vehicleChanged);
        connect(_parameterManager, &ParameterManager::missingParametersChanged,
                this, &NexusVehicleModel::vehicleChanged);
        connect(_parameterManager, &ParameterManager::_paramSetFailure,
                this, &NexusVehicleModel::_parameterWriteFailed);
        _attachKnownFacts();
    }

    emit vehicleChanged();
}

void NexusVehicleModel::_attachKnownFacts()
{
    if (!_parameterManager) return;

    const QList<int> componentIds = _parameterManager->componentIds();
    for (int componentId : componentIds) {
        const QStringList names = _parameterManager->parameterNames(componentId);
        for (const QString &name : names) {
            _attachFact(componentId, _parameterManager->getParameter(componentId, name));
        }
    }
}

void NexusVehicleModel::_attachFact(int componentId, Fact *fact)
{
    if (!fact || _observedFacts.contains(fact)) return;
    _observedFacts.insert(fact);

    connect(fact, &Fact::containerRawValueChanged, this,
            [this, componentId, fact](const QVariant &) {
        if (!fact->vehicleRebootRequired()) return;
        const QString key = QStringLiteral("%1:%2").arg(componentId).arg(fact->name());
        if (!_rebootParameters.contains(key)) {
            _rebootParameters.insert(key);
            emit vehicleChanged();
        }
    });
}

void NexusVehicleModel::_parameterWriteFailed(int componentId, const QString &paramName)
{
    const QString key = QStringLiteral("%1:%2").arg(componentId).arg(paramName);
    if (_rebootParameters.remove(key) > 0) {
        emit vehicleChanged();
    }
}

void NexusVehicleModel::refresh()
{
    emit vehicleChanged();
}

bool NexusVehicleModel::rebootVehicle()
{
    if (!safeToReboot() || !_vehicle) {
        return false;
    }

    _vehicle->rebootVehicle();
    _rebootParameters.clear();
    emit vehicleChanged();
    return true;
}

void NexusVehicleModel::clearRebootRequiredForTest()
{
    _rebootParameters.clear();
    emit vehicleChanged();
}
