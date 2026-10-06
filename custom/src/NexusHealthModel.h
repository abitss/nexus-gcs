#pragma once

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

#include "QGCMAVLink.h"

class Vehicle;

class NexusHealthModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString overallState READ overallState NOTIFY healthChanged)
    Q_PROPERTY(QString overallDetail READ overallDetail NOTIFY healthChanged)

    Q_PROPERTY(QString gpsState READ gpsState NOTIFY healthChanged)
    Q_PROPERTY(QString gpsDetail READ gpsDetail NOTIFY healthChanged)
    Q_PROPERTY(QString ekfState READ ekfState NOTIFY healthChanged)
    Q_PROPERTY(QString ekfDetail READ ekfDetail NOTIFY healthChanged)
    Q_PROPERTY(QString imuState READ imuState NOTIFY healthChanged)
    Q_PROPERTY(QString imuDetail READ imuDetail NOTIFY healthChanged)
    Q_PROPERTY(QString compassState READ compassState NOTIFY healthChanged)
    Q_PROPERTY(QString compassDetail READ compassDetail NOTIFY healthChanged)
    Q_PROPERTY(QString gyroState READ gyroState NOTIFY healthChanged)
    Q_PROPERTY(QString gyroDetail READ gyroDetail NOTIFY healthChanged)
    Q_PROPERTY(QString accelerometerState READ accelerometerState NOTIFY healthChanged)
    Q_PROPERTY(QString accelerometerDetail READ accelerometerDetail NOTIFY healthChanged)
    Q_PROPERTY(QString barometerState READ barometerState NOTIFY healthChanged)
    Q_PROPERTY(QString barometerDetail READ barometerDetail NOTIFY healthChanged)
    Q_PROPERTY(QString batteryState READ batteryState NOTIFY healthChanged)
    Q_PROPERTY(QString batteryDetail READ batteryDetail NOTIFY healthChanged)
    Q_PROPERTY(QString datalinkState READ datalinkState NOTIFY healthChanged)
    Q_PROPERTY(QString datalinkDetail READ datalinkDetail NOTIFY healthChanged)
    Q_PROPERTY(QString homeState READ homeState NOTIFY healthChanged)
    Q_PROPERTY(QString homeDetail READ homeDetail NOTIFY healthChanged)
    Q_PROPERTY(QString geofenceState READ geofenceState NOTIFY healthChanged)
    Q_PROPERTY(QString geofenceDetail READ geofenceDetail NOTIFY healthChanged)
    Q_PROPERTY(QStringList sensorFailures READ sensorFailures NOTIFY healthChanged)
    Q_PROPERTY(bool vehicleConnected READ vehicleConnected NOTIFY healthChanged)

public:
    explicit NexusHealthModel(QObject *parent = nullptr);

    QString overallState() const { return _overallState; }
    QString overallDetail() const { return _overallDetail; }

    QString gpsState() const { return _gpsState; }
    QString gpsDetail() const { return _gpsDetail; }
    QString ekfState() const { return _ekfState; }
    QString ekfDetail() const { return _ekfDetail; }
    QString imuState() const { return _imuState; }
    QString imuDetail() const { return _imuDetail; }
    QString compassState() const { return _compassState; }
    QString compassDetail() const { return _compassDetail; }
    QString gyroState() const { return _gyroState; }
    QString gyroDetail() const { return _gyroDetail; }
    QString accelerometerState() const { return _accelerometerState; }
    QString accelerometerDetail() const { return _accelerometerDetail; }
    QString barometerState() const { return _barometerState; }
    QString barometerDetail() const { return _barometerDetail; }
    QString batteryState() const { return _batteryState; }
    QString batteryDetail() const { return _batteryDetail; }
    QString datalinkState() const { return _datalinkState; }
    QString datalinkDetail() const { return _datalinkDetail; }
    QString homeState() const { return _homeState; }
    QString homeDetail() const { return _homeDetail; }
    QString geofenceState() const { return _geofenceState; }
    QString geofenceDetail() const { return _geofenceDetail; }
    QStringList sensorFailures() const { return _sensorFailures; }
    bool vehicleConnected() const { return !_vehicle.isNull(); }

    Q_INVOKABLE void refresh();

signals:
    void healthChanged();

private slots:
    void _activeVehicleChanged(Vehicle *vehicle);
    void _mavlinkMessageReceived(const mavlink_message_t &message);

private:
    struct StateDetail {
        QString state;
        QString detail;
    };

    static int _severity(const QString &state);
    static QString _worstState(const QStringList &states);
    StateDetail _sensorState(MAV_SYS_STATUS_SENSOR sensor) const;
    void _setVehicle(Vehicle *vehicle);

    QPointer<Vehicle> _vehicle;
    QTimer _timer;

    bool _fenceStatusSeen = false;
    bool _fenceBreached = false;
    FENCE_BREACH _fenceBreachType = FENCE_BREACH_NONE;

    QString _overallState = QStringLiteral("DEGRADED");
    QString _overallDetail = QStringLiteral("No active vehicle");

    QString _gpsState = QStringLiteral("UNKNOWN");
    QString _gpsDetail = QStringLiteral("No vehicle");
    QString _ekfState = QStringLiteral("UNKNOWN");
    QString _ekfDetail = QStringLiteral("No vehicle");
    QString _imuState = QStringLiteral("UNKNOWN");
    QString _imuDetail = QStringLiteral("No vehicle");
    QString _compassState = QStringLiteral("UNKNOWN");
    QString _compassDetail = QStringLiteral("No vehicle");
    QString _gyroState = QStringLiteral("UNKNOWN");
    QString _gyroDetail = QStringLiteral("No vehicle");
    QString _accelerometerState = QStringLiteral("UNKNOWN");
    QString _accelerometerDetail = QStringLiteral("No vehicle");
    QString _barometerState = QStringLiteral("UNKNOWN");
    QString _barometerDetail = QStringLiteral("No vehicle");
    QString _batteryState = QStringLiteral("UNKNOWN");
    QString _batteryDetail = QStringLiteral("No vehicle");
    QString _datalinkState = QStringLiteral("UNKNOWN");
    QString _datalinkDetail = QStringLiteral("No vehicle");
    QString _homeState = QStringLiteral("UNKNOWN");
    QString _homeDetail = QStringLiteral("No vehicle");
    QString _geofenceState = QStringLiteral("UNKNOWN");
    QString _geofenceDetail = QStringLiteral("No vehicle");
    QStringList _sensorFailures;
};
