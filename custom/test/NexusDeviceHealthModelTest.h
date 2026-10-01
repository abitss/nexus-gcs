#pragma once

#include "UnitTest.h"

class NexusDeviceHealthModelTest final : public UnitTest
{
    Q_OBJECT

private slots:
    void _testThresholdContracts();
    void _testPlatformStateContract();
    void _testWarningThresholds();
};
