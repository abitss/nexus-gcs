#include "NexusPX4QualificationTest.h"

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QScopeGuard>
#include <QtCore/QTemporaryDir>
#include <QtPositioning/QGeoCoordinate>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "HealthAndArmingCheckReport.h"
#include "LinkManager.h"
#include "MissionController.h"
#include "MissionManager.h"
#include "MultiVehicleManager.h"
#include "NexusPlanVerifier.h"
#include "NexusRecoveryModel.h"
#include "NexusSecurityModel.h"
#include "PlanMasterController.h"
#include "SimpleMissionItem.h"
#include "UDPLink.h"
#include "Vehicle.h"

UT_REGISTER_TEST_STANDALONE(NexusPX4QualificationTest,
                            TestLabel::Integration,
                            TestLabel::MissionManager,
                            TestLabel::Vehicle,
                            TestLabel::Comms,
                            TestLabel::Slow)

bool NexusPX4QualificationTest::_acceptedAckSince(const QSignalSpy &spy, int command, int startIndex)
{
    for (int i = startIndex; i < spy.count(); ++i) {
        const QList<QVariant> args = spy.at(i);
        if (args.size() >= 5 &&
            args.at(2).toInt() == command &&
            args.at(3).toInt() == MAV_RESULT_ACCEPTED) {
            return true;
        }
    }
    return false;
}

