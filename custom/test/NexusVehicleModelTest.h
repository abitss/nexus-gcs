#pragma once

#include "VehicleTest.h"

class NexusVehicleModelTest final : public VehicleTest
{
    Q_OBJECT

private slots:
    void _testConnectedSummaryAndSetupStates();
    void _testSafetyGates();
};
