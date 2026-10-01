#include "NexusEngineerModelTest.h"

#include <QtTest/QTest>

#include "NexusEngineerModel.h"

UT_REGISTER_TEST(NexusEngineerModelTest, TestLabel::Integration, TestLabel::Vehicle)

void NexusEngineerModelTest::_testProtectedEntryAndRelock()
{
    NexusEngineerModel model;
    QTRY_VERIFY_WITH_TIMEOUT(model.connected(), TestTimeout::longMs());

    QCOMPARE(model.unlocked(), false);
    QVERIFY(!model.unlock(QStringLiteral("wrong")));
    QCOMPARE(model.unlocked(), false);

    QVERIFY(model.unlock(QStringLiteral("SECURITY_AUTHORIZED")));
    QCOMPARE(model.unlocked(), true);

    model.lock();
    QCOMPARE(model.unlocked(), false);

    QVERIFY(model.unlock(QStringLiteral("SECURITY_AUTHORIZED")));
    simulateConnectionRemoved();
    QTRY_VERIFY_WITH_TIMEOUT(!model.connected(), TestTimeout::longMs());
    QCOMPARE(model.unlocked(), false);
}

void NexusEngineerModelTest::_testSnapshotsAndWriteGate()
{
    NexusEngineerModel model;
    QTRY_VERIFY_WITH_TIMEOUT(model.connected(), TestTimeout::longMs());

    QVERIFY(model.parameterCount() >= 0);
    QVERIFY(model.unlock(QStringLiteral("ENGINEER")));

    QTRY_VERIFY_WITH_TIMEOUT(model.parameterCount() > 0, TestTimeout::longMs());
    QVERIFY(model.captureSnapshotA(QStringLiteral("A")));
    QVERIFY(model.captureSnapshotB(QStringLiteral("B")));
    QCOMPARE(model.snapshotAValid(), true);
    QCOMPARE(model.snapshotBValid(), true);
    QCOMPARE(model.snapshotDiffCount(), 0);

    simulateCommLoss(true);
    QTRY_VERIFY_WITH_TIMEOUT(!model.safeToWrite(), TestTimeout::longMs());

    simulateCommLoss(false);
    QTRY_VERIFY_WITH_TIMEOUT(model.connected(), TestTimeout::longMs());

    model.clearSnapshots();
    QCOMPARE(model.snapshotAValid(), false);
    QCOMPARE(model.snapshotBValid(), false);
}
