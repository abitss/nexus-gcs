#pragma once

#include "QmlUITestBase.h"

class QQuickItem;
class QSignalSpy;

class NexusFlightCockpitUITest final : public QmlUITestBase
{
    Q_OBJECT

private slots:
    void _testDisconnectedLandscape();
    void _testPX4TelemetryActionsAndReconnect();

private:
    bool _itemInsideWindow(const QString& objectName);
    bool _itemsDoNotOverlapVertically(const QString& upperName, const QString& lowerName);
    bool _confirmVisibleGuidedAction();
    static bool _acceptedAckSince(const QSignalSpy& spy, int command, int startIndex);
};
