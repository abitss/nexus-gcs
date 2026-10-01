#pragma once

#include "UnitTest.h"

class NexusOfflineModelTest final : public UnitTest
{
    Q_OBJECT

private slots:
    void _testLocalStorageContract();
    void _testNoCloudContract();
};
