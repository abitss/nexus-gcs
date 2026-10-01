#pragma once

#include "MissionTest.h"

class NexusPlanEditingTest final : public OfflineMissionTest
{
    Q_OBJECT

private slots:
    void _testCoreEditingAndPatrolRoute();
    void _testCorridorOrbitFenceRallyAndTerrain();
};

class NexusPlanVerificationTest final : public MissionTest
{
    Q_OBJECT

private slots:
    void _testUploadDownloadReadbackVerification();
};
