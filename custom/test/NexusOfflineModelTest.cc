#include "NexusOfflineModelTest.h"

#include <QtTest/QTest>

#include "NexusOfflineModel.h"

UT_REGISTER_TEST(NexusOfflineModelTest, TestLabel::Integration)

void NexusOfflineModelTest::_testLocalStorageContract()
{
    NexusOfflineModel model;

    QVERIFY(!model.missionPath().isEmpty());
    QVERIFY(!model.parameterPath().isEmpty());
    QVERIFY(!model.settingsPath().isEmpty());
    QVERIFY(!model.telemetryPath().isEmpty());
    QVERIFY(!model.logPath().isEmpty());
    QVERIFY(!model.videoPath().isEmpty());
    QVERIFY(!model.photoPath().isEmpty());

    QVERIFY(model.localStorageReady());
    QVERIFY(model.offlineReady());
    QVERIFY(model.offlineMapSetCount() >= 0);
    QVERIFY(!model.offlineMapCacheSize().isEmpty());
    QVERIFY(!model.readinessState().isEmpty());
}

void NexusOfflineModelTest::_testNoCloudContract()
{
    NexusOfflineModel model;

    QCOMPARE(model.cloudRequired(), false);
    const QString state = model.networkState();
    QVERIFY(state == QStringLiteral("ONLINE") || state == QStringLiteral("OFFLINE"));

    // Core offline readiness is deliberately independent of internet state.
    QCOMPARE(model.offlineReady(), model.localStorageReady());
}
