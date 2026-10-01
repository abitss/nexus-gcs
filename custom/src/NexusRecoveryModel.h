#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtCore/QVariantList>

class LinkInterface;
class NexusDeviceHealthModel;
class NexusPayloadModel;
class NexusPlanVerifier;
class NexusSecurityModel;
class Vehicle;

#include "MAVLinkMessageType.h"

class NexusRecoveryModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString overallState READ overallState NOTIFY recoveryChanged)
    Q_PROPERTY(QString appState READ appState NOTIFY recoveryChanged)
    Q_PROPERTY(bool previousUncleanExit READ previousUncleanExit NOTIFY recoveryChanged)
    Q_PROPERTY(bool telemetryLost READ telemetryLost NOTIFY recoveryChanged)
    Q_PROPERTY(bool usbLost READ usbLost NOTIFY recoveryChanged)
    Q_PROPERTY(bool udpLost READ udpLost NOTIFY recoveryChanged)
    Q_PROPERTY(bool videoLost READ videoLost NOTIFY recoveryChanged)
    Q_PROPERTY(bool interruptedMission READ interruptedMission NOTIFY recoveryChanged)
    Q_PROPERTY(bool storageBlocked READ storageBlocked NOTIFY recoveryChanged)
    Q_PROPERTY(bool permissionRevoked READ permissionRevoked NOTIFY recoveryChanged)
    Q_PROPERTY(bool fcRebootDetected READ fcRebootDetected NOTIFY recoveryChanged)
    Q_PROPERTY(QString lastRecoveryEvent READ lastRecoveryEvent NOTIFY recoveryChanged)
    Q_PROPERTY(QVariantList events READ events NOTIFY recoveryChanged)

public:
    NexusRecoveryModel(NexusPayloadModel *payload,
                       NexusDeviceHealthModel *deviceHealth,
                       NexusPlanVerifier *planVerifier,
                       NexusSecurityModel *security,
                       QObject *parent = nullptr);
    ~NexusRecoveryModel() override;

    QString overallState() const;
    QString appState() const { return _appState; }
    bool previousUncleanExit() const { return _previousUncleanExit; }
    bool telemetryLost() const { return _telemetryLost; }
    bool usbLost() const { return _usbLost; }
    bool udpLost() const { return _udpLost; }
    bool videoLost() const { return _videoLost; }
    bool interruptedMission() const { return _interruptedMission; }
    bool storageBlocked() const { return _storageBlocked; }
    bool permissionRevoked() const { return _permissionRevoked; }
    bool fcRebootDetected() const { return _fcRebootDetected; }
    QString lastRecoveryEvent() const { return _lastRecoveryEvent; }
    QVariantList events() const { return _events; }

    Q_INVOKABLE void acknowledgeInterruptedMission();
    Q_INVOKABLE void clearRecoveredEvents();
    Q_INVOKABLE bool validateRecoveryMission(const QString &path);

signals:
    void recoveryChanged();

private slots:
    void _activeVehicleChanged(Vehicle *vehicle);
    void _communicationLostChanged(bool lost);
    void _payloadChanged();
    void _deviceChanged();
    void _planStateChanged();
    void _applicationStateChanged(Qt::ApplicationState state);
    void _mavlinkMessageReceived(LinkInterface *link, const mavlink_message_t &message);

private:
    void _setVehicle(Vehicle *vehicle);
    void _record(const QString &type, const QString &severity, const QString &detail);
    void _markCleanExit();
    static QString _applicationStateName(Qt::ApplicationState state);

    QPointer<Vehicle> _vehicle;
    NexusPayloadModel *_payload = nullptr;
    NexusDeviceHealthModel *_deviceHealth = nullptr;
    NexusPlanVerifier *_planVerifier = nullptr;
    NexusSecurityModel *_security = nullptr;

    QString _appState = QStringLiteral("ACTIVE");
    bool _previousUncleanExit = false;
    bool _telemetryLost = false;
    bool _usbLost = false;
    bool _udpLost = false;
    bool _videoLost = false;
    bool _interruptedMission = false;
    bool _storageBlocked = false;
    bool _permissionRevoked = false;
    bool _fcRebootDetected = false;
    bool _previousUsbConnected = false;
    bool _previousPermissionHealthy = true;
    bool _missionWasBusy = false;
    quint32 _lastBootMs = 0;
    bool _haveBootCounter = false;

    QString _lastRecoveryEvent = QStringLiteral("No recovery event recorded");
    QVariantList _events;
};
