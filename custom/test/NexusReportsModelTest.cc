#include "NexusReportsModelTest.h"

#include <QtTest/QTest>
#include <QtCore/QtMath>

#include "NexusReportsModel.h"

UT_REGISTER_TEST(NexusReportsModelTest, TestLabel::Integration)

void NexusReportsModelTest::_testLocalMetadataPersistence()
{
    NexusReportsModel model;
    const QString source = QStringLiteral("/tmp/nexus-report-test.ulg");

    model.loadForSource(source);
    model.setMissionId(QStringLiteral("MISSION-42"));
    model.setOperatorName(QStringLiteral("Operator A"));
    model.setAircraft(QStringLiteral("Quadrotor"));
    model.setFirmware(QStringLiteral("PX4"));
    model.setMissionCompletion(QStringLiteral("COMPLETED"));

    NexusReportsModel reloaded;
    reloaded.loadForSource(source);

    QCOMPARE(reloaded.missionId(), QStringLiteral("MISSION-42"));
    QCOMPARE(reloaded.operatorName(), QStringLiteral("Operator A"));
    QCOMPARE(reloaded.aircraft(), QStringLiteral("Quadrotor"));
    QCOMPARE(reloaded.firmware(), QStringLiteral("PX4"));
    QCOMPARE(reloaded.missionCompletion(), QStringLiteral("COMPLETED"));
}

void NexusReportsModelTest::_testReportSchema()
{
    NexusReportsModel model;
    model.loadForSource(QStringLiteral("/tmp/report-source.ulg"));
    model.setMissionId(QStringLiteral("M-1"));
    model.setOperatorName(QStringLiteral("Pilot"));
    model.setMissionCompletion(QStringLiteral("PARTIAL"));

    const QVariantMap report = model.buildReportData(
        QStringLiteral("2026-10-01T10:00:00"),
        120.0,
        850.0,
        42.5,
        18.0,
        QVariantList{},
        QVariantList{},
        QVariantList{});

    QCOMPARE(report.value(QStringLiteral("schemaVersion")).toString(), QStringLiteral("1.0"));
    QCOMPARE(report.value(QStringLiteral("missionId")).toString(), QStringLiteral("M-1"));
    QCOMPARE(report.value(QStringLiteral("operator")).toString(), QStringLiteral("Pilot"));
    QCOMPARE(report.value(QStringLiteral("durationSeconds")).toDouble(), 120.0);
    QCOMPARE(report.value(QStringLiteral("distanceMeters")).toDouble(), 850.0);
    QCOMPARE(report.value(QStringLiteral("maxAltitudeMeters")).toDouble(), 42.5);
    QCOMPARE(report.value(QStringLiteral("batteryUsedPercent")).toDouble(), 18.0);
    QCOMPARE(report.value(QStringLiteral("missionCompletion")).toString(), QStringLiteral("PARTIAL"));
    QVERIFY(report.value(QStringLiteral("exportTargets")).toStringList().contains(QStringLiteral("KML")));
}


void NexusReportsModelTest::_testMetricHelpers()
{
    NexusReportsModel model;

    QVariantList route;
    route << QVariantMap{{QStringLiteral("latitude"), 0.0}, {QStringLiteral("longitude"), 0.0}}
          << QVariantMap{{QStringLiteral("latitude"), 0.0}, {QStringLiteral("longitude"), 0.001}};
    const double distance = model.routeDistanceMeters(route);
    QVERIFY(!qIsNaN(distance));
    QVERIFY(distance > 100.0);
    QVERIFY(distance < 120.0);

    const QVariantList altitudeSamples{
        QVariantMap{{QStringLiteral("y"), 10.0}},
        QVariantMap{{QStringLiteral("y"), 42.5}},
        QVariantMap{{QStringLiteral("y"), 31.0}}
    };
    QCOMPARE(model.maxSampleValue(altitudeSamples), 42.5);

    const QVariantList batterySamples{
        QVariantMap{{QStringLiteral("y"), 96.0}},
        QVariantMap{{QStringLiteral("y"), 72.0}}
    };
    QCOMPARE(model.batteryUsedPercent(batterySamples), 24.0);
}
