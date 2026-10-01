#include "NexusPlanVerifier.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QJsonDocument>
#include <QtCore/QTimer>
#include <QtCore/QVariantMap>

#include "MissionController.h"
#include "PlanMasterController.h"
#include "VisualMissionItem.h"

NexusPlanVerifier::NexusPlanVerifier(QObject *parent)
    : QObject(parent)
{
}

bool NexusPlanVerifier::busy() const
{
    return _stage == Stage::Uploading || _stage == Stage::Downloading;
}

QVariantMap NexusPlanVerifier::validationStatus(PlanMasterController *controller) const
{
    QVariantMap result;
    result.insert(QStringLiteral("ready"), false);
    result.insert(QStringLiteral("state"), QStringLiteral("INVALID"));
    result.insert(QStringLiteral("message"), QStringLiteral("Plan controller unavailable."));

    if (!controller) {
        return result;
    }

    MissionController *const mission = controller->missionController();
    if (!mission) {
        return result;
    }

    if (!controller->containsItems() || mission->visualItems()->count() <= 1) {
        result[QStringLiteral("state")] = QStringLiteral("EMPTY");
        result[QStringLiteral("message")] = QStringLiteral("Add at least one mission item.");
        return result;
    }

    if (!mission->plannedHomePosition().isValid()) {
        result[QStringLiteral("state")] = QStringLiteral("HOME REQUIRED");
        result[QStringLiteral("message")] = QStringLiteral("Set or obtain a valid planned Home position.");
        return result;
    }

    switch (controller->readyForSaveState()) {
    case VisualMissionItem::NotReadyForSaveTerrain:
        result[QStringLiteral("state")] = QStringLiteral("WAITING TERRAIN");
        result[QStringLiteral("message")] = QStringLiteral("Terrain-dependent altitude data is not ready.");
        return result;
    case VisualMissionItem::NotReadyForSaveData:
        result[QStringLiteral("state")] = QStringLiteral("INCOMPLETE");
        result[QStringLiteral("message")] = QStringLiteral("One or more mission items are incomplete.");
        return result;
    case VisualMissionItem::ReadyForSave:
        break;
    default:
        result[QStringLiteral("state")] = QStringLiteral("INVALID");
        result[QStringLiteral("message")] = QStringLiteral("Plan readiness state is unknown.");
        return result;
    }

    if (controller->syncInProgress()) {
        result[QStringLiteral("state")] = QStringLiteral("SYNCING");
        result[QStringLiteral("message")] = QStringLiteral("Mission synchronization is already in progress.");
        return result;
    }

    if (controller->offline()) {
        result[QStringLiteral("ready")] = true;
        result[QStringLiteral("state")] = QStringLiteral("OFFLINE READY");
        result[QStringLiteral("message")] = QStringLiteral("Plan is valid for local editing; connect a vehicle to upload.");
        return result;
    }

    switch (mission->sendToVehiclePreCheck()) {
    case MissionController::SendToVehiclePreCheckStateOk:
        result[QStringLiteral("ready")] = true;
        result[QStringLiteral("state")] = QStringLiteral("MISSION READY");
        result[QStringLiteral("message")] = QStringLiteral("Plan is valid and ready for upload verification.");
        break;
    case MissionController::SendToVehiclePreCheckStateNoActiveVehicle:
        result[QStringLiteral("state")] = QStringLiteral("NO VEHICLE");
        result[QStringLiteral("message")] = QStringLiteral("No active vehicle is available for upload.");
        break;
    case MissionController::SendToVehiclePreCheckStateFirwmareVehicleMismatch:
        result[QStringLiteral("state")] = QStringLiteral("VEHICLE MISMATCH");
        result[QStringLiteral("message")] = QStringLiteral("Plan firmware/vehicle type does not match the active vehicle.");
        break;
    case MissionController::SendToVehiclePreCheckStateActiveMission:
        result[QStringLiteral("state")] = QStringLiteral("MISSION ACTIVE");
        result[QStringLiteral("message")] = QStringLiteral("Pause the active mission before replacing the plan.");
        break;
    }

    return result;
}

