#include "NexusAnalyzeModelTest.h"

#include <QtTest/QTest>

#include "NexusAnalyzeModel.h"

UT_REGISTER_TEST(NexusAnalyzeModelTest, TestLabel::Integration)

void NexusAnalyzeModelTest::_testHistoryContract()
{
    NexusAnalyzeModel model;

    QVERIFY(model.flightCount() >= 0);
    const QVariantList history = model.flightHistory();
    for (const QVariant &row : history) {
        const QVariantMap entry = row.toMap();
        QVERIFY(!entry.value(QStringLiteral("path")).toString().isEmpty());
        QVERIFY(!entry.value(QStringLiteral("name")).toString().isEmpty());
        QVERIFY(!entry.value(QStringLiteral("type")).toString().isEmpty());
        QVERIFY(entry.value(QStringLiteral("sizeBytes")).toLongLong() >= 0);
    }
}

void NexusAnalyzeModelTest::_testSelectionValidation()
{
    NexusAnalyzeModel model;

    QVERIFY(!model.selectFlight(QStringLiteral("/definitely/not/a/flight.tlog")));
    QCOMPARE(model.selectedPath(), QString());

    model.clearSelection();
    QCOMPARE(model.selectedPath(), QString());
}
