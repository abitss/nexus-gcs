#pragma once

#include "UnitTest.h"

class NexusPX4PlanSITLTest final : public UnitTest
{
    Q_OBJECT

private slots:
    void _testMissionUploadReadback();
};
