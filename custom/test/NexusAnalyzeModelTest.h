#pragma once

#include "UnitTest.h"

class NexusAnalyzeModelTest final : public UnitTest
{
    Q_OBJECT

private slots:
    void _testHistoryContract();
    void _testSelectionValidation();
};
