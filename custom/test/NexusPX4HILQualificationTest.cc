#include "NexusPX4HILQualificationTest.h"

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QScopeGuard>
#include <QtPositioning/QGeoCoordinate>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "HealthAndArmingCheckReport.h"
#include "LinkManager.h"
#include "MissionController.h"
#include "MissionManager.h"
#include "MultiVehicleManager.h"
#include "NexusPlanVerifier.h"
#include "PlanMasterController.h"
#include "SerialLink.h"
#include "SimpleMissionItem.h"
#include "Vehicle.h"

UT_REGISTER_TEST_STANDALONE(NexusPX4HILQualificationTest,
                            TestLabel::Integration,
                            TestLabel::MissionManager,
                            TestLabel::Vehicle,
                            TestLabel::Comms,
                            TestLabel::Slow)

bool NexusPX4HILQualificationTest::_acceptedAckSince(const QSignalSpy &spy, int command, int startIndex)
{
    for (int i=startIndex;i<spy.count();++i) {
        const auto args=spy.at(i);
        if(args.size()>=5 && args.at(2).toInt()==command && args.at(3).toInt()==MAV_RESULT_ACCEPTED) return true;
    }
    return false;
}

void NexusPX4HILQualificationTest::_testHILLifecycle()
{
    const QString port=qEnvironmentVariable("NEXUS_HIL_PIXHAWK_PORT").trimmed();
    const int baud=qEnvironmentVariableIntValue("NEXUS_HIL_PIXHAWK_BAUD");
    const QString props=qEnvironmentVariable("NEXUS_PROPS_REMOVED").trimmed().toUpper();
    const QString sim=qEnvironmentVariable("NEXUS_HIL_SIMULATOR_CONFIRMED").trimmed().toUpper();
    if(port.isEmpty()) QSKIP("HIL requires NEXUS_HIL_PIXHAWK_PORT.");
    QVERIFY2(props=="YES","HIL command testing requires props physically removed.");
    QVERIFY2(sim=="YES","HIL simulator confirmation is required.");

    QJsonArray stages;
    QJsonObject root{{"schemaVersion","1.0"},{"suite","NEXUS PX4 HIL QUALIFICATION"}};
    const QString evidence=qEnvironmentVariable("NEXUS_HIL_EVIDENCE","/tmp/nexus-px4-hil.json");
    auto stage=[&](const QString &name,const QString &status,const QString &detail=QString()){
        QJsonObject row{{"stage",name},{"status",status}};
        if(!detail.isEmpty()) row.insert("detail",detail);
        stages.append(row); root.insert("stages",stages);
        QFile f(evidence); if(f.open(QIODevice::WriteOnly|QIODevice::Truncate)) f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    };

    auto *lm=LinkManager::instance();
    auto *mvm=MultiVehicleManager::instance();
    QVERIFY(lm); QVERIFY(mvm);
    lm->setConnectionsAllowed(); lm->disconnectAll();

    auto serial=std::make_shared<SerialConfiguration>(QStringLiteral("NEXUS PX4 HIL"));
    serial->setPortName(port); serial->setBaud(baud>0?baud:921600); serial->setAutoConnect(false);
    SharedLinkConfigurationPtr config=serial;
    QVERIFY(lm->createConnectedLink(config));
    const auto cleanup=qScopeGuard([&]{lm->disconnectAll(); QTest::qWait(250);});

    QVERIFY2(UnitTest::waitForCondition([&]{return mvm->activeVehicle()!=nullptr;},60000,QStringLiteral("HIL Pixhawk discovery")),"No HIL Pixhawk discovered.");
    QPointer<Vehicle> vehicle=mvm->activeVehicle();
    QVERIFY(vehicle); QCOMPARE(vehicle->firmwareType(),MAV_AUTOPILOT_PX4);
    stage("CONNECT","PASS");

    bool hilEnabled=false;
    connect(vehicle,&Vehicle::mavlinkMessageReceived,this,[&](const mavlink_message_t &m){
        if(m.sysid!=vehicle->id() || m.msgid!=MAVLINK_MSG_ID_HEARTBEAT) return;
        mavlink_heartbeat_t hb{}; mavlink_msg_heartbeat_decode(&m,&hb);
        hilEnabled=(hb.base_mode & MAV_MODE_FLAG_HIL_ENABLED)!=0;
    });
    QVERIFY2(UnitTest::waitForCondition([&]{return hilEnabled;},20000,QStringLiteral("PX4 HIL heartbeat flag")),"PX4 heartbeat never reported HIL enabled.");
    stage("HIL_ENABLED","PASS");

    QVERIFY2(UnitTest::waitForCondition([&]{return vehicle->isInitialConnectComplete();},90000,QStringLiteral("HIL initial sync")),"HIL initial sync failed.");
    stage("PREFLIGHT_SYNC","PASS");

    QVERIFY2(UnitTest::waitForCondition([&]{return vehicle->homePosition().isValid() && vehicle->coordinate().isValid();},60000,QStringLiteral("HIL position/home")),"HIL position/home invalid.");

    PlanMasterController plan; plan.setFlyView(false); plan.start(); plan.startStaticActiveVehicle(vehicle);
    auto *mission=plan.missionController(); QVERIFY(mission);
    const QGeoCoordinate home=vehicle->homePosition(); mission->setHomePosition(home);
    auto *a=qobject_cast<SimpleMissionItem*>(mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(80,30),-1,true));
    auto *b=qobject_cast<SimpleMissionItem*>(mission->insertSimpleMissionItem(home.atDistanceAndAzimuth(110,140),-1,true));
    QVERIFY(a); QVERIFY(b); a->altitude()->setRawValue(15.0); b->altitude()->setRawValue(18.0);

    NexusPlanVerifier verifier;
    QVERIFY(verifier.verifyUpload(&plan));
    QVERIFY2(UnitTest::waitForCondition([&]{return verifier.verified()||verifier.state()=="VERIFY FAILED"||verifier.state()=="READBACK MISMATCH";},120000,QStringLiteral("HIL mission verify")),"HIL mission verify timed out.");
    QVERIFY2(verifier.verified(),qPrintable(verifier.message()));
    stage("MISSION_UPLOAD_READBACK","PASS",verifier.fingerprint());

    QSignalSpy ack(vehicle,&Vehicle::mavCommandResult); QVERIFY(ack.isValid());

    int n=ack.count(); vehicle->setArmedShowError(true);
    QVERIFY2(UnitTest::waitForCondition([&]{return vehicle->armed();},30000,QStringLiteral("HIL arm")),"HIL arm failed.");
    QVERIFY2(UnitTest::waitForCondition([&]{return _acceptedAckSince(ack,MAV_CMD_COMPONENT_ARM_DISARM,n);},30000,QStringLiteral("HIL arm ACK")),"No accepted HIL arm ACK.");
    stage("ARM","PASS");

    n=ack.count(); vehicle->guidedModeTakeoff(8.0);
    QVERIFY2(UnitTest::waitForCondition([&]{return _acceptedAckSince(ack,MAV_CMD_NAV_TAKEOFF,n);},30000,QStringLiteral("HIL takeoff ACK")),"No accepted HIL takeoff ACK.");
    QVERIFY2(UnitTest::waitForCondition([&]{return vehicle->flying() && vehicle->altitudeRelative()->rawValue().toDouble()>4.0;},60000,QStringLiteral("HIL airborne")),"HIL vehicle did not become airborne.");
    stage("TAKEOFF","PASS");

    vehicle->startMission();
    QVERIFY2(UnitTest::waitForCondition([&]{return vehicle->flightMode()==vehicle->missionFlightMode();},30000,QStringLiteral("HIL mission mode")),"HIL mission mode failed.");
    QVERIFY2(UnitTest::waitForCondition([&]{return vehicle->missionManager()->currentIndex()>=1;},60000,QStringLiteral("HIL waypoint progress")),"HIL mission did not progress.");
    stage("MISSION_EXECUTION","PASS");

    vehicle->pauseVehicle();
    QVERIFY2(UnitTest::waitForCondition([&]{return vehicle->flightMode()==vehicle->pauseFlightMode();},30000,QStringLiteral("HIL hold")),"HIL hold failed.");
    stage("HOLD","PASS");

    vehicle->startMission();
    QVERIFY2(UnitTest::waitForCondition([&]{return vehicle->flightMode()==vehicle->missionFlightMode();},30000,QStringLiteral("HIL continue")),"HIL continue failed.");
    stage("CONTINUE","PASS");

    vehicle->guidedModeRTL(false);
    QVERIFY2(UnitTest::waitForCondition([&]{return vehicle->flightMode()==vehicle->rtlFlightMode();},30000,QStringLiteral("HIL RTL")),"HIL RTL failed.");
    stage("RTL","PASS");

    vehicle->guidedModeLand();
    QVERIFY2(UnitTest::waitForCondition([&]{return vehicle->flightMode()==vehicle->landFlightMode();},30000,QStringLiteral("HIL land mode")),"HIL land mode failed.");
    QVERIFY2(UnitTest::waitForCondition([&]{return !vehicle->flying();},90000,QStringLiteral("HIL landed")),"HIL did not land.");
    QVERIFY2(UnitTest::waitForCondition([&]{return !vehicle->armed();},60000,QStringLiteral("HIL disarm")),"HIL did not disarm.");
    stage("LAND","PASS");

    root.insert("qualification","PASS");
    QFile f(evidence); if(f.open(QIODevice::WriteOnly|QIODevice::Truncate)) f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}
