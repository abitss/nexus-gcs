#include "NexusFlightCockpitUITest.h"

#include <QtCore/QPointer>
#include <QtCore/QScopeGuard>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "MockLink.h"
#include "MultiVehicleManager.h"
#include "Vehicle.h"

UT_REGISTER_TEST(NexusFlightCockpitUITest, TestLabel::Integration, TestLabel::Vehicle)

bool NexusFlightCockpitUITest::_itemInsideWindow(const QString& objectName)
{
    QQuickItem* const item = findVisibleItem(_rootItem, objectName, 3000);
    if (!item || !_window) {
        return false;
    }

    item->ensurePolished();
    const QPointF topLeft = item->mapToScene(QPointF(0, 0));
    const QPointF bottomRight = item->mapToScene(QPointF(item->width(), item->height()));
    constexpr qreal epsilon = 1.0;

    return topLeft.x() >= -epsilon &&
           topLeft.y() >= -epsilon &&
           bottomRight.x() <= _window->width() + epsilon &&
           bottomRight.y() <= _window->height() + epsilon;
}

bool NexusFlightCockpitUITest::_itemsDoNotOverlapVertically(const QString& upperName, const QString& lowerName)
{
    QQuickItem* const upper = findVisibleItem(_rootItem, upperName, 3000);
    QQuickItem* const lower = findVisibleItem(_rootItem, lowerName, 3000);
    if (!upper || !lower) {
        return false;
    }

    upper->ensurePolished();
    lower->ensurePolished();

    const qreal upperBottom = upper->mapToScene(QPointF(0, upper->height())).y();
    const qreal lowerTop = lower->mapToScene(QPointF(0, 0)).y();
    return upperBottom <= lowerTop + 1.0;
}

bool NexusFlightCockpitUITest::_confirmVisibleGuidedAction()
{
    QQuickItem* const confirmButton = findVisibleItem(_rootItem, QStringLiteral("guidedActionConfirmButton"), 3000);
    if (!confirmButton) {
        return false;
    }
    return QMetaObject::invokeMethod(confirmButton, "activated");
}

bool NexusFlightCockpitUITest::_acceptedAckSince(const QSignalSpy& spy, int command, int startIndex)
{
    for (int i = startIndex; i < spy.count(); ++i) {
        const QList<QVariant> args = spy.at(i);
        if (args.size() >= 5 &&
            args.at(2).toInt() == command &&
            args.at(3).toInt() == MAV_RESULT_ACCEPTED) {
            return true;
        }
    }
    return false;
}

void NexusFlightCockpitUITest::_testDisconnectedLandscape()
{
    startUI();
    if (QTest::currentTestFailed()) {
        return;
    }

    _window->resize(1280, 800);
    QTest::qWait(300);

    QVERIFY(verifyProperty(QStringLiteral("nexusConnectionChip"), "value",
                           QStringLiteral("DISCONNECTED"), QStringLiteral("no-vehicle connection state")));
    QVERIFY(verifyProperty(QStringLiteral("nexusFlightStateText"), "text",
                           QStringLiteral("WAITING FOR VEHICLE"), QStringLiteral("no-vehicle flight state")));
    QVERIFY(verifyProperty(QStringLiteral("nexusAltMetric"), "value",
                           QStringLiteral("--"), QStringLiteral("no stale altitude without vehicle")));

    const QStringList requiredChrome = {
        QStringLiteral("nexusStatusRibbon"),
        QStringLiteral("nexusTelemetryBar"),
        QStringLiteral("nexusActionBar"),
        QStringLiteral("nexusBottomNav")
    };
    for (const QString& name : requiredChrome) {
        QVERIFY2(_itemInsideWindow(name), qPrintable(QStringLiteral("%1 is outside 1280x800 landscape viewport").arg(name)));
    }

    QVERIFY2(_itemsDoNotOverlapVertically(QStringLiteral("nexusStatusRibbon"), QStringLiteral("nexusTelemetryBar")),
             "Top status ribbon overlaps the bottom telemetry bar at 1280x800");
    QVERIFY2(_itemsDoNotOverlapVertically(QStringLiteral("nexusOpsPanel"), QStringLiteral("nexusTelemetryBar")),
             "Operations panel overlaps bottom telemetry at 1280x800");

    _window->resize(1024, 600);
    QTest::qWait(300);

    for (const QString& name : requiredChrome) {
        QVERIFY2(_itemInsideWindow(name), qPrintable(QStringLiteral("%1 is outside 1024x600 landscape viewport").arg(name)));
    }
    QVERIFY2(_itemInsideWindow(QStringLiteral("nexusOpsPanel")), "Operations panel is outside 1024x600 viewport");
    QVERIFY2(_itemsDoNotOverlapVertically(QStringLiteral("nexusOpsPanel"), QStringLiteral("nexusTelemetryBar")),
             "Operations panel overlaps bottom telemetry at 1024x600");
}

