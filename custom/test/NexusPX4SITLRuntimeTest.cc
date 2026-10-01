#include "NexusPX4SITLRuntimeTest.h"

#include <QtCore/QPointer>
#include <QtCore/QScopeGuard>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "BatteryFactGroupListModel.h"
#include "Fact.h"
#include "LinkManager.h"
#include "MultiVehicleManager.h"
#include "QmlObjectListModel.h"
#include "UDPLink.h"
#include "Vehicle.h"
#include "VehicleGPSFactGroup.h"

UT_REGISTER_TEST_STANDALONE(NexusPX4SITLRuntimeTest,
                            TestLabel::Integration,
                            TestLabel::Vehicle,
                            TestLabel::Comms,
                            TestLabel::Slow)

bool NexusPX4SITLRuntimeTest::_acceptedAckSince(const QSignalSpy& spy, int command, int startIndex)
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

void NexusPX4SITLRuntimeTest::_testPX4SITLFlightLifecycle()
{
    LinkManager* const linkManager = LinkManager::instance();
    MultiVehicleManager* const vehicleManager = MultiVehicleManager::instance();
    QVERIFY(linkManager);
    QVERIFY(vehicleManager);

    linkManager->setConnectionsAllowed();
    linkManager->disconnectAll();

    auto* udp = new UDPConfiguration(QStringLiteral("NEXUS PX4 SITL"));
    udp->setLocalPort(14550);
    udp->setAutoConnect(false);
    SharedLinkConfigurationPtr config = linkManager->addConfiguration(udp);
    QVERIFY(config);
    QVERIFY2(linkManager->createConnectedLink(config), "Failed to bind NEXUS PX4 SITL UDP link on port 14550");

    const auto cleanup = qScopeGuard([linkManager] {
        linkManager->disconnectAll();
        QTest::qWait(250);
    });

    QVERIFY2(UnitTest::waitForCondition(
                 [vehicleManager] { return vehicleManager->activeVehicle() != nullptr; },
                 60000, QStringLiteral("PX4 SITL vehicle discovery")),
             "No PX4 SITL heartbeat reached NEXUS on UDP 14550");

    QPointer<Vehicle> vehicle = vehicleManager->activeVehicle();
    QVERIFY(vehicle);
    QCOMPARE(vehicle->firmwareType(), MAV_AUTOPILOT_PX4);

    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->isInitialConnectComplete(); },
                 90000, QStringLiteral("PX4 initial connect complete")),
             "PX4 parameter/mission initial synchronization did not complete");

    // Prove live navigation/telemetry facts from the actual PX4 SITL instance.
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->coordinate().isValid() && vehicle->homePosition().isValid(); },
                 60000, QStringLiteral("PX4 valid vehicle/home coordinates")),
             "PX4 SITL did not provide valid vehicle/home coordinates");

    auto* gps = qobject_cast<VehicleGPSFactGroup*>(vehicle->gpsFactGroup());
    QVERIFY(gps);
    QVERIFY2(UnitTest::waitForCondition(
                 [gps] {
                     return gps->lock()->rawValue().toInt() >= 3 &&
                            gps->count()->rawValue().toInt() > 0;
                 },
                 60000, QStringLiteral("PX4 GPS 3D fix")),
             "PX4 SITL GPS never reached a 3D fix");

    const auto finiteFact = [](Fact* fact) {
        return fact && qIsFinite(fact->rawValue().toDouble());
    };

    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return finiteFact(vehicle->altitudeRelative()) &&
                            finiteFact(vehicle->groundSpeed()) &&
                            finiteFact(vehicle->climbRate()) &&
                            finiteFact(vehicle->heading()) &&
                            finiteFact(vehicle->distanceToHome());
                 },
                 60000, QStringLiteral("PX4 core telemetry facts")),
             "One or more core PX4 telemetry facts remained unavailable");

    QVERIFY(vehicle->batteries());
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->batteries()->count() > 0; },
                 30000, QStringLiteral("PX4 battery fact group")),
             "PX4 SITL did not expose a battery fact group");
    auto* battery = qobject_cast<BatteryFactGroup*>(vehicle->batteries()->get(0));
    QVERIFY(battery);
    QVERIFY2(UnitTest::waitForCondition(
                 [battery] {
                     const double pct = battery->percentRemaining()->rawValue().toDouble();
                     return qIsFinite(pct) && pct >= 0.0 && pct <= 100.0;
                 },
                 30000, QStringLiteral("PX4 battery percentage")),
             "PX4 SITL battery percentage did not become valid");

    QSignalSpy ackSpy(vehicle, &Vehicle::mavCommandResult);
    QVERIFY(ackSpy.isValid());

    // ARM
    const int armAckStart = ackSpy.count();
    vehicle->setArmedShowError(true);
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->armed(); },
                 30000, QStringLiteral("PX4 armed state")),
             "PX4 SITL did not arm");
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return _acceptedAckSince(ackSpy, MAV_CMD_COMPONENT_ARM_DISARM, armAckStart); },
                 30000, QStringLiteral("PX4 ARM COMMAND_ACK")),
             "No accepted PX4 COMMAND_ACK for ARM");

    // TAKEOFF
    const int takeoffAckStart = ackSpy.count();
    vehicle->guidedModeTakeoff(5.0);
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return _acceptedAckSince(ackSpy, MAV_CMD_NAV_TAKEOFF, takeoffAckStart); },
                 30000, QStringLiteral("PX4 TAKEOFF COMMAND_ACK")),
             "No accepted PX4 COMMAND_ACK for TAKEOFF");
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle && vehicle->flying() &&
                            vehicle->altitudeRelative()->rawValue().toDouble() > 2.0;
                 },
                 60000, QStringLiteral("PX4 airborne state")),
             "PX4 SITL did not climb after TAKEOFF");

    // HOLD / PAUSE
    const int holdAckStart = ackSpy.count();
    vehicle->pauseVehicle();
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return _acceptedAckSince(ackSpy, MAV_CMD_DO_REPOSITION, holdAckStart); },
                 30000, QStringLiteral("PX4 HOLD COMMAND_ACK")),
             "No accepted PX4 COMMAND_ACK for HOLD");
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->flightMode() == vehicle->pauseFlightMode(); },
                 30000, QStringLiteral("PX4 HOLD flight mode")),
             "PX4 did not enter its pause/hold flight mode");

    // RTL is a PX4 flight-mode transition. The authoritative returned mode is the gate.
    vehicle->guidedModeRTL(false);
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->flightMode() == vehicle->rtlFlightMode(); },
                 30000, QStringLiteral("PX4 RTL flight mode")),
             "PX4 did not enter RTL");

    // LAND and wait for the simulator to report the aircraft on the ground.
    vehicle->guidedModeLand();
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->flightMode() == vehicle->landFlightMode(); },
                 30000, QStringLiteral("PX4 LAND flight mode")),
             "PX4 did not enter LAND");
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && !vehicle->flying(); },
                 90000, QStringLiteral("PX4 landed state")),
             "PX4 SITL did not report landed state");

    // Real UDP link-loss and reconnect without restarting NEXUS.
    const QList<SharedLinkInterfacePtr> links = linkManager->links();
    QVERIFY2(!links.isEmpty(), "No active UDP link available for loss/reconnect test");
    linkManager->disconnectLink(links.first().get());

    QVERIFY2(UnitTest::waitForCondition(
                 [vehicleManager] { return vehicleManager->activeVehicle() == nullptr; },
                 30000, QStringLiteral("vehicle removed after UDP link loss")),
             "NEXUS retained an active PX4 vehicle after explicit UDP link loss");

    QVERIFY2(linkManager->createConnectedLink(config), "Failed to reopen PX4 SITL UDP link");
    QVERIFY2(UnitTest::waitForCondition(
                 [vehicleManager] { return vehicleManager->activeVehicle() != nullptr; },
                 60000, QStringLiteral("PX4 vehicle rediscovery after reconnect")),
             "PX4 SITL did not reconnect");

    QPointer<Vehicle> reconnected = vehicleManager->activeVehicle();
    QVERIFY(reconnected);
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return reconnected && reconnected->isInitialConnectComplete(); },
                 90000, QStringLiteral("PX4 reconnect synchronization")),
             "PX4 reconnect did not complete parameter/mission synchronization");
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return reconnected && reconnected->coordinate().isValid(); },
                 30000, QStringLiteral("PX4 telemetry after reconnect")),
             "PX4 telemetry did not recover after reconnect");
}
