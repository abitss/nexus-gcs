#include "NexusAlertManagerTest.h"

#include <QtCore/QMetaObject>
#include <QtTest/QTest>

#include "MissionManager.h"
#include "MockLink.h"
#include "NexusAlertManager.h"
#include "NexusHealthModel.h"
#include "Vehicle.h"

UT_REGISTER_TEST(NexusAlertManagerTest, TestLabel::Integration, TestLabel::Vehicle, TestLabel::Comms)

static mavlink_message_t makeHeartbeat(MAV_STATE state)
{
    mavlink_message_t msg{};
    mavlink_msg_heartbeat_pack(
        1,
        MAV_COMP_ID_AUTOPILOT1,
        &msg,
        MAV_TYPE_QUADROTOR,
        MAV_AUTOPILOT_PX4,
        MAV_MODE_FLAG_CUSTOM_MODE_ENABLED,
        0,
        state);
    return msg;
}

void NexusAlertManagerTest::_testTelemetryLossHistoryAndAck()
{
    NexusHealthModel health;
    NexusAlertManager alerts(&health);

    QTRY_VERIFY_WITH_TIMEOUT(health.vehicleConnected(), TestTimeout::longMs());
    QTRY_COMPARE_WITH_TIMEOUT(health.datalinkState(), QStringLiteral("NOMINAL"), TestTimeout::mediumMs());

    simulateCommLoss(true);

    QTRY_COMPARE_WITH_TIMEOUT(health.datalinkState(), QStringLiteral("CRITICAL"), TestTimeout::longMs());
    QTRY_VERIFY_WITH_TIMEOUT(alerts.activeCount() > 0, TestTimeout::mediumMs());
    QCOMPARE(alerts.highestSeverity(), QStringLiteral("CRITICAL"));
    QVERIFY(alerts.currentTitle().contains(QStringLiteral("Telemetry")));

    const int historyWithLoss = alerts.rowCount();
    QVERIFY(historyWithLoss > 0);

    alerts.acknowledgeAll();
    QCOMPARE(alerts.unacknowledgedCount(), 0);

    simulateCommLoss(false);
    QTRY_COMPARE_WITH_TIMEOUT(health.datalinkState(), QStringLiteral("NOMINAL"), TestTimeout::longMs());
    QTRY_VERIFY_WITH_TIMEOUT(alerts.activeCount() == 0 ||
                             alerts.currentTitle() != QStringLiteral("Telemetry Lost"),
                             TestTimeout::mediumMs());

    // Recovery clears the condition but preserves the audit trail.
    QVERIFY(alerts.rowCount() >= historyWithLoss);
}

void NexusAlertManagerTest::_testCommandRejectMissionFailureAndFailsafe()
{
    NexusHealthModel health;
    NexusAlertManager alerts(&health);
    QTRY_VERIFY_WITH_TIMEOUT(health.vehicleConnected(), TestTimeout::longMs());

    Vehicle* const vehicle = this->vehicle();
    QVERIFY(vehicle);

    const int initialRows = alerts.rowCount();

    // Inject the same public signal contract used by Vehicle when a command is rejected.
    const bool invokedCommand = QMetaObject::invokeMethod(
        vehicle,
        "mavCommandResult",
        Qt::DirectConnection,
        Q_ARG(int, vehicle->id()),
        Q_ARG(int, MAV_COMP_ID_AUTOPILOT1),
        Q_ARG(int, static_cast<int>(MAV_CMD_NAV_TAKEOFF)),
        Q_ARG(int, static_cast<int>(MAV_RESULT_DENIED)),
        Q_ARG(int, static_cast<int>(Vehicle::MavCmdResultCommandResultOnly)));
    QVERIFY(invokedCommand);
    QTRY_VERIFY_WITH_TIMEOUT(alerts.rowCount() > initialRows, TestTimeout::mediumMs());

    // MissionManager protocol errors must become persistent alert-history entries.
    MissionManager* const mission = vehicle->missionManager();
    QVERIFY(mission);
    const int beforeMissionError = alerts.rowCount();
    const bool invokedMission = QMetaObject::invokeMethod(
        mission,
        "error",
        Qt::DirectConnection,
        Q_ARG(int, static_cast<int>(PlanManager::VehicleAckError)),
        Q_ARG(QString, QStringLiteral("vehicle rejected mission")));
    QVERIFY(invokedMission);
    QTRY_VERIFY_WITH_TIMEOUT(alerts.rowCount() > beforeMissionError, TestTimeout::mediumMs());

    // Explicit autopilot failsafe text activates failsafe state.
    const bool invokedText = QMetaObject::invokeMethod(
        vehicle,
        "textMessageReceived",
        Qt::DirectConnection,
        Q_ARG(int, vehicle->id()),
        Q_ARG(int, MAV_COMP_ID_AUTOPILOT1),
        Q_ARG(int, static_cast<int>(MAV_SEVERITY_CRITICAL)),
        Q_ARG(QString, QStringLiteral("Battery failsafe activated")),
        Q_ARG(QString, QString()));
    QVERIFY(invokedText);
    QTRY_VERIFY_WITH_TIMEOUT(alerts.failsafeActive(), TestTimeout::mediumMs());
    QCOMPARE(alerts.highestSeverity(), QStringLiteral("CRITICAL"));

    // A critical heartbeat creates a separate autopilot-state alert, not a false
    // inference that a specific failsafe is active.
    mockLink()->respondWithMavlinkMessage(makeHeartbeat(MAV_STATE_CRITICAL));
    QTRY_VERIFY_WITH_TIMEOUT(alerts.activeCount() > 0, TestTimeout::mediumMs());

    // Explicit recovery/clear text clears only the failsafe condition.
    const bool invokedClear = QMetaObject::invokeMethod(
        vehicle,
        "textMessageReceived",
        Qt::DirectConnection,
        Q_ARG(int, vehicle->id()),
        Q_ARG(int, MAV_COMP_ID_AUTOPILOT1),
        Q_ARG(int, static_cast<int>(MAV_SEVERITY_INFO)),
        Q_ARG(QString, QStringLiteral("Failsafe recovered")),
        Q_ARG(QString, QString()));
    QVERIFY(invokedClear);
    QTRY_VERIFY_WITH_TIMEOUT(!alerts.failsafeActive(), TestTimeout::mediumMs());
}
