#pragma once

#include "VehicleTest.h"

class NexusHealthModelTest final : public VehicleTest
{
    Q_OBJECT

private slots:
    void _testNominalAndSensorFailure();
    void _testDatalinkLossAndRecovery();
};
