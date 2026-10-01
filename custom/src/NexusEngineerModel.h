#pragma once

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

class ParameterManager;
class Vehicle;

class NexusEngineerModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool unlocked READ unlocked NOTIFY engineerChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY engineerChanged)
    Q_PROPERTY(bool safeToWrite READ safeToWrite NOTIFY engineerChanged)
    Q_PROPERTY(int parameterCount READ parameterCount NOTIFY engineerChanged)
    Q_PROPERTY(bool snapshotAValid READ snapshotAValid NOTIFY engineerChanged)
    Q_PROPERTY(bool snapshotBValid READ snapshotBValid NOTIFY engineerChanged)
    Q_PROPERTY(QString snapshotALabel READ snapshotALabel NOTIFY engineerChanged)
    Q_PROPERTY(QString snapshotBLabel READ snapshotBLabel NOTIFY engineerChanged)
    Q_PROPERTY(int snapshotDiffCount READ snapshotDiffCount NOTIFY engineerChanged)
    Q_PROPERTY(QStringList snapshotDiffSummary READ snapshotDiffSummary NOTIFY engineerChanged)

public:
    explicit NexusEngineerModel(QObject *parent = nullptr);

    bool unlocked() const { return _unlocked; }
    bool connected() const { return !_vehicle.isNull(); }
    bool safeToWrite() const;
    int parameterCount() const;

    bool snapshotAValid() const { return !_snapshotA.isEmpty(); }
    bool snapshotBValid() const { return !_snapshotB.isEmpty(); }
    QString snapshotALabel() const { return _snapshotALabel; }
    QString snapshotBLabel() const { return _snapshotBLabel; }
    int snapshotDiffCount() const { return _snapshotDiffCount; }
    QStringList snapshotDiffSummary() const { return _snapshotDiffSummary; }

    Q_INVOKABLE bool unlock(const QString &confirmation);
    Q_INVOKABLE void lock();
    Q_INVOKABLE bool captureSnapshotA(const QString &label = QString());
    Q_INVOKABLE bool captureSnapshotB(const QString &label = QString());
    Q_INVOKABLE void clearSnapshots();
    Q_INVOKABLE void compareSnapshots();

signals:
    void engineerChanged();

private slots:
    void _activeVehicleChanged(Vehicle *vehicle);

private:
    using Snapshot = QMap<QString, QVariant>;
    void _setVehicle(Vehicle *vehicle);
    Snapshot _capture() const;
    static QString _normalizedLabel(const QString &label, const QString &fallback);

    bool _unlocked = false;
    QPointer<Vehicle> _vehicle;
    QPointer<ParameterManager> _parameterManager;
    Snapshot _snapshotA;
    Snapshot _snapshotB;
    QString _snapshotALabel = QStringLiteral("Snapshot A");
    QString _snapshotBLabel = QStringLiteral("Snapshot B");
    int _snapshotDiffCount = 0;
    QStringList _snapshotDiffSummary;
};
