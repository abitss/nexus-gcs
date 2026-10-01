#include "NexusRealPixhawkBenchTest.h"

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QScopeGuard>
#include <QtPositioning/QGeoCoordinate>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "BatteryFactGroupListModel.h"
#include "Fact.h"
#include "LinkManager.h"
#include "MissionController.h"
#include "MultiVehicleManager.h"
#include "NexusPlanVerifier.h"
#include "ParameterManager.h"
#include "PlanMasterController.h"
#include "SerialLink.h"
#include "SimpleMissionItem.h"
#include "Vehicle.h"
#include "VehicleGPSFactGroup.h"

UT_REGISTER_TEST_STANDALONE(NexusRealPixhawkBenchTest,
                            TestLabel::Integration,
                            TestLabel::MissionManager,
                            TestLabel::Vehicle,
                            TestLabel::Comms,
                            TestLabel::Slow)

bool NexusRealPixhawkBenchTest::_acceptedAckSince(const QSignalSpy &spy, int command, int startIndex)
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

void NexusRealPixhawkBenchTest::_writeEvidence(const QJsonObject &root)
{
    const QString path = qEnvironmentVariable(
        "NEXUS_PIXHAWK_BENCH_EVIDENCE",
        QStringLiteral("/tmp/nexus-real-pixhawk-bench.json"));

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

void NexusRealPixhawkBenchTest::_testRealPixhawkBenchQualification()
{
    const QString port = qEnvironmentVariable("NEXUS_REAL_PIXHAWK_PORT").trimmed();
    const int baud = qEnvironmentVariableIntValue("NEXUS_REAL_PIXHAWK_BAUD");
    const QString propsRemoved = qEnvironmentVariable("NEXUS_PROPS_REMOVED").trimmed().toUpper();
    const QString killConfirmed = qEnvironmentVariable("NEXUS_KILL_SWITCH_CONFIRMED").trimmed().toUpper();

    if (port.isEmpty()) {
        QSKIP("Real Pixhawk bench test requires NEXUS_REAL_PIXHAWK_PORT.");
    }
    QVERIFY2(propsRemoved == QStringLiteral("YES"),
             "Refusing real Pixhawk command testing: NEXUS_PROPS_REMOVED must equal YES.");
    QVERIFY2(killConfirmed == QStringLiteral("YES"),
             "Refusing real Pixhawk command testing: NEXUS_KILL_SWITCH_CONFIRMED must equal YES.");

    QJsonArray stages;
    QJsonObject facts;
    QJsonObject root{
        {QStringLiteral("schemaVersion"), QStringLiteral("1.0")},
        {QStringLiteral("suite"), QStringLiteral("NEXUS REAL PIXHAWK BENCH QUALIFICATION")},
        {QStringLiteral("port"), port},
        {QStringLiteral("baud"), baud > 0 ? baud : 115200},
        {QStringLiteral("propsRemoved"), true},
        {QStringLiteral("killSwitchConfirmed"), true},
    };

    auto stage = [&](const QString &name, const QString &status, const QString &detail = QString()) {
        QJsonObject row{
            {QStringLiteral("stage"), name},
            {QStringLiteral("status"), status}
        };
        if (!detail.isEmpty()) row.insert(QStringLiteral("detail"), detail);
        stages.append(row);
        root.insert(QStringLiteral("stages"), stages);
        root.insert(QStringLiteral("facts"), facts);
        _writeEvidence(root);
    };

    LinkManager *const linkManager = LinkManager::instance();
    MultiVehicleManager *const vehicleManager = MultiVehicleManager::instance();
    QVERIFY(linkManager);
    QVERIFY(vehicleManager);

    linkManager->setConnectionsAllowed();
    linkManager->disconnectAll();

    auto serial = std::make_shared<SerialConfiguration>(QStringLiteral("NEXUS REAL PIXHAWK BENCH"));
    serial->setPortName(port);
    serial->setBaud(baud > 0 ? baud : 115200);
    serial->setAutoConnect(false);

    SharedLinkConfigurationPtr config = serial;
    QVERIFY2(linkManager->createConnectedLink(config),
             qPrintable(QStringLiteral("Failed to open Pixhawk serial port %1").arg(port)));

    const auto cleanup = qScopeGuard([&] {
        root.insert(QStringLiteral("stages"), stages);
        root.insert(QStringLiteral("facts"), facts);
        _writeEvidence(root);
        linkManager->disconnectAll();
        QTest::qWait(250);
    });

    stage(QStringLiteral("USB_TELEMETRY_CONNECTION"), QStringLiteral("PASS"),
          QStringLiteral("%1 @ %2 baud").arg(port).arg(serial->baud()));

    QVERIFY2(UnitTest::waitForCondition(
                 [vehicleManager] { return vehicleManager->activeVehicle() != nullptr; },
                 60000, QStringLiteral("real Pixhawk heartbeat discovery")),
             "No real Pixhawk vehicle was discovered on the configured serial link.");

    QPointer<Vehicle> vehicle = vehicleManager->activeVehicle();
    QVERIFY(vehicle);
    QCOMPARE(vehicle->firmwareType(), MAV_AUTOPILOT_PX4);

    facts.insert(QStringLiteral("systemId"), vehicle->id());
    facts.insert(QStringLiteral("componentId"), vehicle->defaultComponentId());
    facts.insert(QStringLiteral("firmwareType"), vehicle->firmwareTypeString());
    facts.insert(QStringLiteral("vehicleType"), vehicle->vehicleTypeString());

    // Count actual MAVLink HEARTBEAT frames from the discovered system.
    int heartbeatCount = 0;
    const QMetaObject::Connection heartbeatConnection =
        connect(vehicle, &Vehicle::mavlinkMessageReceived, this,
                [&](const mavlink_message_t &message) {
                    if (message.sysid == vehicle->id() && message.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
                        ++heartbeatCount;
                    }
                });

    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return heartbeatCount >= 3; },
                 10000, QStringLiteral("real Pixhawk heartbeat stream")),
             "Fewer than three HEARTBEAT frames were observed from the Pixhawk.");
    disconnect(heartbeatConnection);
    facts.insert(QStringLiteral("heartbeatCountObserved"), heartbeatCount);
    stage(QStringLiteral("HEARTBEAT"), QStringLiteral("PASS"),
          QStringLiteral("%1 HEARTBEAT frames observed").arg(heartbeatCount));

    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return vehicle && vehicle->parameterManager() &&
                            vehicle->parameterManager()->parametersReady();
                 },
                 120000, QStringLiteral("real Pixhawk parameter synchronization")),
             "Pixhawk parameter synchronization did not complete.");

    ParameterManager *const params = vehicle->parameterManager();
    QVERIFY(params);
    const QStringList paramNames = params->parameterNames(ParameterManager::defaultComponentId);
    QVERIFY2(paramNames.size() > 100, "Unexpectedly small Pixhawk parameter set.");
    facts.insert(QStringLiteral("parameterCount"), paramNames.size());
    facts.insert(QStringLiteral("parametersMissing"), params->missingParameters());
    stage(QStringLiteral("PARAMETERS"), QStringLiteral("PASS"),
          QStringLiteral("%1 parameters synchronized").arg(paramNames.size()));

    auto *gps = qobject_cast<VehicleGPSFactGroup *>(vehicle->gpsFactGroup());
    QVERIFY(gps);
    QVERIFY2(UnitTest::waitForCondition(
                 [gps] {
                     return gps->lock()->rawValue().toInt() >= 3 &&
                            gps->count()->rawValue().toInt() >= 4;
                 },
                 120000, QStringLiteral("real Pixhawk GPS fix")),
             "Pixhawk GPS did not reach a 3D fix with at least four satellites.");

    facts.insert(QStringLiteral("gpsFix"), gps->lock()->rawValue().toInt());
    facts.insert(QStringLiteral("satellites"), gps->count()->rawValue().toInt());
    stage(QStringLiteral("GPS"), QStringLiteral("PASS"),
          QStringLiteral("3D fix · %1 satellites").arg(gps->count()->rawValue().toInt()));

    QVERIFY(vehicle->batteries());
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->batteries()->count() > 0; },
                 30000, QStringLiteral("real Pixhawk battery group")),
             "No battery telemetry group was received.");

    auto *battery = qobject_cast<BatteryFactGroup *>(vehicle->batteries()->get(0));
    QVERIFY(battery);
    QVERIFY2(UnitTest::waitForCondition(
                 [battery] {
                     const double voltage = battery->voltage()->rawValue().toDouble();
                     return qIsFinite(voltage) && voltage > 1.0;
                 },
                 30000, QStringLiteral("real Pixhawk battery voltage")),
             "Battery voltage telemetry is unavailable or invalid.");

    const double voltage = battery->voltage()->rawValue().toDouble();
    const double current = battery->current()->rawValue().toDouble();
    const double remaining = battery->percentRemaining()->rawValue().toDouble();
    facts.insert(QStringLiteral("batteryVoltageV"), voltage);
    if (qIsFinite(current)) facts.insert(QStringLiteral("batteryCurrentA"), current);
    if (qIsFinite(remaining)) facts.insert(QStringLiteral("batteryRemainingPercent"), remaining);
    stage(QStringLiteral("BATTERY"), QStringLiteral("PASS"),
          QStringLiteral("%1 V").arg(voltage, 0, 'f', 2));

    const QStringList modes = vehicle->flightModes();
    QVERIFY2(!modes.isEmpty(), "Pixhawk exposed no flight modes.");
    QVERIFY2(!vehicle->flightMode().isEmpty(), "Current Pixhawk flight mode is empty.");
    facts.insert(QStringLiteral("currentMode"), vehicle->flightMode());
    facts.insert(QStringLiteral("availableModeCount"), modes.size());
    facts.insert(QStringLiteral("availableModes"), QJsonArray::fromStringList(modes));
    stage(QStringLiteral("MODES"), QStringLiteral("PASS"),
          QStringLiteral("%1 modes · current %2").arg(modes.size()).arg(vehicle->flightMode()));

    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && vehicle->homePosition().isValid(); },
                 120000, QStringLiteral("real Pixhawk Home")),
             "Pixhawk Home position did not become valid.");

    const QGeoCoordinate home = vehicle->homePosition();
    facts.insert(QStringLiteral("homeLatitude"), home.latitude());
    facts.insert(QStringLiteral("homeLongitude"), home.longitude());
    facts.insert(QStringLiteral("homeAltitude"), home.altitude());
    stage(QStringLiteral("HOME"), QStringLiteral("PASS"));

    // Upload and immediately read back a small mission. The mission is never started.
    PlanMasterController plan;
    plan.setFlyView(false);
    plan.start();
    plan.startStaticActiveVehicle(vehicle);

    MissionController *const mission = plan.missionController();
    QVERIFY(mission);
    mission->setHomePosition(home);

    auto *wp1 = qobject_cast<SimpleMissionItem *>(
        mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(70.0, 45.0), -1, true));
    auto *wp2 = qobject_cast<SimpleMissionItem *>(
        mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(90.0, 135.0), -1, true));
    QVERIFY(wp1);
    QVERIFY(wp2);
    wp1->altitude()->setRawValue(20.0);
    wp2->altitude()->setRawValue(20.0);

    NexusPlanVerifier verifier;
    const QVariantMap planStatus = verifier.validationStatus(&plan);
    QVERIFY2(planStatus.value(QStringLiteral("ready")).toBool(),
             qPrintable(planStatus.value(QStringLiteral("message")).toString()));

    QVERIFY(verifier.verifyUpload(&plan));
    QVERIFY2(UnitTest::waitForCondition(
                 [&] {
                     return verifier.verified() ||
                            verifier.state() == QStringLiteral("VERIFY FAILED") ||
                            verifier.state() == QStringLiteral("READBACK MISMATCH");
                 },
                 120000, QStringLiteral("real Pixhawk mission upload/readback")),
             "Real Pixhawk mission upload/readback verification timed out.");
    QVERIFY2(verifier.verified(), qPrintable(verifier.message()));
    facts.insert(QStringLiteral("missionFingerprint"), verifier.fingerprint());
    stage(QStringLiteral("MISSION_UPLOAD_DOWNLOAD"), QStringLiteral("PASS"),
          QStringLiteral("Readback fingerprint %1").arg(verifier.fingerprint()));

    // Props-off command-path qualification. No throttle, takeoff, mission start,
    // actuator test or movement command is issued.
    QVERIFY2(!vehicle->flying(), "Refusing props-off command test because vehicle reports flying.");

    QSignalSpy ackSpy(vehicle, &Vehicle::mavCommandResult);
    QVERIFY(ackSpy.isValid());

    const int armAckStart = ackSpy.count();
    vehicle->setArmedShowError(true);

    const bool armAccepted = UnitTest::waitForCondition(
        [&] { return vehicle && vehicle->armed(); },
        30000, QStringLiteral("real Pixhawk props-off arm state"));

    const bool armAckAccepted = UnitTest::waitForCondition(
        [&] { return _acceptedAckSince(ackSpy, MAV_CMD_COMPONENT_ARM_DISARM, armAckStart); },
        30000, QStringLiteral("real Pixhawk props-off arm ACK"));

    QVERIFY2(armAccepted && armAckAccepted,
             "Props-off ARM command was not accepted. Resolve real PX4 preflight/arming checks before bench qualification.");

    stage(QStringLiteral("PROPS_OFF_ARM"), QStringLiteral("PASS"),
          QStringLiteral("ARM accepted with explicit props-removed interlock"));

    const int disarmAckStart = ackSpy.count();
    vehicle->setArmedShowError(false);

    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return vehicle && !vehicle->armed(); },
                 30000, QStringLiteral("real Pixhawk props-off disarm state")),
             "Pixhawk did not return to disarmed state.");

    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return _acceptedAckSince(ackSpy, MAV_CMD_COMPONENT_ARM_DISARM, disarmAckStart); },
                 30000, QStringLiteral("real Pixhawk props-off disarm ACK")),
             "No accepted DISARM COMMAND_ACK was observed.");

    stage(QStringLiteral("PROPS_OFF_DISARM"), QStringLiteral("PASS"));

    root.insert(QStringLiteral("facts"), facts);
    root.insert(QStringLiteral("stages"), stages);
    root.insert(QStringLiteral("qualification"), QStringLiteral("PASS"));
    root.insert(QStringLiteral("scope"), QStringLiteral("REAL PIXHAWK BENCH ONLY · NO PROP-ON OR FLIGHT AUTHORIZATION"));
    _writeEvidence(root);
}
