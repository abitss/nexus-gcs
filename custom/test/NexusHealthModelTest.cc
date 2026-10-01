#include "NexusHealthModelTest.h"

#include <QtTest/QTest>

#include "MockLink.h"
#include "NexusHealthModel.h"
#include "QGCMAVLink.h"

UT_REGISTER_TEST(NexusHealthModelTest, TestLabel::Integration, TestLabel::Vehicle, TestLabel::Comms)

static mavlink_message_t makeSysStatus(uint32_t present, uint32_t enabled, uint32_t healthy)
{
    mavlink_message_t msg{};
    mavlink_msg_sys_status_pack(
        1,
        MAV_COMP_ID_AUTOPILOT1,
        &msg,
        present,
        enabled,
        healthy,
        250,
        16800,
        8000,
        80,
        0, 0, 0, 0, 0, 0, 0, 0, 0);
    return msg;
}

void NexusHealthModelTest::_testNominalAndSensorFailure()
{
    NexusHealthModel model;
    QTRY_VERIFY_WITH_TIMEOUT(model.vehicleConnected(), TestTimeout::longMs());

    const uint32_t sensors =
        MAV_SYS_STATUS_SENSOR_3D_GYRO |
        MAV_SYS_STATUS_SENSOR_3D_ACCEL |
        MAV_SYS_STATUS_SENSOR_3D_MAG |
        MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE |
        MAV_SYS_STATUS_SENSOR_GPS |
        MAV_SYS_STATUS_GEOFENCE;

    mockLink()->respondWithMavlinkMessage(makeSysStatus(sensors, sensors, sensors));
    QTRY_COMPARE_WITH_TIMEOUT(model.gyroState(), QStringLiteral("NOMINAL"), TestTimeout::mediumMs());
    QCOMPARE(model.accelerometerState(), QStringLiteral("NOMINAL"));
    QCOMPARE(model.compassState(), QStringLiteral("NOMINAL"));
    QCOMPARE(model.barometerState(), QStringLiteral("NOMINAL"));
    QVERIFY(model.sensorFailures().isEmpty());

    const uint32_t healthWithoutAccel = sensors & ~MAV_SYS_STATUS_SENSOR_3D_ACCEL;
    mockLink()->respondWithMavlinkMessage(makeSysStatus(sensors, sensors, healthWithoutAccel));

    QTRY_COMPARE_WITH_TIMEOUT(model.accelerometerState(), QStringLiteral("CRITICAL"), TestTimeout::mediumMs());
    QCOMPARE(model.imuState(), QStringLiteral("CRITICAL"));
    QVERIFY(model.sensorFailures().contains(QGCMAVLink::mavSysStatusSensorToString(MAV_SYS_STATUS_SENSOR_3D_ACCEL)));
    QCOMPARE(model.overallState(), QStringLiteral("CRITICAL"));

    // Geofence SYS_STATUS health is also authoritative.
    const uint32_t healthWithoutFence = sensors & ~MAV_SYS_STATUS_GEOFENCE;
    mockLink()->respondWithMavlinkMessage(makeSysStatus(sensors, sensors, healthWithoutFence));
    QTRY_COMPARE_WITH_TIMEOUT(model.geofenceState(), QStringLiteral("CRITICAL"), TestTimeout::mediumMs());
}

void NexusHealthModelTest::_testDatalinkLossAndRecovery()
{
    NexusHealthModel model;
    QTRY_VERIFY_WITH_TIMEOUT(model.vehicleConnected(), TestTimeout::longMs());

    QTRY_COMPARE_WITH_TIMEOUT(model.datalinkState(), QStringLiteral("NOMINAL"), TestTimeout::mediumMs());

    simulateCommLoss(true);
    QTRY_COMPARE_WITH_TIMEOUT(model.datalinkState(), QStringLiteral("CRITICAL"), TestTimeout::longMs());
    QCOMPARE(model.overallState(), QStringLiteral("CRITICAL"));
    QCOMPARE(model.gpsState(), QStringLiteral("UNKNOWN"));

    simulateCommLoss(false);
    QTRY_COMPARE_WITH_TIMEOUT(model.datalinkState(), QStringLiteral("NOMINAL"), TestTimeout::longMs());
}
