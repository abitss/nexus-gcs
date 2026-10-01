#pragma once

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QTimer>

class NexusAlertManager;
class NexusHealthModel;
class Vehicle;

class NexusPreflightModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString overallState READ overallState NOTIFY preflightChanged)
    Q_PROPERTY(QString overallDetail READ overallDetail NOTIFY preflightChanged)

    Q_PROPERTY(QString vehicleState READ vehicleState NOTIFY preflightChanged)
    Q_PROPERTY(QString vehicleDetail READ vehicleDetail NOTIFY preflightChanged)
    Q_PROPERTY(QString sensorsState READ sensorsState NOTIFY preflightChanged)
    Q_PROPERTY(QString sensorsDetail READ sensorsDetail NOTIFY preflightChanged)
    Q_PROPERTY(QString navigationState READ navigationState NOTIFY preflightChanged)
    Q_PROPERTY(QString navigationDetail READ navigationDetail NOTIFY preflightChanged)
    Q_PROPERTY(QString batteryState READ batteryState NOTIFY preflightChanged)
    Q_PROPERTY(QString batteryDetail READ batteryDetail NOTIFY preflightChanged)
    Q_PROPERTY(QString homeState READ homeState NOTIFY preflightChanged)
    Q_PROPERTY(QString homeDetail READ homeDetail NOTIFY preflightChanged)
    Q_PROPERTY(QString missionState READ missionState NOTIFY preflightChanged)
    Q_PROPERTY(QString missionDetail READ missionDetail NOTIFY preflightChanged)
    Q_PROPERTY(QString geofenceState READ geofenceState NOTIFY preflightChanged)
    Q_PROPERTY(QString geofenceDetail READ geofenceDetail NOTIFY preflightChanged)
    Q_PROPERTY(QString datalinkState READ datalinkState NOTIFY preflightChanged)
    Q_PROPERTY(QString datalinkDetail READ datalinkDetail NOTIFY preflightChanged)
    Q_PROPERTY(QString payloadState READ payloadState NOTIFY preflightChanged)
    Q_PROPERTY(QString payloadDetail READ payloadDetail NOTIFY preflightChanged)

    Q_PROPERTY(int blockedCount READ blockedCount NOTIFY preflightChanged)
    Q_PROPERTY(int warningCount READ warningCount NOTIFY preflightChanged)

public:
    explicit NexusPreflightModel(NexusHealthModel *health,
                                 NexusAlertManager *alerts,
                                 QObject *parent = nullptr);

    QString overallState() const { return _overallState; }
    QString overallDetail() const { return _overallDetail; }

    QString vehicleState() const { return _vehicleState; }
    QString vehicleDetail() const { return _vehicleDetail; }
    QString sensorsState() const { return _sensorsState; }
    QString sensorsDetail() const { return _sensorsDetail; }
    QString navigationState() const { return _navigationState; }
    QString navigationDetail() const { return _navigationDetail; }
    QString batteryState() const { return _batteryState; }
    QString batteryDetail() const { return _batteryDetail; }
    QString homeState() const { return _homeState; }
    QString homeDetail() const { return _homeDetail; }
    QString missionState() const { return _missionState; }
    QString missionDetail() const { return _missionDetail; }
    QString geofenceState() const { return _geofenceState; }
    QString geofenceDetail() const { return _geofenceDetail; }
    QString datalinkState() const { return _datalinkState; }
    QString datalinkDetail() const { return _datalinkDetail; }
    QString payloadState() const { return _payloadState; }
    QString payloadDetail() const { return _payloadDetail; }

    int blockedCount() const { return _blockedCount; }
    int warningCount() const { return _warningCount; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void updateMission(bool hasMission,
                                   bool valid,
                                   bool verified,
                                   const QString &state,
                                   const QString &detail);

signals:
    void preflightChanged();

private slots:
    void _activeVehicleChanged(Vehicle *vehicle);

private:
    static bool _isBlocked(const QString &state) { return state == QStringLiteral("BLOCKED"); }
    static bool _isWarning(const QString &state) { return state == QStringLiteral("WARNING"); }
    static QString _stateFromHealth(const QString &healthState, bool unknownIsWarning = true);

    NexusHealthModel *_health = nullptr;
    NexusAlertManager *_alerts = nullptr;
    QPointer<Vehicle> _vehicle;
    QTimer _timer;

    bool _hasMission = false;
    bool _missionValid = true;
    bool _missionVerified = false;
    QString _missionValidationState = QStringLiteral("NO MISSION");
    QString _missionValidationDetail = QStringLiteral("Manual flight available");

    QString _overallState = QStringLiteral("BLOCKED");
    QString _overallDetail = QStringLiteral("No active vehicle");

    QString _vehicleState = QStringLiteral("BLOCKED");
    QString _vehicleDetail = QStringLiteral("No vehicle connected");
    QString _sensorsState = QStringLiteral("WARNING");
    QString _sensorsDetail = QStringLiteral("Sensor health unknown");
    QString _navigationState = QStringLiteral("WARNING");
    QString _navigationDetail = QStringLiteral("Navigation health unknown");
    QString _batteryState = QStringLiteral("WARNING");
    QString _batteryDetail = QStringLiteral("Battery health unknown");
    QString _homeState = QStringLiteral("WARNING");
    QString _homeDetail = QStringLiteral("Home validity unknown");
    QString _missionState = QStringLiteral("GO");
    QString _missionDetail = QStringLiteral("No mission loaded; manual flight available");
    QString _geofenceState = QStringLiteral("WARNING");
    QString _geofenceDetail = QStringLiteral("Geofence status unknown");
    QString _datalinkState = QStringLiteral("BLOCKED");
    QString _datalinkDetail = QStringLiteral("No datalink");
    QString _payloadState = QStringLiteral("GO");
    QString _payloadDetail = QStringLiteral("Payload not configured");

    int _blockedCount = 0;
    int _warningCount = 0;
};
