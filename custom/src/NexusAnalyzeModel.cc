#include "NexusAnalyzeModel.h"

#include <QtCore/QDir>
#include <QtCore/QSet>

#include "AppSettings.h"
#include "QGCFormat.h"
#include "SettingsManager.h"

NexusAnalyzeModel::NexusAnalyzeModel(QObject *parent)
    : QObject(parent)
{
    refreshHistory();
}

QString NexusAnalyzeModel::_typeForSuffix(const QString &suffix)
{
    const QString ext = suffix.toLower();
    if (ext == QStringLiteral("tlog")) return QStringLiteral("TELEMETRY REPLAY");
    if (ext == QStringLiteral("ulg")) return QStringLiteral("PX4 ULOG");
    if (ext == QStringLiteral("bin") || ext == QStringLiteral("log")) return QStringLiteral("DATAFLASH");
    return QStringLiteral("UNKNOWN");
}

bool NexusAnalyzeModel::_supportedSuffix(const QString &suffix)
{
    const QString ext = suffix.toLower();
    return ext == QStringLiteral("tlog") ||
           ext == QStringLiteral("ulg") ||
           ext == QStringLiteral("bin") ||
           ext == QStringLiteral("log");
}

QVariantMap NexusAnalyzeModel::_entryForFile(const QFileInfo &fileInfo)
{
    QVariantMap entry;
    entry.insert(QStringLiteral("path"), fileInfo.absoluteFilePath());
    entry.insert(QStringLiteral("name"), fileInfo.fileName());
    entry.insert(QStringLiteral("type"), _typeForSuffix(fileInfo.suffix()));
    entry.insert(QStringLiteral("modified"), fileInfo.lastModified());
    entry.insert(QStringLiteral("dateText"), fileInfo.lastModified().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")));
    entry.insert(QStringLiteral("sizeBytes"), fileInfo.size());
    entry.insert(QStringLiteral("sizeText"), QGC::bigSizeToString(static_cast<quint64>(fileInfo.size())));
    entry.insert(QStringLiteral("replayOnly"), fileInfo.suffix().compare(QStringLiteral("tlog"), Qt::CaseInsensitive) == 0);
    return entry;
}

void NexusAnalyzeModel::refreshHistory()
{
    const auto *app = SettingsManager::instance()->appSettings();
    const QStringList roots = {
        app->telemetrySavePath(),
        app->logSavePath()
    };

    QList<QFileInfo> files;
    QSet<QString> seen;

    for (const QString &root : roots) {
        QDir dir(root);
        if (!dir.exists()) continue;

        const QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Readable, QDir::Time);
        for (const QFileInfo &info : entries) {
            if (!_supportedSuffix(info.suffix())) continue;
            const QString canonical = info.absoluteFilePath();
            if (seen.contains(canonical)) continue;
            seen.insert(canonical);
            files.append(info);
        }
    }

    std::sort(files.begin(), files.end(), [](const QFileInfo &a, const QFileInfo &b) {
        return a.lastModified() > b.lastModified();
    });

    _history.clear();
    for (const QFileInfo &info : files) {
        _history.append(_entryForFile(info));
    }

    if (!_selectedPath.isEmpty() && !QFileInfo::exists(_selectedPath)) {
        _selectedPath.clear();
    }

    emit analyzeChanged();
}

bool NexusAnalyzeModel::selectFlight(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists() || !info.isFile() || !_supportedSuffix(info.suffix())) {
        return false;
    }

    if (_selectedPath != info.absoluteFilePath()) {
        _selectedPath = info.absoluteFilePath();
        emit analyzeChanged();
    }
    return true;
}

void NexusAnalyzeModel::clearSelection()
{
    if (_selectedPath.isEmpty()) return;
    _selectedPath.clear();
    emit analyzeChanged();
}

QString NexusAnalyzeModel::selectedName() const
{
    return _selectedPath.isEmpty() ? QString() : QFileInfo(_selectedPath).fileName();
}

QString NexusAnalyzeModel::selectedType() const
{
    return _selectedPath.isEmpty() ? QStringLiteral("--") : _typeForSuffix(QFileInfo(_selectedPath).suffix());
}

QString NexusAnalyzeModel::selectedDate() const
{
    return _selectedPath.isEmpty()
        ? QStringLiteral("--")
        : QFileInfo(_selectedPath).lastModified().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
}

QString NexusAnalyzeModel::selectedSize() const
{
    return _selectedPath.isEmpty()
        ? QStringLiteral("--")
        : QGC::bigSizeToString(static_cast<quint64>(QFileInfo(_selectedPath).size()));
}

bool NexusAnalyzeModel::selectedReplayOnly() const
{
    return !_selectedPath.isEmpty() &&
           QFileInfo(_selectedPath).suffix().compare(QStringLiteral("tlog"), Qt::CaseInsensitive) == 0;
}

bool NexusAnalyzeModel::selectedFirmwareLog() const
{
    if (_selectedPath.isEmpty()) return false;
    const QString ext = QFileInfo(_selectedPath).suffix().toLower();
    return ext == QStringLiteral("ulg") || ext == QStringLiteral("bin") || ext == QStringLiteral("log");
}
