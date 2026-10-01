#pragma once

#include "VehicleTest.h"

class NexusAlertManagerTest final : public VehicleTest
{
    Q_OBJECT

private slots:
    void _testTelemetryLossHistoryAndAck();
    void _testCommandRejectMissionFailureAndFailsafe();
};