void NexusPX4QualificationTest::_writeEvidence(const QJsonArray &stages)
{
    const QString path = qEnvironmentVariable(
        "NEXUS_PX4_QUALIFICATION_EVIDENCE",
        QStringLiteral("/tmp/nexus-px4-qualification.json"));

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    QJsonObject root;
    root.insert(QStringLiteral("schemaVersion"), QStringLiteral("1.0"));
    root.insert(QStringLiteral("suite"), QStringLiteral("NEXUS PX4 FULL LIFECYCLE"));
    root.insert(QStringLiteral("stages"), stages);
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

void NexusPX4QualificationTest::_testFullPX4Qualification()
{
    QJsonArray evidence;
    auto stage = [&evidence](const QString &name, const QString &status, const QString &detail = QString()) {
        QJsonObject row{
            {QStringLiteral("stage"), name},
            {QStringLiteral("status"), status}
        };
        if (!detail.isEmpty()) {
            row.insert(QStringLiteral("detail"), detail);
        }
        evidence.append(row);
        _writeEvidence(evidence);
    };

    LinkManager *const linkManager = LinkManager::instance();
    MultiVehicleManager *const vehicleManager = MultiVehicleManager::instance();
    QVERIFY(linkManager);
    QVERIFY(vehicleManager);

    linkManager->setConnectionsAllowed();
    linkManager->disconnectAll();

    auto *udp = new UDPConfiguration(QStringLiteral("NEXUS PX4 QUALIFICATION"));
    udp->setLocalPort(14550);
    udp->setAutoConnect(false);
    SharedLinkConfigurationPtr config = linkManager->addConfiguration(udp);
    QVERIFY(config);
    QVERIFY2(linkManager->createConnectedLink(config), "Unable to open PX4 qualification UDP link");
    stage(QStringLiteral("CONNECT_REQUEST"), QStringLiteral("PASS"));

    const auto cleanup = qScopeGuard([&] {
        _writeEvidence(evidence);
        linkManager->disconnectAll();
        QTest::qWait(250);
    });

    QVERIFY2(UnitTest::waitForCondition(
                 [vehicleManager] { return vehicleManager->activeVehicle() != nullptr; },
                 60000, QStringLiteral("PX4 qualification discovery")),
             "No PX4 heartbeat reached NEXUS");
    QPointer<Vehicle> vehicle = vehicleManager->activeVehicle();
    QVERIFY(vehicle);
    QCOMPARE(vehicle->firmwareType(), MAV_AUTOPILOT_PX4);
    stage(QStringLiteral("CONNECT"), QStringLiteral("PASS"),
          QStringLiteral("PX4 vehicle discovered on UDP 14550"));

    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->isInitialConnectComplete(); },
                 90000, QStringLiteral("PX4 full initial synchronization")),
             "PX4 initial synchronization did not complete");
    stage(QStringLiteral("INITIAL_SYNC"), QStringLiteral("PASS"));

    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle &&
                            vehicle->coordinate().isValid() &&
                            vehicle->homePosition().isValid();
                 },
                 60000, QStringLiteral("valid PX4 position/home")),
             "PX4 position/home never became valid");

    HealthAndArmingCheckReport *health = vehicle->healthAndArmingCheckReport();
    QVERIFY(health);
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle && health &&
                            (!health->supported() || health->canArm());
                 },
                 60000, QStringLiteral("PX4 preflight armability")),
             "PX4 preflight never reached an armable state");
    stage(QStringLiteral("PREFLIGHT"), QStringLiteral("PASS"),
          health->supported() ? QStringLiteral("Health report canArm=true")
                              : QStringLiteral("Legacy PX4 preflight path available"));

    // Build a real multi-waypoint mission.
    PlanMasterController plan;
    plan.setFlyView(false);
    plan.start();
    plan.startStaticActiveVehicle(vehicle);

    MissionController *const mission = plan.missionController();
    QVERIFY(mission);

    QGeoCoordinate home = vehicle->homePosition();
    QVERIFY(home.isValid());
    mission->setHomePosition(home);

    auto *wp1 = qobject_cast<SimpleMissionItem *>(
        mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(140.0, 20.0), -1, true));
    auto *wp2 = qobject_cast<SimpleMissionItem *>(
        mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(190.0, 115.0), -1, true));
    auto *wp3 = qobject_cast<SimpleMissionItem *>(
        mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(150.0, 225.0), -1, true));
    QVERIFY(wp1);
    QVERIFY(wp2);
    QVERIFY(wp3);

    wp1->altitude()->setRawValue(20.0);
    wp2->altitude()->setRawValue(25.0);
    wp3->altitude()->setRawValue(20.0);

    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return mission->missionTotalDistance() > 0.0; },
                 5000, QStringLiteral("qualification mission geometry")),
             "Qualification mission distance did not calculate");

    NexusPlanVerifier verifier;
    const QVariantMap validation = verifier.validationStatus(&plan);
    QVERIFY2(validation.value(QStringLiteral("ready")).toBool(),
             qPrintable(validation.value(QStringLiteral("message")).toString()));
    stage(QStringLiteral("MISSION_VALIDATE"), QStringLiteral("PASS"));

    QVERIFY(verifier.verifyUpload(&plan));
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return verifier.verified() ||
                            verifier.state() == QStringLiteral("VERIFY FAILED") ||
                            verifier.state() == QStringLiteral("READBACK MISMATCH");
                 },
                 120000, QStringLiteral("mission upload/readback verification")),
             "Qualification mission upload/readback timed out");
    QVERIFY2(verifier.verified(), qPrintable(verifier.message()));
    stage(QStringLiteral("MISSION_UPLOAD_READBACK"), QStringLiteral("PASS"),
          verifier.fingerprint());

    QSignalSpy ackSpy(vehicle, &Vehicle::mavCommandResult);
    QVERIFY(ackSpy.isValid());

    // Arm.
    const int armStart = ackSpy.count();
    vehicle->setArmedShowError(true);
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->armed(); },
                 30000, QStringLiteral("PX4 qualification arm")),
             "PX4 failed to arm");
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return _acceptedAckSince(ackSpy, MAV_CMD_COMPONENT_ARM_DISARM, armStart); },
                 30000, QStringLiteral("PX4 arm ACK")),
             "PX4 did not return accepted ARM ACK");
    stage(QStringLiteral("ARM"), QStringLiteral("PASS"));

    // Take off.
    const int takeoffStart = ackSpy.count();
    vehicle->guidedModeTakeoff(10.0);
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return _acceptedAckSince(ackSpy, MAV_CMD_NAV_TAKEOFF, takeoffStart); },
                 30000, QStringLiteral("PX4 takeoff ACK")),
             "PX4 did not accept takeoff");
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle && vehicle->flying() &&
                            vehicle->altitudeRelative()->rawValue().toDouble() > 5.0;
                 },
                 60000, QStringLiteral("PX4 airborne qualification")),
             "PX4 failed to become airborne");
    stage(QStringLiteral("TAKEOFF"), QStringLiteral("PASS"));

    // Start mission and prove waypoint progression from MISSION_CURRENT.
    vehicle->startMission();
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle &&
                            vehicle->flightMode() == vehicle->missionFlightMode();
                 },
                 30000, QStringLiteral("PX4 AUTO MISSION mode")),
             "PX4 did not enter mission flight mode");
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle && vehicle->missionManager() &&
                            vehicle->missionManager()->currentIndex() >= 1;
                 },
                 60000, QStringLiteral("PX4 first mission waypoint")),
             "PX4 mission did not report waypoint progression");
    stage(QStringLiteral("WAYPOINT_MISSION"), QStringLiteral("PASS"),
          QStringLiteral("MISSION_CURRENT advanced"));

    // Let the mission advance, then HOLD.
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle && vehicle->missionManager() &&
                            vehicle->missionManager()->currentIndex() >= 2;
                 },
                 120000, QStringLiteral("PX4 mission second item")),
             "PX4 mission did not advance far enough for hold/continue qualification");

    const int heldIndex = vehicle->missionManager()->currentIndex();
    vehicle->pauseVehicle();
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle &&
                            vehicle->flightMode() == vehicle->pauseFlightMode();
                 },
                 30000, QStringLiteral("PX4 hold mode")),
             "PX4 did not enter HOLD/LOITER");
    stage(QStringLiteral("HOLD"), QStringLiteral("PASS"),
          QStringLiteral("Held at mission index %1").arg(heldIndex));

    // Continue by returning to mission mode. Verify mission state remains valid
    // and that MISSION_CURRENT does not regress behind the held point.
    vehicle->startMission();
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle &&
                            vehicle->flightMode() == vehicle->missionFlightMode();
                 },
                 30000, QStringLiteral("PX4 continue mission mode")),
             "PX4 did not continue mission");
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle && vehicle->missionManager() &&
                            vehicle->missionManager()->currentIndex() >= heldIndex;
                 },
                 30000, QStringLiteral("PX4 continue mission index")),
             "PX4 mission index regressed after continue");
    stage(QStringLiteral("CONTINUE"), QStringLiteral("PASS"));

    // RTL and authoritative returned mode.
    vehicle->guidedModeRTL(false);
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle &&
                            vehicle->flightMode() == vehicle->rtlFlightMode();
                 },
                 30000, QStringLiteral("PX4 RTL mode")),
             "PX4 did not enter RTL");
    stage(QStringLiteral("RTL"), QStringLiteral("PASS"));

    // Land explicitly and verify landed/disarmed state.
    vehicle->guidedModeLand();
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle &&
                            vehicle->flightMode() == vehicle->landFlightMode();
                 },
                 30000, QStringLiteral("PX4 LAND mode")),
             "PX4 did not enter LAND");
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && !vehicle->flying(); },
                 120000, QStringLiteral("PX4 landed state")),
             "PX4 never reported landed");
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && !vehicle->armed(); },
                 60000, QStringLiteral("PX4 post-land disarm")),
             "PX4 did not disarm after landing");
    stage(QStringLiteral("LAND"), QStringLiteral("PASS"));

    // Explicit disconnect/reconnect and fresh synchronization.
    const QList<SharedLinkInterfacePtr> links = linkManager->links();
    QVERIFY(!links.isEmpty());
    linkManager->disconnectLink(links.first().get());
    QVERIFY2(UnitTest::waitForCondition(
                 [vehicleManager] { return vehicleManager->activeVehicle() == nullptr; },
                 30000, QStringLiteral("PX4 disconnect removal")),
             "PX4 vehicle remained active after link disconnect");
    stage(QStringLiteral("DISCONNECT"), QStringLiteral("PASS"));

    QVERIFY(linkManager->createConnectedLink(config));
    QVERIFY2(UnitTest::waitForCondition(
                 [vehicleManager] { return vehicleManager->activeVehicle() != nullptr; },
                 60000, QStringLiteral("PX4 reconnect discovery")),
             "PX4 did not rediscover after UDP reconnect");

    vehicle = vehicleManager->activeVehicle();
    QVERIFY(vehicle);
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->isInitialConnectComplete(); },
                 90000, QStringLiteral("PX4 reconnect fresh sync")),
             "PX4 reconnect did not complete fresh synchronization");
    stage(QStringLiteral("RECONNECT"), QStringLiteral("PASS"),
          QStringLiteral("Fresh initial-connect synchronization completed"));

    // Failure test 1: corrupted mission is rejected before operational trust.
    NexusSecurityModel security;
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString corruptMission = tmp.filePath(QStringLiteral("corrupt.plan"));
    QFile corrupt(corruptMission);
    QVERIFY(corrupt.open(QIODevice::WriteOnly | QIODevice::Truncate));
    corrupt.write("{\"fileType\":\"Plan\",\"version\":1,\"mission\":");
    corrupt.close();
    QVERIFY(!security.validateMissionFile(corruptMission));
    stage(QStringLiteral("FAILURE_CORRUPT_MISSION"), QStringLiteral("PASS"),
          QStringLiteral("Malformed plan rejected"));

    // Failure test 2: interrupt a real upload/readback by dropping UDP.
    PlanMasterController failurePlan;
    failurePlan.setFlyView(false);
    failurePlan.start();
    failurePlan.startStaticActiveVehicle(vehicle);
    MissionController *failureMission = failurePlan.missionController();
    QVERIFY(failureMission);
    const QGeoCoordinate failureHome = vehicle->homePosition();
    QVERIFY(failureHome.isValid());
    failureMission->setHomePosition(failureHome);

    auto *fwp1 = qobject_cast<SimpleMissionItem *>(
        failureMission->insertSimpleMissionItem(failureHome.atDistanceAndAzimuth(90.0, 45.0), -1, true));
    auto *fwp2 = qobject_cast<SimpleMissionItem *>(
        failureMission->insertSimpleMissionItem(failureHome.atDistanceAndAzimuth(120.0, 160.0), -1, true));
    QVERIFY(fwp1);
    QVERIFY(fwp2);
    fwp1->altitude()->setRawValue(20.0);
    fwp2->altitude()->setRawValue(20.0);

    NexusPlanVerifier interruptedVerifier;
    NexusRecoveryModel recovery(nullptr, nullptr, &interruptedVerifier, &security);
    QVERIFY(interruptedVerifier.verifyUpload(&failurePlan));
    QVERIFY(interruptedVerifier.busy());

    const QList<SharedLinkInterfacePtr> failureLinks = linkManager->links();
    QVERIFY(!failureLinks.isEmpty());
    linkManager->disconnectLink(failureLinks.first().get());

    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return recovery.interruptedMission() ||
                            (!interruptedVerifier.busy() && !interruptedVerifier.verified());
                 },
                 30000, QStringLiteral("interrupted mission invalidation")),
             "Interrupted mission transfer remained trusted");
    QVERIFY(!interruptedVerifier.verified());
    stage(QStringLiteral("FAILURE_INTERRUPTED_MISSION"), QStringLiteral("PASS"),
          QStringLiteral("Mission trust invalidated on link interruption"));

    // Reconnect once more and prove the application can recover to a live PX4 session.
    QVERIFY(linkManager->createConnectedLink(config));
    QVERIFY2(UnitTest::waitForCondition(
                 [vehicleManager] { return vehicleManager->activeVehicle() != nullptr; },
                 60000, QStringLiteral("post-failure reconnect discovery")),
             "PX4 did not recover after injected telemetry interruption");
    QPointer<Vehicle> recovered = vehicleManager->activeVehicle();
    QVERIFY(recovered);
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return recovered && recovered->isInitialConnectComplete(); },
                 90000, QStringLiteral("post-failure reconnect sync")),
             "Fresh PX4 synchronization failed after injected failure");
    stage(QStringLiteral("FAILURE_RECOVERY"), QStringLiteral("PASS"));

    stage(QStringLiteral("QUALIFICATION"), QStringLiteral("PASS"),
          QStringLiteral("Full PX4 lifecycle + failure recovery completed"));
}