bool NexusPlanVerifier::verifyUpload(PlanMasterController *controller)
{
    if (busy()) {
        return false;
    }

    const QVariantMap status = validationStatus(controller);
    if (!status.value(QStringLiteral("ready")).toBool() || !controller || controller->offline()) {
        _fail(status.value(QStringLiteral("message")).toString());
        return false;
    }

    _controller = controller;
    _expectedPlan = _comparablePlan(controller->saveToJson().object());
    _fingerprint = _fingerprintForPlan(_expectedPlan);
    _sawBusy = false;

    connect(_controller, &PlanMasterController::syncInProgressChanged,
            this, &NexusPlanVerifier::_syncChanged, Qt::UniqueConnection);
    connect(_controller, &QObject::destroyed,
            this, &NexusPlanVerifier::_controllerDestroyed, Qt::UniqueConnection);

    _setStage(Stage::Uploading, QStringLiteral("UPLOADING"),
              QStringLiteral("Sending mission, geofence and rally data to the vehicle."));
    controller->sendToVehicle();

    // Defensive check for an operation that was refused before sync ever started.
    QTimer::singleShot(1500, this, [this]() {
        if (_stage == Stage::Uploading && !_sawBusy && _controller && !_controller->syncInProgress()) {
            _fail(QStringLiteral("Upload did not start. Check vehicle/link state."));
        }
    });

    return true;
}

void NexusPlanVerifier::_syncChanged()
{
    if (!_controller) {
        return;
    }

    if (_controller->syncInProgress()) {
        _sawBusy = true;
        return;
    }

    if (!_sawBusy) {
        return;
    }

    if (_stage == Stage::Uploading) {
        if (_controller->dirtyForUpload()) {
            _fail(QStringLiteral("Vehicle upload did not complete successfully."));
            return;
        }

        _sawBusy = false;
        _setStage(Stage::Downloading, QStringLiteral("READBACK"),
                  QStringLiteral("Upload completed. Downloading the vehicle copy for comparison."));
        QTimer::singleShot(0, _controller, [controller = QPointer<PlanMasterController>(_controller)]() {
            if (controller) {
                controller->loadFromVehicle();
            }
        });
        return;
    }

    if (_stage == Stage::Downloading) {
        const QJsonObject actualPlan = _comparablePlan(_controller->saveToJson().object());
        const QString actualFingerprint = _fingerprintForPlan(actualPlan);

        if (actualPlan == _expectedPlan) {
            _fingerprint = actualFingerprint;
            _setStage(Stage::Verified, QStringLiteral("MISSION READY"),
                      QStringLiteral("Vehicle readback matches the uploaded plan."));
        } else {
            _fingerprint = actualFingerprint;
            _setStage(Stage::Mismatch, QStringLiteral("READBACK MISMATCH"),
                      QStringLiteral("Vehicle readback differs from the uploaded plan. Do not treat the mission as verified."));
        }
    }
}

void NexusPlanVerifier::reset()
{
    if (_controller) {
        disconnect(_controller, nullptr, this, nullptr);
    }
    _controller.clear();
    _expectedPlan = QJsonObject();
    _fingerprint.clear();
    _sawBusy = false;
    _setStage(Stage::Idle, QStringLiteral("NOT VERIFIED"),
              QStringLiteral("Upload and read back the mission to verify vehicle integrity."));
}

void NexusPlanVerifier::_controllerDestroyed()
{
    _controller.clear();
    _fail(QStringLiteral("Plan controller closed during verification."));
}

void NexusPlanVerifier::_setStage(Stage stage, const QString &state, const QString &message)
{
    _stage = stage;
    _state = state;
    _message = message;
    emit stateChanged();
}

void NexusPlanVerifier::_fail(const QString &message)
{
    _sawBusy = false;
    _setStage(Stage::Failed, QStringLiteral("VERIFY FAILED"), message);
}

QString NexusPlanVerifier::_fingerprintForPlan(const QJsonObject &plan)
{
    const QByteArray compact = QJsonDocument(plan).toJson(QJsonDocument::Compact);
    return QString::fromLatin1(QCryptographicHash::hash(compact, QCryptographicHash::Sha256).toHex().left(12)).toUpper();
}

QJsonObject NexusPlanVerifier::_comparablePlan(const QJsonObject &plan)
{
    // Compare exactly the flight-critical plan payload. Top-level file metadata
    // can legitimately change across save/load cycles and is not part of the
    // vehicle mission integrity decision.
    QJsonObject comparable;
    comparable.insert(QStringLiteral("mission"), plan.value(QStringLiteral("mission")));
    comparable.insert(QStringLiteral("geoFence"), plan.value(QStringLiteral("geoFence")));
    comparable.insert(QStringLiteral("rallyPoints"), plan.value(QStringLiteral("rallyPoints")));
    return comparable;
}
