#include "NexusOfflineModel.h"

#include <QtCore/QDir>

#include "AppSettings.h"
#include "QGCMapEngineManager.h"
#include "QGCNetworkHelper.h"
#include "QmlObjectListModel.h"
#include "SettingsManager.h"

NexusOfflineModel::NexusOfflineModel(QObject *parent)
    : QObject(parent)
{
    _timer.setInterval(1000);
    _timer.setTimerType(Qt::CoarseTimer);
    connect(&_timer, &QTimer::timeout, this, &NexusOfflineModel::offlineChanged);
    _timer.start();

    reloadMapSets();
}

bool NexusOfflineModel::internetAvailable() const
{
    return QGCNetworkHelper::isInternetAvailable();
}

QString NexusOfflineModel::missionPath() const
{
    return SettingsManager::instance()->appSettings()->missionSavePath();
}

QString NexusOfflineModel::parameterPath() const
{
    return SettingsManager::instance()->appSettings()->parameterSavePath();
}

QString NexusOfflineModel::settingsPath() const
{
    return SettingsManager::instance()->appSettings()->settingsSavePath();
}

QString NexusOfflineModel::telemetryPath() const
{
    return SettingsManager::instance()->appSettings()->telemetrySavePath();
}

QString NexusOfflineModel::logPath() const
{
    return SettingsManager::instance()->appSettings()->logSavePath();
}

QString NexusOfflineModel::videoPath() const
{
    return SettingsManager::instance()->appSettings()->videoSavePath();
}

QString NexusOfflineModel::photoPath() const
{
    return SettingsManager::instance()->appSettings()->photoSavePath();
}

bool NexusOfflineModel::_pathReady(const QString &path) const
{
    if (path.trimmed().isEmpty()) return false;
    QDir dir(path);
    if (!dir.exists() && !QDir().mkpath(path)) return false;
    return QFileInfo(path).isDir() && QFileInfo(path).isWritable();
}

bool NexusOfflineModel::localStorageReady() const
{
    return _pathReady(missionPath()) &&
           _pathReady(parameterPath()) &&
           _pathReady(settingsPath()) &&
           _pathReady(telemetryPath()) &&
           _pathReady(logPath()) &&
           _pathReady(videoPath()) &&
           _pathReady(photoPath());
}

int NexusOfflineModel::offlineMapSetCount() const
{
    auto *manager = QGCMapEngineManager::instance();
    return manager && manager->tileSets() ? manager->tileSets()->count() : 0;
}

QString NexusOfflineModel::offlineMapCacheSize() const
{
    auto *manager = QGCMapEngineManager::instance();
    return manager ? manager->tileSizeStr() : QStringLiteral("--");
}

bool NexusOfflineModel::offlineReady() const
{
    // Map coverage is location-specific, so cached-set count is reported
    // separately rather than falsely claiming every future area is cached.
    return localStorageReady();
}

QString NexusOfflineModel::networkState() const
{
    return internetAvailable() ? QStringLiteral("ONLINE") : QStringLiteral("OFFLINE");
}

QString NexusOfflineModel::readinessState() const
{
    if (!localStorageReady()) return QStringLiteral("STORAGE BLOCKED");
    if (offlineMapSetCount() <= 0) return QStringLiteral("LOCAL READY · MAPS NOT CACHED");
    return QStringLiteral("OFFLINE READY");
}

void NexusOfflineModel::refresh()
{
    emit offlineChanged();
}

void NexusOfflineModel::reloadMapSets()
{
    if (auto *manager = QGCMapEngineManager::instance()) {
        manager->loadTileSets();
    }
    emit offlineChanged();
}
