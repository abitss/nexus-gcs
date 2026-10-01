#include "NexusPreflightModelTest.h"

#include <QtCore/QMetaObject>
#include <QtTest/QTest>

#include "NexusAlertManager.h"
#include "NexusHealthModel.h"
#include "NexusPreflightModel.h"
#include "Vehicle.h"

UT_REGISTER_TEST(NexusPreflightModelTest, TestLabel::Integration, TestLabel::Vehicle, TestLabel::Comms)

void NexusPreflightModelTest::_testMissionStatesAndLinkLoss()
{
    NexusHealthModel health;
    NexusAlertManager alerts(&health);
    NexusPreflightModel preflight(&health, &alerts);

    QTRY_VERIFY_WITH_TIMEOUT(health.vehicleConnected(), TestTimeout::longMs());

    preflight.updateMission(true,
                            false,
                            false,
                            QStringLiteral("INCOMPLETE"),
                            QStringLiteral("Mission item incomplete"));
    QTRY_COMPARE_WITH_TIMEOUT(preflight.missionState(), QStringLiteral("BLOCKED"), TestTimeout::mediumMs());
    QCOMPARE(preflight.overallState(), QStringLiteral("BLOCKED"));

    preflight.updateMission(true,
                            true,
                            false,
                            QStringLiteral("MISSION READY"),
                            QStringLiteral("Ready for upload"));
    QTRY_COMPARE_WITH_TIMEOUT(preflight.missionState(), QStringLiteral("WARNING"), TestTimeout::mediumMs());

    preflight.updateMission(true,
                            true,
                            true,
                            QStringLiteral("MISSION READY"),
                            QStringLiteral("Vehicle readback verified"));
    QTRY_COMPARE_WITH_TIMEOUT(preflight.missionState(), QStringLiteral("GO"), TestTimeout::mediumMs());

    simulateCommLoss(true);
    QTRY_COMPARE_WITH_TIMEOUT(preflight.datalinkState(), QStringLiteral("BLOCKED"), TestTimeout::longMs());
    QCOMPARE(preflight.overallState(), QStringLiteral("BLOCKED"));

    simulateCommLoss(false);
    QTRY_VERIFY_WITH_TIMEOUT(preflight.datalinkState() != QStringLiteral("BLOCKED"), TestTimeout::longMs());
}

void NexusPreflightModelTest::_testFailsafeBlocksPreflight()
{
    NexusHealthModel health;
    NexusAlertManager alerts(&health);
    NexusPreflightModel preflight(&health, &alerts);

    QTRY_VERIFY_WITH_TIMEOUT(health.vehicleConnected(), TestTimeout::longMs());

    Vehicle* const v = vehicle();
    QVERIFY(v);

    const bool invoked = QMetaObject::invokeMethod(
        v,
        "textMessageReceived",
        Qt::DirectConnection,
        Q_ARG(int, v->id()),
        Q_ARG(int, MAV_COMP_ID_AUTOPILOT1),
        Q_ARG(int, static_cast<int>(MAV_SEVERITY_CRITICAL)),
        Q_ARG(QString, QStringLiteral("RC failsafe activated")),
        Q_ARG(QString, QString()));
    QVERIFY(invoked);

    QTRY_VERIFY_WITH_TIMEOUT(alerts.failsafeActive(), TestTimeout::mediumMs());
    QTRY_COMPARE_WITH_TIMEOUT(preflight.overallState(), QStringLiteral("BLOCKED"), TestTimeout::mediumMs());
    QVERIFY(preflight.overallDetail().contains(QStringLiteral("failsafe"), Qt::CaseInsensitive));
}

void NexusPreflightModelTest::_testNoVehicleBlocks()
{
    NexusHealthModel health;
    NexusAlertManager alerts(&health);
    NexusPreflightModel preflight(&health, &alerts);

    QTRY_VERIFY_WITH_TIMEOUT(health.vehicleConnected(), TestTimeout::longMs());

    _disconnectMockLink();
    QTRY_VERIFY_WITH_TIMEOUT(!health.vehicleConnected(), TestTimeout::longMs());

    preflight.refresh();
    QCOMPARE(preflight.vehicleState(), QStringLiteral("BLOCKED"));
    QCOMPARE(preflight.datalinkState(), QStringLiteral("BLOCKED"));
    QCOMPARE(preflight.overallState(), QStringLiteral("BLOCKED"));
}
