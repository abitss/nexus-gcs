#pragma once

#include "VehicleTest.h"

class NexusPreflightModelTest final : public VehicleTest
{
    Q_OBJECT

private slots:
    void _testMissionStatesAndLinkLoss();
    void _testFailsafeBlocksPreflight();
    void _testNoVehicleBlocks();
};
