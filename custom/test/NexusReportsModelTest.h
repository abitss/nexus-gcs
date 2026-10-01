#pragma once

#include "UnitTest.h"

class NexusReportsModelTest final : public UnitTest
{
    Q_OBJECT

private slots:
    void _testLocalMetadataPersistence();
    void _testReportSchema();
};
