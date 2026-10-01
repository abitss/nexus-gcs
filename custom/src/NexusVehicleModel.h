#pragma once

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

class Fact;
class ParameterManager;
class Vehicle;
class VehicleComponent;

class NexusVehicleModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ connected NOTIFY vehicleChanged)
    Q_PROPERTY(bool linkLost READ linkLost NOTIFY vehicleChanged)
    Q_PROPERTY(bool armed READ armed NOTIFY vehicleChanged)
    Q_PROPERTY(bool flying READ flying NOTIFY vehicleChanged)
    Q_PROPERTY(bool safeToConfigure READ safeToConfigure NOTIFY vehicleChanged)
    Q_PROPERTY(bool safeToReboot READ safeToReboot NOTIFY vehicleChanged)
    Q_PROPERTY(bool parametersReady READ parametersReady NOTIFY vehicleChanged)
    Q_PROPERTY(bool pendingWrites READ pendingWrites NOTIFY vehicleChanged)
    Q_PROPERTY(bool rebootRequired READ rebootRequired NOTIFY vehicleChanged)
    Q_PROPERTY(QStringList rebootParameters READ rebootParameters NOTIFY vehicleChanged)

    Q_PROPERTY(QString vehicleIdText READ vehicleIdText NOTIFY vehicleChanged)
    Q_PROPERTY(QString uidText READ uidText NOTIFY vehicleChanged)
    Q_PROPERTY(QString vehicleType READ vehicleType NOTIFY vehicleChanged)
    Q_PROPERTY(QString firmwareType READ firmwareType NOTIFY vehicleChanged)
    Q_PROPERTY(QString firmwareVersion READ firmwareVersion NOTIFY vehicleChanged)
    Q_PROPERTY(QString firmwareGitHash READ firmwareGitHash NOTIFY vehicleChanged)
    Q_PROPERTY(QString flightMode READ flightMode NOTIFY vehicleChanged)
    Q_PROPERTY(QString setupState READ setupState NOTIFY vehicleChanged)
    Q_PROPERTY(QString parameterState READ parameterState NOTIFY vehicleChanged)

    Q_PROPERTY(bool airframeAvailable READ airframeAvailable NOTIFY vehicleChanged)
    Q_PROPERTY(QString airframeState READ airframeState NOTIFY vehicleChanged)
    Q_PROPERTY(bool sensorsAvailable READ sensorsAvailable NOTIFY vehicleChanged)
    Q_PROPERTY(QString sensorsState READ sensorsState NOTIFY vehicleChanged)
    Q_PROPERTY(bool powerAvailable READ powerAvailable NOTIFY vehicleChanged)
    Q_PROPERTY(QString powerState READ powerState NOTIFY vehicleChanged)
    Q_PROPERTY(bool radioAvailable READ radioAvailable NOTIFY vehicleChanged)
    Q_PROPERTY(QString radioState READ radioState NOTIFY vehicleChanged)
    Q_PROPERTY(bool flightModesAvailable READ flightModesAvailable NOTIFY vehicleChanged)
    Q_PROPERTY(QString flightModesState READ flightModesState NOTIFY vehicleChanged)
    Q_PROPERTY(bool safetyAvailable READ safetyAvailable NOTIFY vehicleChanged)
    Q_PROPERTY(QString safetyState READ safetyState NOTIFY vehicleChanged)

public:
    explicit NexusVehicleModel(QObject *parent = nullptr);

    bool connected() const { return !_vehicle.isNull(); }
    bool linkLost() const;
    bool armed() const;
    bool flying() const;
    bool safeToConfigure() const;
    bool safeToReboot() const;
    bool parametersReady() const;
    bool pendingWrites() const;
    bool rebootRequired() const { return !_rebootParameters.isEmpty(); }
    QStringList rebootParameters() const;

    QString vehicleIdText() const;
    QString uidText() const;
    QString vehicleType() const;
    QString firmwareType() const;
    QString firmwareVersion() const;
    QString firmwareGitHash() const;
    QString flightMode() const;
    QString setupState() const;
    QString parameterState() const;

    bool airframeAvailable() const;
    QString airframeState() const;
    bool sensorsAvailable() const;
    QString sensorsState() const;
    bool powerAvailable() const;
    QString powerState() const;
    bool radioAvailable() const;
    QString radioState() const;
    bool flightModesAvailable() const;
    QString flightModesState() const;
    bool safetyAvailable() const;
    QString safetyState() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool rebootVehicle();

signals:
    void vehicleChanged();

private slots:
    void _activeVehicleChanged(Vehicle *vehicle);
    void _parameterWriteFailed(int componentId, const QString &paramName);

private:
    void _setVehicle(Vehicle *vehicle);
    void _attachKnownFacts();
    void _attachFact(int componentId, Fact *fact);
    VehicleComponent *_knownComponent(int knownComponent) const;
    VehicleComponent *_airframeComponent() const;
    static QString _componentState(VehicleComponent *component);

    QPointer<Vehicle> _vehicle;
    QPointer<ParameterManager> _parameterManager;
    QSet<QString> _rebootParameters;
    QSet<Fact *> _observedFacts;
    QTimer _timer;
};
