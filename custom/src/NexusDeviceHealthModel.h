#pragma once

#include <QtCore/QObject>
#include <QtCore/QTimer>

class NexusDeviceHealthModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool android READ android CONSTANT)
    Q_PROPERTY(int batteryPercent READ batteryPercent NOTIFY deviceHealthChanged)
    Q_PROPERTY(QString batteryState READ batteryState NOTIFY deviceHealthChanged)
    Q_PROPERTY(double temperatureC READ temperatureC NOTIFY deviceHealthChanged)
    Q_PROPERTY(QString thermalState READ thermalState NOTIFY deviceHealthChanged)
    Q_PROPERTY(quint64 storageFreeBytes READ storageFreeBytes NOTIFY deviceHealthChanged)
    Q_PROPERTY(quint64 storageTotalBytes READ storageTotalBytes NOTIFY deviceHealthChanged)
    Q_PROPERTY(QString storageFreeText READ storageFreeText NOTIFY deviceHealthChanged)
    Q_PROPERTY(int ramTotalMb READ ramTotalMb NOTIFY deviceHealthChanged)
    Q_PROPERTY(int ramAvailableMb READ ramAvailableMb NOTIFY deviceHealthChanged)
    Q_PROPERTY(bool usbConnected READ usbConnected NOTIFY deviceHealthChanged)
    Q_PROPERTY(QStringList usbPorts READ usbPorts NOTIFY deviceHealthChanged)
    Q_PROPERTY(QString networkState READ networkState NOTIFY deviceHealthChanged)
    Q_PROPERTY(QString cameraPermission READ cameraPermission NOTIFY deviceHealthChanged)
    Q_PROPERTY(QString locationPermission READ locationPermission NOTIFY deviceHealthChanged)
    Q_PROPERTY(QString storagePermission READ storagePermission NOTIFY deviceHealthChanged)
    Q_PROPERTY(bool keepScreenAwake READ keepScreenAwake NOTIFY deviceHealthChanged)
    Q_PROPERTY(bool overheatingWarning READ overheatingWarning NOTIFY deviceHealthChanged)
    Q_PROPERTY(bool lowStorageWarning READ lowStorageWarning NOTIFY deviceHealthChanged)
    Q_PROPERTY(bool lowBatteryWarning READ lowBatteryWarning NOTIFY deviceHealthChanged)
    Q_PROPERTY(QString overallState READ overallState NOTIFY deviceHealthChanged)

public:
    explicit NexusDeviceHealthModel(QObject *parent = nullptr);
    ~NexusDeviceHealthModel() override;

    bool android() const;
    int batteryPercent() const { return _batteryPercent; }
    QString batteryState() const { return _batteryState; }
    double temperatureC() const { return _temperatureC; }
    QString thermalState() const { return _thermalState; }
    quint64 storageFreeBytes() const { return _storageFreeBytes; }
    quint64 storageTotalBytes() const { return _storageTotalBytes; }
    QString storageFreeText() const;
    int ramTotalMb() const { return _ramTotalMb; }
    int ramAvailableMb() const { return _ramAvailableMb; }
    bool usbConnected() const { return !_usbPorts.isEmpty(); }
    QStringList usbPorts() const { return _usbPorts; }
    QString networkState() const { return _networkState; }
    QString cameraPermission() const { return _cameraPermission; }
    QString locationPermission() const { return _locationPermission; }
    QString storagePermission() const { return _storagePermission; }
    bool keepScreenAwake() const { return _keepScreenAwake; }

    bool overheatingWarning() const;
    bool lowStorageWarning() const;
    bool lowBatteryWarning() const;
    QString overallState() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setOperationalScreenAwake(bool awake);
    Q_INVOKABLE void requestCameraPermission();
    Q_INVOKABLE void requestLocationPermission();

signals:
    void deviceHealthChanged();

private:
    static QString _permissionState(Qt::PermissionStatus status);
    int _readMemAvailableMb() const;
    double _readBatteryTemperatureC() const;
    int _androidThermalStatus() const;
    void _refreshPermissions();

    QTimer _timer;
    int _batteryPercent = -1;
    QString _batteryState = QStringLiteral("UNKNOWN");
    double _temperatureC = qQNaN();
    QString _thermalState = QStringLiteral("UNKNOWN");
    quint64 _storageFreeBytes = 0;
    quint64 _storageTotalBytes = 0;
    int _ramTotalMb = -1;
    int _ramAvailableMb = -1;
    QStringList _usbPorts;
    QString _networkState = QStringLiteral("UNKNOWN");
    QString _cameraPermission = QStringLiteral("UNKNOWN");
    QString _locationPermission = QStringLiteral("UNKNOWN");
    QString _storagePermission = QStringLiteral("UNKNOWN");
    bool _keepScreenAwake = false;
};
