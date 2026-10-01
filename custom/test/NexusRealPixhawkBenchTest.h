#pragma once

#include "UnitTest.h"

class QJsonObject;
class QSignalSpy;

class NexusRealPixhawkBenchTest final : public UnitTest
{
    Q_OBJECT

private slots:
    void _testRealPixhawkBenchQualification();

private:
    static bool _acceptedAckSince(const QSignalSpy &spy, int command, int startIndex);
    static void _writeEvidence(const QJsonObject &root);
};
