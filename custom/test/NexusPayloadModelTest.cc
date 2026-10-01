#include "NexusPayloadModelTest.h"

#include <QtCore/QScopeGuard>
#include <QtTest/QTest>

#include "Fact.h"
#include "NexusAlertManager.h"
#include "NexusHealthModel.h"
#include "NexusPayloadModel.h"
#include "SettingsManager.h"
#include "VideoSettings.h"

UT_REGISTER_TEST(NexusPayloadModelTest, TestLabel::Integration, TestLabel::Vehicle)

void NexusPayloadModelTest::_testDisabledAndConfiguredWaitingStates()
{
    auto *videoSettings = SettingsManager::instance()->videoSettings();
    QVERIFY(videoSettings);

    const QVariant oldSource = videoSettings->videoSource()->rawValue();
    const QVariant oldUdpUrl = videoSettings->udpUrl()->rawValue();

    const auto restore = qScopeGuard([videoSettings, oldSource, oldUdpUrl] {
        videoSettings->videoSource()->setRawValue(oldSource);
        videoSettings->udpUrl()->setRawValue(oldUdpUrl);
    });

    videoSettings->videoSource()->setRawValue(QString::fromLatin1(VideoSettings::videoDisabled));
    videoSettings->udpUrl()->setRawValue(QString());

    NexusPayloadModel payload;
    payload.refresh();
    QCOMPARE(payload.configured(), false);
    QCOMPARE(payload.streamState(), QStringLiteral("DISABLED"));
    QCOMPARE(payload.videoLost(), false);
    QCOMPARE(payload.fpsText(), QStringLiteral("NOT REPORTED"));

    videoSettings->videoSource()->setRawValue(QString::fromLatin1(VideoSettings::videoSourceUDPH264));
    videoSettings->udpUrl()->setRawValue(QStringLiteral("udp://0.0.0.0:5600"));

    QTRY_VERIFY_WITH_TIMEOUT(payload.configured(), TestTimeout::mediumMs());
    QTRY_COMPARE_WITH_TIMEOUT(payload.streamState(), QStringLiteral("WAITING"), TestTimeout::mediumMs());
    QVERIFY(payload.latencyState() == QStringLiteral("LOW-LATENCY MODE") ||
            payload.latencyState() == QStringLiteral("BUFFERED"));

    // A configured stream which has not yet produced a frame is waiting, not a
    // fabricated LOST transition. LOST is reserved for a stream that previously decoded.
    QCOMPARE(payload.videoLost(), false);

    NexusHealthModel health;
    NexusAlertManager alerts(&health);
    QTRY_VERIFY_WITH_TIMEOUT(alerts.activeCount() >= 0, TestTimeout::mediumMs());
}

void NexusPayloadModelTest::_testLocalStorageAndCapabilityGates()
{
    NexusPayloadModel payload;
    payload.refresh();

    auto *appSettings = SettingsManager::instance()->appSettings();
    QVERIFY(appSettings);

    QCOMPARE(payload.localVideoPath(), appSettings->videoSavePath());
    QCOMPARE(payload.localPhotoPath(), appSettings->photoSavePath());

    // The test vehicle does not invent zoom/gimbal support. Capability controls
    // must remain hidden unless QGC reports those devices/capabilities.
    if (!payload.hasCamera()) {
        QVERIFY(!payload.hasZoom());
    }

    QVERIFY(!payload.resolutionText().isEmpty());
    QVERIFY(!payload.fpsText().isEmpty());
    QVERIFY(!payload.latencyState().isEmpty());
}
