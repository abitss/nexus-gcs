#include "NexusPX4PlanSITLTest.h"

#include <QtCore/QPointer>
#include <QtCore/QScopeGuard>
#include <QtPositioning/QGeoCoordinate>
#include <QtTest/QTest>

#include "LinkManager.h"
#include "MissionController.h"
#include "MultiVehicleManager.h"
#include "NexusPlanVerifier.h"
#include "PlanMasterController.h"
#include "SimpleMissionItem.h"
#include "UDPLink.h"
#include "Vehicle.h"

UT_REGISTER_TEST_STANDALONE(NexusPX4PlanSITLTest,
                            TestLabel::Integration,
                            TestLabel::MissionManager,
                            TestLabel::Vehicle,
                            TestLabel::Comms,
                            TestLabel::Slow)

void NexusPX4PlanSITLTest::_testMissionUploadReadback()
{
    LinkManager* const linkManager = LinkManager::instance();
    MultiVehicleManager* const vehicleManager = MultiVehicleManager::instance();
    QVERIFY(linkManager);
    QVERIFY(vehicleManager);

    linkManager->setConnectionsAllowed();
    linkManager->disconnectAll();

    auto* udp = new UDPConfiguration(QStringLiteral("NEXUS PX4 PLAN SITL"));
    udp->setLocalPort(14550);
    udp->setAutoConnect(false);
    SharedLinkConfigurationPtr config = linkManager->addConfiguration(udp);
    QVERIFY(config);
    QVERIFY2(linkManager->createConnectedLink(config), "Failed to bind PX4 plan-validation UDP link");

    const auto cleanup = qScopeGuard([linkManager] {
        linkManager->disconnectAll();
        QTest::qWait(250);
    });

    QVERIFY2(UnitTest::waitForCondition(
                 [vehicleManager] { return vehicleManager->activeVehicle() != nullptr; },
                 60000, QStringLiteral("PX4 plan SITL discovery")),
             "PX4 SITL was not discovered");

    QPointer<Vehicle> vehicle = vehicleManager->activeVehicle();
    QVERIFY(vehicle);
    QCOMPARE(vehicle->firmwareType(), MAV_AUTOPILOT_PX4);
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->isInitialConnectComplete(); },
                 90000, QStringLiteral("PX4 initial plan synchronization")),
             "PX4 initial plan synchronization did not complete");

    PlanMasterController plan;
    plan.setFlyView(false);
    plan.start();
    plan.startStaticActiveVehicle(vehicle);
    MissionController* const mission = plan.missionController();
    QVERIFY(mission);

    QGeoCoordinate home = vehicle->homePosition();
    if (!home.isValid()) {
        home = QGeoCoordinate(47.397742, 8.545594, 488.0);
    }
    mission->setHomePosition(home);

    auto* wp1 = qobject_cast<SimpleMissionItem*>(
        mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(80.0, 15.0), -1, true));
    auto* wp2 = qobject_cast<SimpleMissionItem*>(
        mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(110.0, 100.0), -1, true));
    auto* wp3 = qobject_cast<SimpleMissionItem*>(
        mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(75.0, 220.0), -1, true));
    QVERIFY(wp1);
    QVERIFY(wp2);
    QVERIFY(wp3);

    wp1->altitude()->setRawValue(30.0);
    wp2->altitude()->setRawValue(35.0);
    wp3->altitude()->setRawValue(30.0);

    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return mission->missionTotalDistance() > 0.0; },
                 5000, QStringLiteral("mission distance calculation")),
             "Mission distance did not calculate");

    NexusPlanVerifier verifier;
    const QVariantMap status = verifier.validationStatus(&plan);
    QVERIFY2(status.value(QStringLiteral("ready")).toBool(),
             qPrintable(status.value(QStringLiteral("message")).toString()));

    QVERIFY(verifier.verifyUpload(&plan));
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return verifier.verified() ||
                            verifier.state() == QStringLiteral("VERIFY FAILED") ||
                            verifier.state() == QStringLiteral("READBACK MISMATCH");
                 },
                 120000, QStringLiteral("PX4 mission upload/readback verification")),
             "PX4 mission verification timed out");

    QVERIFY2(verifier.verified(), qPrintable(verifier.message()));
    QVERIFY(!verifier.fingerprint().isEmpty());
    QVERIFY(!plan.dirtyForUpload());
    QCOMPARE(mission->visualItems()->count(), 4);
}
