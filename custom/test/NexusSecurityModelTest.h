#pragma once
#include "UnitTest.h"

class NexusSecurityModelTest final : public UnitTest
{
    Q_OBJECT
private slots:
    void _testRoleAuthentication();
    void _testMissionValidation();
    void _testUpdateHashVerification();
    void _testAuditChain();
};
