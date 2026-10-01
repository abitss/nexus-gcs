#pragma once

#include "VehicleTest.h"

class NexusEngineerModelTest final : public VehicleTest
{
    Q_OBJECT

private slots:
    void _testProtectedEntryAndRelock();
    void _testSnapshotsAndWriteGate();
};
