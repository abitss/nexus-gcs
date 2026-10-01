#pragma once

#include <QtCore/QFileInfo>
#include <QtCore/QObject>
#include <QtCore/QVariantList>

class NexusAnalyzeModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList flightHistory READ flightHistory NOTIFY analyzeChanged)
    Q_PROPERTY(QString selectedPath READ selectedPath NOTIFY analyzeChanged)
    Q_PROPERTY(QString selectedName READ selectedName NOTIFY analyzeChanged)
    Q_PROPERTY(QString selectedType READ selectedType NOTIFY analyzeChanged)
    Q_PROPERTY(QString selectedDate READ selectedDate NOTIFY analyzeChanged)
    Q_PROPERTY(QString selectedSize READ selectedSize NOTIFY analyzeChanged)
    Q_PROPERTY(bool selectedReplayOnly READ selectedReplayOnly NOTIFY analyzeChanged)
    Q_PROPERTY(bool selectedFirmwareLog READ selectedFirmwareLog NOTIFY analyzeChanged)
    Q_PROPERTY(int flightCount READ flightCount NOTIFY analyzeChanged)

public:
    explicit NexusAnalyzeModel(QObject *parent = nullptr);

    QVariantList flightHistory() const { return _history; }
    QString selectedPath() const { return _selectedPath; }
    QString selectedName() const;
    QString selectedType() const;
    QString selectedDate() const;
    QString selectedSize() const;
    bool selectedReplayOnly() const;
    bool selectedFirmwareLog() const;
    int flightCount() const { return _history.size(); }

    Q_INVOKABLE void refreshHistory();
    Q_INVOKABLE bool selectFlight(const QString &path);
    Q_INVOKABLE void clearSelection();

signals:
    void analyzeChanged();

private:
    static QString _typeForSuffix(const QString &suffix);
    static bool _supportedSuffix(const QString &suffix);
    static QVariantMap _entryForFile(const QFileInfo &fileInfo);

    QVariantList _history;
    QString _selectedPath;
};
