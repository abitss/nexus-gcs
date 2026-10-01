#include "NexusDeviceHealthModel.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QPermissions>
#include <QtCore/QStorageInfo>
#include <QtCore/QtMath>

#include "AppSettings.h"
#include "MultiVehicleManager.h"
#include "Vehicle.h"
#include "QGCFormat.h"
#include "QGCNetworkHelper.h"
#include "SerialPortManager.h"
#include "SettingsManager.h"
#include "SDLPlatform.h"

#ifdef Q_OS_ANDROID
#include "AndroidInterface.h"
#include <QtCore/QJniEnvironment>
#include <QtCore/QJniObject>
#endif

NexusDeviceHealthModel::NexusDeviceHealthModel(QObject *parent)
    : QObject(parent)
{
    _timer.setInterval(2000);
    _timer.setTimerType(Qt::CoarseTimer);
    connect(&_timer, &QTimer::timeout, this, &NexusDeviceHealthModel::refresh);
    _timer.start();

    auto *manager = MultiVehicleManager::instance();
    connect(manager, &MultiVehicleManager::activeVehicleChanged,
            this, &NexusDeviceHealthModel::_activeVehicleChanged);
    _activeVehicleChanged(manager->activeVehicle());
    refresh();
}

NexusDeviceHealthModel::~NexusDeviceHealthModel()
{
#ifdef Q_OS_ANDROID
    if (_keepScreenAwake) {
        AndroidInterface::setKeepScreenOn(false);
    }
#endif
}

bool NexusDeviceHealthModel::android() const
{
#ifdef Q_OS_ANDROID
    return true;
#else
    return false;
#endif
}

QString NexusDeviceHealthModel::storageFreeText() const
{
    return QGC::bigSizeToString(_storageFreeBytes);
}

QString NexusDeviceHealthModel::_permissionState(Qt::PermissionStatus status)
{
    switch (status) {
    case Qt::PermissionStatus::Granted: return QStringLiteral("GRANTED");
    case Qt::PermissionStatus::Denied: return QStringLiteral("DENIED");
    case Qt::PermissionStatus::Undetermined: return QStringLiteral("REQUIRED");
    }
    return QStringLiteral("UNKNOWN");
}

int NexusDeviceHealthModel::_readMemAvailableMb() const
{
#if defined(Q_OS_LINUX) || defined(Q_OS_ANDROID)
    QFile file(QStringLiteral("/proc/meminfo"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!file.atEnd()) {
            const QByteArray line = file.readLine();
            if (line.startsWith("MemAvailable:")) {
                const QList<QByteArray> parts = line.simplified().split(' ');
                if (parts.size() >= 2) {
                    bool ok = false;
                    const qlonglong kb = parts.at(1).toLongLong(&ok);
                    if (ok) return static_cast<int>(kb / 1024);
                }
            }
        }
    }
#endif
    return -1;
}

double NexusDeviceHealthModel::_readBatteryTemperatureC() const
{
#if defined(Q_OS_LINUX) || defined(Q_OS_ANDROID)
    const QStringList candidates = {
        QStringLiteral("/sys/class/power_supply/battery/temp"),
        QStringLiteral("/sys/class/power_supply/BAT0/temp")
    };
    for (const QString &path : candidates) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        bool ok = false;
        double raw = QString::fromUtf8(file.readAll()).trimmed().toDouble(&ok);
        if (!ok) continue;
        if (raw > 1000.0) raw /= 1000.0;
        else if (raw > 100.0) raw /= 10.0;
        if (raw >= -20.0 && raw <= 120.0) return raw;
    }
#endif
    return qQNaN();
}

int NexusDeviceHealthModel::_androidThermalStatus() const
{
#ifdef Q_OS_ANDROID
    if (SDLPlatform::getAndroidSDKVersion() < 29) return -1;

    QJniObject activity = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative",
        "activity",
        "()Landroid/app/Activity;");
    if (!activity.isValid()) return -1;

    QJniObject serviceName = QJniObject::fromString(QStringLiteral("power"));
    QJniObject powerManager = activity.callObjectMethod(
        "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;",
        serviceName.object<jstring>());
    if (!powerManager.isValid()) return -1;

    const jint status = powerManager.callMethod<jint>("getCurrentThermalStatus", "()I");
    QJniEnvironment env;
    if (env.checkAndClearExceptions()) return -1;
    return static_cast<int>(status);
#else
    return -1;
#endif
}

void NexusDeviceHealthModel::_refreshPermissions()
{
    QCameraPermission camera;
    _cameraPermission = _permissionState(QCoreApplication::instance()->checkPermission(camera));

    QLocationPermission location;
    location.setAccuracy(QLocationPermission::Precise);
    _locationPermission = _permissionState(QCoreApplication::instance()->checkPermission(location));

#ifdef Q_OS_ANDROID
    _storagePermission = AndroidInterface::checkStoragePermissions()
        ? QStringLiteral("GRANTED")
        : QStringLiteral("DENIED");
#else
    _storagePermission = QStringLiteral("NOT REQUIRED");
#endif
}

