#pragma once

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QTimer>

class NexusOfflineModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool internetAvailable READ internetAvailable NOTIFY offlineChanged)
    Q_PROPERTY(bool cloudRequired READ cloudRequired CONSTANT)
    Q_PROPERTY(bool localStorageReady READ localStorageReady NOTIFY offlineChanged)
    Q_PROPERTY(bool offlineReady READ offlineReady NOTIFY offlineChanged)

    Q_PROPERTY(QString missionPath READ missionPath NOTIFY offlineChanged)
    Q_PROPERTY(QString parameterPath READ parameterPath NOTIFY offlineChanged)
    Q_PROPERTY(QString settingsPath READ settingsPath NOTIFY offlineChanged)
    Q_PROPERTY(QString telemetryPath READ telemetryPath NOTIFY offlineChanged)
    Q_PROPERTY(QString logPath READ logPath NOTIFY offlineChanged)
    Q_PROPERTY(QString videoPath READ videoPath NOTIFY offlineChanged)
    Q_PROPERTY(QString photoPath READ photoPath NOTIFY offlineChanged)

    Q_PROPERTY(int offlineMapSetCount READ offlineMapSetCount NOTIFY offlineChanged)
    Q_PROPERTY(QString offlineMapCacheSize READ offlineMapCacheSize NOTIFY offlineChanged)
    Q_PROPERTY(QString networkState READ networkState NOTIFY offlineChanged)
    Q_PROPERTY(QString readinessState READ readinessState NOTIFY offlineChanged)

public:
    explicit NexusOfflineModel(QObject *parent = nullptr);

    bool internetAvailable() const;
    bool cloudRequired() const { return false; }
    bool localStorageReady() const;
    bool offlineReady() const;

    QString missionPath() const;
    QString parameterPath() const;
    QString settingsPath() const;
    QString telemetryPath() const;
    QString logPath() const;
    QString videoPath() const;
    QString photoPath() const;

    int offlineMapSetCount() const;
    QString offlineMapCacheSize() const;
    QString networkState() const;
    QString readinessState() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void reloadMapSets();

signals:
    void offlineChanged();

private:
    bool _pathReady(const QString &path) const;
    QTimer _timer;
};
