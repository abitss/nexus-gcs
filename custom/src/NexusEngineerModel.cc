#include "NexusEngineerModel.h"

#include <QtCore/QDateTime>
#include <QtCore/QSet>

#include "Fact.h"
#include "MultiVehicleManager.h"
#include "ParameterManager.h"
#include "Vehicle.h"
#include "VehicleLinkManager.h"

NexusEngineerModel::NexusEngineerModel(QObject *parent)
    : QObject(parent)
{
    auto *manager = MultiVehicleManager::instance();
    connect(manager, &MultiVehicleManager::activeVehicleChanged,
            this, &NexusEngineerModel::_activeVehicleChanged);
    _setVehicle(manager->activeVehicle());
}

bool NexusEngineerModel::safeToWrite() const
{
    if (!_unlocked || !_vehicle || !_parameterManager) return false;
    if (_vehicle->armed() || _vehicle->flying()) return false;
    if (_vehicle->vehicleLinkManager() && _vehicle->vehicleLinkManager()->communicationLost()) return false;
    return !_parameterManager->pendingWrites();
}

int NexusEngineerModel::parameterCount() const
{
    if (!_parameterManager) return 0;
    int total = 0;
    for (int componentId : _parameterManager->componentIds()) {
        total += _parameterManager->parameterNames(componentId).size();
    }
    return total;
}

bool NexusEngineerModel::unlock(const QString &confirmation)
{
    const bool accepted = confirmation.trimmed().compare(QStringLiteral("ENGINEER"), Qt::CaseInsensitive) == 0;
    if (_unlocked != accepted) {
        _unlocked = accepted;
        emit engineerChanged();
    }
    return accepted;
}

void NexusEngineerModel::lock()
{
    if (!_unlocked) return;
    _unlocked = false;
    emit engineerChanged();
}

QString NexusEngineerModel::_normalizedLabel(const QString &label, const QString &fallback)
{
    const QString trimmed = label.trimmed();
    if (!trimmed.isEmpty()) return trimmed;
    return QStringLiteral("%1 · %2").arg(fallback, QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")));
}

NexusEngineerModel::Snapshot NexusEngineerModel::_capture() const
{
    Snapshot result;
    if (!_unlocked || !_parameterManager) return result;

    for (int componentId : _parameterManager->componentIds()) {
        const QStringList names = _parameterManager->parameterNames(componentId);
        for (const QString &name : names) {
            Fact *fact = _parameterManager->getParameter(componentId, name);
            if (!fact) continue;
            const QString key = QStringLiteral("%1:%2").arg(componentId).arg(name);
            result.insert(key, fact->rawValue());
        }
    }
    return result;
}

bool NexusEngineerModel::captureSnapshotA(const QString &label)
{
    const Snapshot captured = _capture();
    if (captured.isEmpty()) return false;
    _snapshotA = captured;
    _snapshotALabel = _normalizedLabel(label, QStringLiteral("Snapshot A"));
    compareSnapshots();
    emit engineerChanged();
    return true;
}

bool NexusEngineerModel::captureSnapshotB(const QString &label)
{
    const Snapshot captured = _capture();
    if (captured.isEmpty()) return false;
    _snapshotB = captured;
    _snapshotBLabel = _normalizedLabel(label, QStringLiteral("Snapshot B"));
    compareSnapshots();
    emit engineerChanged();
    return true;
}

void NexusEngineerModel::clearSnapshots()
{
    _snapshotA.clear();
    _snapshotB.clear();
    _snapshotDiffCount = 0;
    _snapshotDiffSummary.clear();
    _snapshotALabel = QStringLiteral("Snapshot A");
    _snapshotBLabel = QStringLiteral("Snapshot B");
    emit engineerChanged();
}

void NexusEngineerModel::compareSnapshots()
{
    _snapshotDiffCount = 0;
    _snapshotDiffSummary.clear();

    if (_snapshotA.isEmpty() || _snapshotB.isEmpty()) {
        emit engineerChanged();
        return;
    }

    QSet<QString> keys;
    for (auto it = _snapshotA.cbegin(); it != _snapshotA.cend(); ++it) keys.insert(it.key());
    for (auto it = _snapshotB.cbegin(); it != _snapshotB.cend(); ++it) keys.insert(it.key());

    QStringList sortedKeys = keys.values();
    sortedKeys.sort(Qt::CaseInsensitive);

    constexpr int kSummaryLimit = 60;
    for (const QString &key : sortedKeys) {
        const bool hasA = _snapshotA.contains(key);
        const bool hasB = _snapshotB.contains(key);
        const QVariant a = _snapshotA.value(key);
        const QVariant b = _snapshotB.value(key);

        if (hasA && hasB && a == b) continue;

        ++_snapshotDiffCount;
        if (_snapshotDiffSummary.size() < kSummaryLimit) {
            if (!hasA) {
                _snapshotDiffSummary.append(QStringLiteral("%1 · added · %2").arg(key, b.toString()));
            } else if (!hasB) {
                _snapshotDiffSummary.append(QStringLiteral("%1 · removed · %2").arg(key, a.toString()));
            } else {
                _snapshotDiffSummary.append(QStringLiteral("%1 · %2 → %3").arg(key, a.toString(), b.toString()));
            }
        }
    }

    if (_snapshotDiffCount > kSummaryLimit) {
        _snapshotDiffSummary.append(QStringLiteral("… %1 additional difference(s)").arg(_snapshotDiffCount - kSummaryLimit));
    }

    emit engineerChanged();
}

void NexusEngineerModel::_activeVehicleChanged(Vehicle *vehicle)
{
    _setVehicle(vehicle);
}

void NexusEngineerModel::_setVehicle(Vehicle *vehicle)
{
    _vehicle = vehicle;
    _parameterManager = vehicle ? vehicle->parameterManager() : nullptr;

    // Engineering access and snapshots are intentionally session/vehicle scoped.
    _unlocked = false;
    clearSnapshots();
    emit engineerChanged();
}
