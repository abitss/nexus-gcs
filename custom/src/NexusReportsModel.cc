#include "NexusReportsModel.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QSettings>
#include <QtCore/QtMath>
#include <QtPositioning/QGeoCoordinate>
#include <limits>

NexusReportsModel::NexusReportsModel(QObject *parent)
    : QObject(parent)
{
}

QString NexusReportsModel::_settingsGroupForSource(const QString &path) const
{
    const QByteArray digest = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha256).toHex();
    return QStringLiteral("NexusReports/%1").arg(QString::fromLatin1(digest));
}

void NexusReportsModel::loadForSource(const QString &sourcePath)
{
    _sourcePath = sourcePath;

    _missionId.clear();
    _operatorName.clear();
    _aircraft.clear();
    _firmware.clear();
    _missionCompletion = QStringLiteral("NOT RECORDED");

    if (!_sourcePath.isEmpty()) {
        QSettings settings;
        settings.beginGroup(_settingsGroupForSource(_sourcePath));
        _missionId = settings.value(QStringLiteral("missionId")).toString();
        _operatorName = settings.value(QStringLiteral("operatorName")).toString();
        _aircraft = settings.value(QStringLiteral("aircraft")).toString();
        _firmware = settings.value(QStringLiteral("firmware")).toString();
        _missionCompletion = settings.value(QStringLiteral("missionCompletion"), QStringLiteral("NOT RECORDED")).toString();
    }

    emit reportChanged();
}

void NexusReportsModel::clear()
{
    loadForSource(QString());
}

void NexusReportsModel::_save()
{
    if (_sourcePath.isEmpty()) return;

    QSettings settings;
    settings.beginGroup(_settingsGroupForSource(_sourcePath));
    settings.setValue(QStringLiteral("missionId"), _missionId);
    settings.setValue(QStringLiteral("operatorName"), _operatorName);
    settings.setValue(QStringLiteral("aircraft"), _aircraft);
    settings.setValue(QStringLiteral("firmware"), _firmware);
    settings.setValue(QStringLiteral("missionCompletion"), _missionCompletion);
}

void NexusReportsModel::setMissionId(const QString &value)
{
    if (_missionId == value) return;
    _missionId = value;
    _save();
    emit reportChanged();
}

void NexusReportsModel::setOperatorName(const QString &value)
{
    if (_operatorName == value) return;
    _operatorName = value;
    _save();
    emit reportChanged();
}

void NexusReportsModel::setAircraft(const QString &value)
{
    if (_aircraft == value) return;
    _aircraft = value;
    _save();
    emit reportChanged();
}

void NexusReportsModel::setFirmware(const QString &value)
{
    if (_firmware == value) return;
    _firmware = value;
    _save();
    emit reportChanged();
}

void NexusReportsModel::setMissionCompletion(const QString &value)
{
    if (_missionCompletion == value) return;
    _missionCompletion = value;
    _save();
    emit reportChanged();
}

double NexusReportsModel::routeDistanceMeters(const QVariantList &route) const
{
    if (route.size() < 2) return qQNaN();

    double total = 0.0;
    bool haveSegment = false;
    for (qsizetype i = 1; i < route.size(); ++i) {
        const QVariantMap aMap = route.at(i - 1).toMap();
        const QVariantMap bMap = route.at(i).toMap();
        const QGeoCoordinate a(aMap.value(QStringLiteral("latitude")).toDouble(),
                               aMap.value(QStringLiteral("longitude")).toDouble());
        const QGeoCoordinate b(bMap.value(QStringLiteral("latitude")).toDouble(),
                               bMap.value(QStringLiteral("longitude")).toDouble());
        if (!a.isValid() || !b.isValid()) continue;
        total += a.distanceTo(b);
        haveSegment = true;
    }
    return haveSegment ? total : qQNaN();
}

double NexusReportsModel::maxSampleValue(const QVariantList &samples) const
{
    if (samples.isEmpty()) return qQNaN();

    double maximum = -std::numeric_limits<double>::infinity();
    bool found = false;
    for (const QVariant &sample : samples) {
        const QVariantMap point = sample.toMap();
        bool ok = false;
        const double value = point.value(QStringLiteral("y")).toDouble(&ok);
        if (!ok || qIsNaN(value)) continue;
        maximum = qMax(maximum, value);
        found = true;
    }
    return found ? maximum : qQNaN();
}

double NexusReportsModel::batteryUsedPercent(const QVariantList &samples) const
{
    if (samples.size() < 2) return qQNaN();

    const QVariantMap first = samples.first().toMap();
    const QVariantMap last = samples.last().toMap();
    bool firstOk = false;
    bool lastOk = false;
    const double start = first.value(QStringLiteral("y")).toDouble(&firstOk);
    const double end = last.value(QStringLiteral("y")).toDouble(&lastOk);
    if (!firstOk || !lastOk || qIsNaN(start) || qIsNaN(end)) return qQNaN();

    const double used = start - end;
    return (used >= 0.0 && used <= 100.0) ? used : qQNaN();
}

QVariantMap NexusReportsModel::buildReportData(
    const QString &dateTime,
    double durationSeconds,
    double distanceMeters,
    double maxAltitudeMeters,
    double batteryUsedPercent,
    const QVariantList &warnings,
    const QVariantList &events,
    const QVariantList &route) const
{
    QVariantMap report;
    report.insert(QStringLiteral("schemaVersion"), QStringLiteral("1.0"));
    report.insert(QStringLiteral("sourcePath"), _sourcePath);
    report.insert(QStringLiteral("missionId"), _missionId);
    report.insert(QStringLiteral("aircraft"), _aircraft);
    report.insert(QStringLiteral("operator"), _operatorName);
    report.insert(QStringLiteral("firmware"), _firmware);
    report.insert(QStringLiteral("dateTime"), dateTime);
    report.insert(QStringLiteral("durationSeconds"), durationSeconds);
    report.insert(QStringLiteral("distanceMeters"), distanceMeters);
    report.insert(QStringLiteral("maxAltitudeMeters"), maxAltitudeMeters);
    report.insert(QStringLiteral("batteryUsedPercent"), batteryUsedPercent);
    report.insert(QStringLiteral("missionCompletion"), _missionCompletion);
    report.insert(QStringLiteral("warnings"), warnings);
    report.insert(QStringLiteral("events"), events);
    report.insert(QStringLiteral("route"), route);
    report.insert(QStringLiteral("exportTargets"), QStringList{
        QStringLiteral("PDF"),
        QStringLiteral("CSV"),
        QStringLiteral("KML")
    });
    return report;
}
