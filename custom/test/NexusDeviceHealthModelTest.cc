#include "NexusDeviceHealthModelTest.h"

#include <QtTest/QTest>

#include "NexusDeviceHealthModel.h"

UT_REGISTER_TEST(NexusDeviceHealthModelTest, TestLabel::Integration)

void NexusDeviceHealthModelTest::_testThresholdContracts()
{
    NexusDeviceHealthModel model;
    model.refresh();

    QVERIFY(model.batteryPercent() >= -1);
    QVERIFY(model.storageFreeBytes() >= 0);
    QVERIFY(model.storageTotalBytes() >= 0);
    QVERIFY(model.ramTotalMb() >= -1);
    QVERIFY(model.ramAvailableMb() >= -1);

    const QString overall = model.overallState();
    QVERIFY(overall == QStringLiteral("NOMINAL") ||
            overall == QStringLiteral("WARNING") ||
            overall == QStringLiteral("CRITICAL"));
}

void NexusDeviceHealthModelTest::_testPlatformStateContract()
{
    NexusDeviceHealthModel model;
    model.refresh();

    QVERIFY(!model.networkState().isEmpty());
    QVERIFY(!model.cameraPermission().isEmpty());
    QVERIFY(!model.locationPermission().isEmpty());
    QVERIFY(!model.storagePermission().isEmpty());
    QVERIFY(!model.thermalState().isEmpty());

    model.setOperationalScreenAwake(true);
    QCOMPARE(model.keepScreenAwake(), true);
    model.setOperationalScreenAwake(false);
    QCOMPARE(model.keepScreenAwake(), false);
}

void NexusDeviceHealthModelTest::_testWarningThresholds()
{
    QVERIFY(NexusDeviceHealthModel::batteryWarningFor(20, QStringLiteral("ON BATTERY")));
    QVERIFY(!NexusDeviceHealthModel::batteryWarningFor(21, QStringLiteral("ON BATTERY")));
    QVERIFY(!NexusDeviceHealthModel::batteryWarningFor(10, QStringLiteral("CHARGING")));

    constexpr quint64 GiB = 1024ULL * 1024ULL * 1024ULL;
    QVERIFY(NexusDeviceHealthModel::storageWarningFor(1 * GiB, 64 * GiB));
    QVERIFY(NexusDeviceHealthModel::storageWarningFor(2 * GiB, 100 * GiB));
    QVERIFY(!NexusDeviceHealthModel::storageWarningFor(10 * GiB, 64 * GiB));

    QVERIFY(NexusDeviceHealthModel::thermalWarningFor(45.0, QStringLiteral("NORMAL")));
    QVERIFY(NexusDeviceHealthModel::thermalWarningFor(qQNaN(), QStringLiteral("SEVERE")));
    QVERIFY(!NexusDeviceHealthModel::thermalWarningFor(39.0, QStringLiteral("NORMAL")));
}