void NexusFlightCockpitUITest::_testPX4TelemetryActionsAndReconnect()
{
    startUI();
    if (QTest::currentTestFailed()) {
        return;
    }

    _window->resize(1280, 800);

    QPointer<MockLink> activeMock;
    const auto guard = qScopeGuard([this, &activeMock] {
        disconnectMockLink(activeMock);
        closeUIWindow();
        destroyUIEngine();
    });

    Vehicle* vehicle = nullptr;
    activeMock = connectMockLinkAndWaitReady([] { return MockLink::startPX4MockLink(); }, vehicle);
    QVERIFY2(activeMock && vehicle, "PX4 MockLink failed to reach ready state");

    QVERIFY(verifyProperty(QStringLiteral("nexusConnectionChip"), "value",
                           QStringLiteral("CONNECTED"), QStringLiteral("connected cockpit")));
    QVERIFY_TRUE_WAIT(vehicle->coordinate().isValid(), TestTimeout::longMs());
    QVERIFY_TRUE_WAIT(vehicle->homePosition().isValid(), TestTimeout::longMs());

    // Real QGC facts must propagate into the custom layer rather than synthetic UI values.
    QVERIFY_TRUE_WAIT(findVisibleItem(_rootItem, QStringLiteral("nexusAltMetric"), 0)->property("value").toString() != QStringLiteral("--"),
                      TestTimeout::longMs());
    QVERIFY_TRUE_WAIT(findVisibleItem(_rootItem, QStringLiteral("nexusHeadingMetric"), 0)->property("value").toString() != QStringLiteral("--"),
                      TestTimeout::longMs());

    QSignalSpy ackSpy(vehicle, &Vehicle::mavCommandResult);
    QVERIFY(ackSpy.isValid());

    // ARM through the NEXUS button -> stock QGC confirmation -> Vehicle command path.
    QQuickItem* armButton = findVisibleItem(_rootItem, QStringLiteral("nexusActionArm"), 5000);
    QVERIFY2(armButton, "NEXUS ARM action did not become visible");
    const int armAckStart = ackSpy.count();
    QVERIFY(clickButton(QStringLiteral("nexusActionArm")));
    QVERIFY2(_confirmVisibleGuidedAction(), "ARM confirmation control did not activate");
    QVERIFY_TRUE_WAIT(vehicle->armed(), TestTimeout::longMs());
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return _acceptedAckSince(ackSpy, MAV_CMD_COMPONENT_ARM_DISARM, armAckStart); },
                 TestTimeout::longMs(), QStringLiteral("ARM COMMAND_ACK accepted")),
             "No accepted COMMAND_ACK for ARM");

    QQuickItem* feedback = findVisibleItem(_rootItem, QStringLiteral("nexusCommandFeedbackText"), 3000);
    QVERIFY2(feedback, "NEXUS command feedback did not appear after ARM ACK");
    QVERIFY(feedback->property("text").toString().contains(QStringLiteral("ACCEPTED")));

    // TAKEOFF follows the same real QGC guided-action path.
    QQuickItem* takeoffButton = findVisibleItem(_rootItem, QStringLiteral("nexusActionTakeoff"), 5000);
    QVERIFY2(takeoffButton, "NEXUS TAKEOFF action did not become visible");
    const int takeoffAckStart = ackSpy.count();
    QVERIFY(clickButton(QStringLiteral("nexusActionTakeoff")));
    QVERIFY2(_confirmVisibleGuidedAction(), "TAKEOFF confirmation control did not activate");
    QVERIFY2(UnitTest::waitForCondition(
                 [&] { return _acceptedAckSince(ackSpy, MAV_CMD_NAV_TAKEOFF, takeoffAckStart); },
                 TestTimeout::longMs(), QStringLiteral("TAKEOFF COMMAND_ACK accepted")),
             "No accepted COMMAND_ACK for TAKEOFF");
    QVERIFY_TRUE_WAIT(vehicle->flying(), TestTimeout::longMs());

    // In-flight NEXUS actions must be visible when QGC says they are available.
    QVERIFY2(findVisibleItem(_rootItem, QStringLiteral("nexusActionHold"), 5000), "HOLD action not visible in flight");
    QVERIFY2(findVisibleItem(_rootItem, QStringLiteral("nexusActionRTL"), 5000), "RTL action not visible in flight");
    QVERIFY2(findVisibleItem(_rootItem, QStringLiteral("nexusActionLand"), 5000), "LAND action not visible in flight");

    // RTL and LAND are mode transitions on PX4; MockLink validates the UI-to-QGC path.
    QVERIFY(clickButton(QStringLiteral("nexusActionRTL")));
    QVERIFY2(_confirmVisibleGuidedAction(), "RTL confirmation control did not activate");
    QVERIFY_TRUE_WAIT(vehicle->flightMode() == vehicle->rtlFlightMode(), TestTimeout::longMs());

    QVERIFY(clickButton(QStringLiteral("nexusActionLand")));
    QVERIFY2(_confirmVisibleGuidedAction(), "LAND confirmation control did not activate");
    QVERIFY_TRUE_WAIT(vehicle->flightMode() == vehicle->landFlightMode(), TestTimeout::longMs());

    // Disconnect while the UI is alive. The cockpit must remove live-looking values.
    disconnectMockLink(activeMock);
    activeMock = nullptr;
    QVERIFY_TRUE_WAIT(MultiVehicleManager::instance()->activeVehicle() == nullptr, TestTimeout::longMs());
    QVERIFY(verifyProperty(QStringLiteral("nexusConnectionChip"), "value",
                           QStringLiteral("DISCONNECTED"), QStringLiteral("post-link-loss connection state")));
    QVERIFY(verifyProperty(QStringLiteral("nexusAltMetric"), "value",
                           QStringLiteral("--"), QStringLiteral("post-link-loss stale-data suppression")));

    // Reconnect a fresh PX4 link without restarting the app.
    Vehicle* reconnectedVehicle = nullptr;
    activeMock = connectMockLinkAndWaitReady([] { return MockLink::startPX4MockLink(); }, reconnectedVehicle);
    QVERIFY2(activeMock && reconnectedVehicle, "PX4 MockLink failed to reconnect");
    QVERIFY(verifyProperty(QStringLiteral("nexusConnectionChip"), "value",
                           QStringLiteral("CONNECTED"), QStringLiteral("reconnected cockpit")));
    QVERIFY_TRUE_WAIT(reconnectedVehicle->coordinate().isValid(), TestTimeout::longMs());
}
