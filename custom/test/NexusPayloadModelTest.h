#pragma once

#include "VehicleTest.h"

class NexusPayloadModelTest final : public VehicleTest
{
    Q_OBJECT

private slots:
    void _testDisabledAndConfiguredWaitingStates();
    void _testLocalStorageAndCapabilityGates();
};
