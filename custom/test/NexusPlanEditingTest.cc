#include "NexusPlanEditingTest.h"

#include <QtPositioning/QGeoCoordinate>
#include <QtTest/QTest>

#include "ComplexMissionItem.h"
#include "GeoFenceController.h"
#include "MissionController.h"
#include "NexusPlanVerifier.h"
#include "PlanMasterController.h"
#include "QmlObjectListModel.h"
#include "RallyPointController.h"
#include "SimpleMissionItem.h"
#include "SpeedSection.h"
#include "VisualMissionItem.h"

UT_REGISTER_TEST(NexusPlanEditingTest, TestLabel::Integration, TestLabel::MissionManager)
UT_REGISTER_TEST(NexusPlanVerificationTest, TestLabel::Integration, TestLabel::MissionManager, TestLabel::Vehicle)

void NexusPlanEditingTest::_testCoreEditingAndPatrolRoute()
{
    MissionController* const mission = missionController();
    QVERIFY(mission);
    QVERIFY(planController());
    QVERIFY(planController()->offline());

    QGeoCoordinate home(30.9686, 76.4736, 300.0);
    mission->setHomePosition(home);
    QVERIFY(mission->plannedHomePosition().isValid());

    // Create a simple patrol-style waypoint route.
    const QList<QGeoCoordinate> route = {
        home.atDistanceAndAzimuth(80.0, 0.0),
        home.atDistanceAndAzimuth(100.0, 90.0),
        home.atDistanceAndAzimuth(80.0, 180.0),
        home.atDistanceAndAzimuth(100.0, 270.0),
    };

    for (const QGeoCoordinate& coordinate : route) {
        auto* item = qobject_cast<SimpleMissionItem*>(mission->insertSimpleMissionItem(coordinate, -1, true));
        QVERIFY(item);
    }

    QCOMPARE(mission->visualItems()->count(), 5); // Home/settings + four route items.
    QTRY_VERIFY_WITH_TIMEOUT(mission->missionTotalDistance() > 0.0, TestTimeout::mediumMs());

    auto* first = qobject_cast<SimpleMissionItem*>(mission->visualItems()->get(1));
    QVERIFY(first);

    // Altitude.
    first->altitude()->setRawValue(45.0);
    QCOMPARE(first->altitude()->rawValue().toDouble(), 45.0);

    // Speed.
    QVERIFY(first->speedSection());
    first->speedSection()->setSpecifyFlightSpeed(true);
    first->speedSection()->flightSpeed()->setRawValue(6.5);
    QCOMPARE(first->specifiedFlightSpeed(), 6.5);

    // Move waypoint.
    const QGeoCoordinate moved = first->coordinate().atDistanceAndAzimuth(20.0, 45.0);
    first->setCoordinate(moved);
    QVERIFY(first->coordinate().distanceTo(moved) < 0.5);

    // Delete waypoint.
    const int beforeDelete = mission->visualItems()->count();
    mission->removeVisualItem(2);
    QCOMPARE(mission->visualItems()->count(), beforeDelete - 1);

    // Offline plan remains locally editable and valid without a network/cloud dependency.
    const QVariantMap status = NexusPlanVerifier().validationStatus(planController());
    QVERIFY(status.value(QStringLiteral("ready")).toBool());
    QCOMPARE(status.value(QStringLiteral("state")).toString(), QStringLiteral("OFFLINE READY"));
}

void NexusPlanEditingTest::_testCorridorOrbitFenceRallyAndTerrain()
{
    MissionController* const mission = missionController();
    QVERIFY(mission);

    QGeoCoordinate home(30.9686, 76.4736, 300.0);
    mission->setHomePosition(home);

    // Orbit/loiter is represented by the standard MAVLink loiter command.
    auto* orbit = qobject_cast<SimpleMissionItem*>(
        mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(60.0, 30.0), -1, true));
    QVERIFY(orbit);
    orbit->setCommand(MAV_CMD_NAV_LOITER_TIME);
    orbit->missionItem().setParam1(20.0);
    QCOMPARE(orbit->command(), static_cast<int>(MAV_CMD_NAV_LOITER_TIME));
    QVERIFY(orbit->isLoiterItem());

    // Terrain-aware altitude mode uses QGC's standard terrain machinery.
    orbit->setAltitudeFrame(QGroundControlQmlGlobal::AltitudeFrameTerrain);
    QVERIFY(orbit->terrainAltitudeRequiredInFlyView());

    // Corridor route uses QGC's mature Corridor Scan complex item.
    VisualMissionItem* corridor = mission->insertComplexMissionItem(
        QStringLiteral("Corridor Scan"),
        home.atDistanceAndAzimuth(120.0, 45.0),
        -1,
        true);
    QVERIFY2(corridor, "Corridor Scan complex item could not be inserted");

    // Geofence basics.
    GeoFenceController* const fence = geoFenceController();
    QVERIFY(fence);
    const QGeoCoordinate topLeft = home.atDistanceAndAzimuth(150.0, 315.0);
    const QGeoCoordinate bottomRight = home.atDistanceAndAzimuth(150.0, 135.0);
    fence->addInclusionCircle(topLeft, bottomRight);
    fence->addInclusionPolygon(topLeft, bottomRight);
    QCOMPARE(fence->circles()->count(), 1);
    QCOMPARE(fence->polygons()->count(), 1);

    // Rally point editing is available in the plan data model. Vehicle capability
    // determines whether it can be uploaded to a specific autopilot.
    RallyPointController* const rally = rallyPointController();
    QVERIFY(rally);
    rally->addPoint(home.atDistanceAndAzimuth(40.0, 225.0));
    QCOMPARE(rally->points()->count(), 1);
}

void NexusPlanVerificationTest::_testUploadDownloadReadbackVerification()
{
    PlanMasterController* const plan = planController();
    MissionController* const mission = missionController();
    QVERIFY(plan);
    QVERIFY(mission);
    QVERIFY(!plan->offline());

    const QGeoCoordinate home(47.397742, 8.545594, 488.0);
    mission->setHomePosition(home);

    auto* wp1 = qobject_cast<SimpleMissionItem*>(
        mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(80.0, 0.0), -1, true));
    auto* wp2 = qobject_cast<SimpleMissionItem*>(
        mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(100.0, 90.0), -1, true));
    QVERIFY(wp1);
    QVERIFY(wp2);
    wp1->altitude()->setRawValue(35.0);
    wp2->altitude()->setRawValue(40.0);

    NexusPlanVerifier verifier;
    const QVariantMap status = verifier.validationStatus(plan);
    QVERIFY2(status.value(QStringLiteral("ready")).toBool(), qPrintable(status.value(QStringLiteral("message")).toString()));
    QCOMPARE(status.value(QStringLiteral("state")).toString(), QStringLiteral("MISSION READY"));

    QVERIFY(verifier.verifyUpload(plan));
    QVERIFY2(UnitTest::waitForCondition(
                 [&verifier] { return verifier.verified() || (!verifier.busy() && verifier.state() == QStringLiteral("VERIFY FAILED")) ||
                                      verifier.state() == QStringLiteral("READBACK MISMATCH"); },
                 TestTimeout::longMs() * 2,
                 QStringLiteral("NEXUS upload-readback verification completion")),
             "NEXUS verifier did not complete");

    QVERIFY2(verifier.verified(), qPrintable(verifier.message()));
    QCOMPARE(verifier.state(), QStringLiteral("MISSION READY"));
    QVERIFY(!verifier.fingerprint().isEmpty());
    QVERIFY(!plan->dirtyForUpload());
}
