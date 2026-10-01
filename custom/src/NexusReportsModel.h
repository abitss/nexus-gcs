#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariantList>
#include <QtCore/QVariantMap>

class NexusReportsModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sourcePath READ sourcePath NOTIFY reportChanged)
    Q_PROPERTY(QString missionId READ missionId WRITE setMissionId NOTIFY reportChanged)
    Q_PROPERTY(QString operatorName READ operatorName WRITE setOperatorName NOTIFY reportChanged)
    Q_PROPERTY(QString aircraft READ aircraft WRITE setAircraft NOTIFY reportChanged)
    Q_PROPERTY(QString firmware READ firmware WRITE setFirmware NOTIFY reportChanged)
    Q_PROPERTY(QString missionCompletion READ missionCompletion WRITE setMissionCompletion NOTIFY reportChanged)

public:
    explicit NexusReportsModel(QObject *parent = nullptr);

    QString sourcePath() const { return _sourcePath; }
    QString missionId() const { return _missionId; }
    QString operatorName() const { return _operatorName; }
    QString aircraft() const { return _aircraft; }
    QString firmware() const { return _firmware; }
    QString missionCompletion() const { return _missionCompletion; }

    void setMissionId(const QString &value);
    void setOperatorName(const QString &value);
    void setAircraft(const QString &value);
    void setFirmware(const QString &value);
    void setMissionCompletion(const QString &value);

    Q_INVOKABLE void loadForSource(const QString &sourcePath);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QVariantMap buildReportData(
        const QString &dateTime,
        double durationSeconds,
        double distanceMeters,
        double maxAltitudeMeters,
        double batteryUsedPercent,
        const QVariantList &warnings,
        const QVariantList &events,
        const QVariantList &route) const;

signals:
    void reportChanged();

private:
    QString _settingsGroupForSource(const QString &path) const;
    void _save();

    QString _sourcePath;
    QString _missionId;
    QString _operatorName;
    QString _aircraft;
    QString _firmware;
    QString _missionCompletion = QStringLiteral("NOT RECORDED");
};