void NexusDeviceHealthModel::refresh()
{
    int seconds = -1;
    _batteryState = SDLPlatform::getDevicePowerInfo(&seconds, &_batteryPercent).toUpper();

    _temperatureC = _readBatteryTemperatureC();

    const int thermal = _androidThermalStatus();
    static const QStringList thermalNames = {
        QStringLiteral("NONE"),
        QStringLiteral("LIGHT"),
        QStringLiteral("MODERATE"),
        QStringLiteral("SEVERE"),
        QStringLiteral("CRITICAL"),
        QStringLiteral("EMERGENCY"),
        QStringLiteral("SHUTDOWN")
    };
    if (thermal >= 0 && thermal < thermalNames.size()) {
        _thermalState = thermalNames.at(thermal);
    } else if (!qIsNaN(_temperatureC)) {
        _thermalState = _temperatureC >= 50.0 ? QStringLiteral("CRITICAL")
                      : _temperatureC >= 45.0 ? QStringLiteral("SEVERE")
                      : _temperatureC >= 40.0 ? QStringLiteral("MODERATE")
                                              : QStringLiteral("NORMAL");
    } else {
        _thermalState = QStringLiteral("UNKNOWN");
    }

    const QString savePath = SettingsManager::instance()->appSettings()->savePath()->rawValue().toString();
    QStorageInfo storage(savePath.isEmpty() ? QStorageInfo::root() : QStorageInfo(savePath));
    _storageFreeBytes = storage.isValid() && storage.isReady() ? static_cast<quint64>(storage.bytesAvailable()) : 0;
    _storageTotalBytes = storage.isValid() && storage.isReady() ? static_cast<quint64>(storage.bytesTotal()) : 0;

    _ramTotalMb = SDLPlatform::getSystemRAM();
    _ramAvailableMb = _readMemAvailableMb();

    const auto ports = SerialPortManager::instance()->availablePorts();
    _usbPorts.clear();
    for (const auto &port : ports) {
        const QString display = port.displayName.isEmpty() ? port.portName : port.displayName;
        if (!display.isEmpty()) _usbPorts.append(display);
    }

    _networkState = QGCNetworkHelper::isInternetAvailable()
        ? QStringLiteral("ONLINE")
        : QStringLiteral("OFFLINE");

    _refreshPermissions();
    emit deviceHealthChanged();
}

bool NexusDeviceHealthModel::thermalWarningFor(double temperatureC, const QString &thermalState)
{
    return thermalState == QStringLiteral("SEVERE") ||
           thermalState == QStringLiteral("CRITICAL") ||
           thermalState == QStringLiteral("EMERGENCY") ||
           thermalState == QStringLiteral("SHUTDOWN") ||
           (!qIsNaN(temperatureC) && temperatureC >= 45.0);
}

bool NexusDeviceHealthModel::storageWarningFor(quint64 freeBytes, quint64 totalBytes)
{
    constexpr quint64 kLowStorageBytes = 2ULL * 1024ULL * 1024ULL * 1024ULL;
    if (totalBytes == 0) return false;
    const double fraction = static_cast<double>(freeBytes) / static_cast<double>(totalBytes);
    return freeBytes < kLowStorageBytes || fraction < 0.05;
}

bool NexusDeviceHealthModel::batteryWarningFor(int percent, const QString &state)
{
    return percent >= 0 && percent <= 20 &&
           state != QStringLiteral("CHARGING") &&
           state != QStringLiteral("CHARGED");
}

bool NexusDeviceHealthModel::overheatingWarning() const
{
    return thermalWarningFor(_temperatureC, _thermalState);
}

bool NexusDeviceHealthModel::lowStorageWarning() const
{
    return storageWarningFor(_storageFreeBytes, _storageTotalBytes);
}

bool NexusDeviceHealthModel::lowBatteryWarning() const
{
    return batteryWarningFor(_batteryPercent, _batteryState);
}

QString NexusDeviceHealthModel::overallState() const
{
    if (overheatingWarning()) return QStringLiteral("CRITICAL");
    if (lowBatteryWarning() || lowStorageWarning()) return QStringLiteral("WARNING");
    return QStringLiteral("NOMINAL");
}

void NexusDeviceHealthModel::setOperationalScreenAwake(bool awake)
{
    if (_keepScreenAwake == awake) return;
    _keepScreenAwake = awake;
#ifdef Q_OS_ANDROID
    AndroidInterface::setKeepScreenOn(awake);
#endif
    emit deviceHealthChanged();
}

void NexusDeviceHealthModel::requestCameraPermission()
{
    QCameraPermission permission;
    QCoreApplication::instance()->requestPermission(permission, this, [this](const QPermission &) {
        _refreshPermissions();
        emit deviceHealthChanged();
    });
}

void NexusDeviceHealthModel::requestLocationPermission()
{
    QLocationPermission permission;
    permission.setAccuracy(QLocationPermission::Precise);
    QCoreApplication::instance()->requestPermission(permission, this, [this](const QPermission &) {
        _refreshPermissions();
        emit deviceHealthChanged();
    });
}

void NexusDeviceHealthModel::_activeVehicleChanged(Vehicle *vehicle)
{
    setOperationalScreenAwake(vehicle != nullptr);
}
