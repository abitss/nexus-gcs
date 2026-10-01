#include "NexusVehicleModelTest.h"

#include <QtTest/QTest>

#include "NexusVehicleModel.h"
#include "Vehicle.h"

UT_REGISTER_TEST(NexusVehicleModelTest, TestLabel::Integration, TestLabel::Vehicle)

void NexusVehicleModelTest::_testConnectedSummaryAndSetupStates()
{
    NexusVehicleModel model;

    QTRY_VERIFY_WITH_TIMEOUT(model.connected(), TestTimeout::longMs());
    QVERIFY(!model.vehicleIdText().isEmpty());
    QVERIFY(!model.vehicleType().isEmpty());
    QVERIFY(!model.firmwareType().isEmpty());
    QVERIFY(!model.setupState().isEmpty());
    QVERIFY(!model.parameterState().isEmpty());

    QVERIFY(!model.sensorsState().isEmpty());
    QVERIFY(!model.powerState().isEmpty());
    QVERIFY(!model.radioState().isEmpty());
    QVERIFY(!model.flightModesState().isEmpty());
    QVERIFY(!model.safetyState().isEmpty());

    QCOMPARE(model.rebootRequired(), false);
    QVERIFY(model.rebootParameters().isEmpty());
}

void NexusVehicleModelTest::_testSafetyGates()
{
    NexusVehicleModel model;
    QTRY_VERIFY_WITH_TIMEOUT(model.connected(), TestTimeout::longMs());

    Vehicle *testVehicle = vehicle();
    QVERIFY(testVehicle);

    if (!model.linkLost() && !testVehicle->armed() && !testVehicle->flying()) {
        QVERIFY(model.safeToConfigure());
    }

    simulateCommLoss(true);
    QTRY_VERIFY_WITH_TIMEOUT(model.linkLost(), TestTimeout::longMs());
    QVERIFY(!model.safeToConfigure());
    QVERIFY(!model.safeToReboot());

    simulateCommLoss(false);
    QTRY_VERIFY_WITH_TIMEOUT(!model.linkLost(), TestTimeout::longMs());

    simulateConnectionRemoved();
    QTRY_VERIFY_WITH_TIMEOUT(!model.connected(), TestTimeout::longMs());
    QVERIFY(!model.safeToConfigure());
    QVERIFY(!model.safeToReboot());
}
