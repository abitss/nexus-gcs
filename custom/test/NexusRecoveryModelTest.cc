#include "NexusRecoveryModelTest.h"

#include <QtTest/QTest>

#include "NexusRecoveryModel.h"

UT_REGISTER_TEST(NexusRecoveryModelTest, TestLabel::Integration)

void NexusRecoveryModelTest::_testCleanStateContract()
{
    NexusRecoveryModel model(nullptr, nullptr, nullptr, nullptr);
    QVERIFY(!model.overallState().isEmpty());
    QVERIFY(!model.appState().isEmpty());
    QVERIFY(!model.lastRecoveryEvent().isEmpty());
    QVERIFY(model.events().size() >= 0);

    model.clearRecoveredEvents();
    QVERIFY(!model.lastRecoveryEvent().isEmpty());
}
