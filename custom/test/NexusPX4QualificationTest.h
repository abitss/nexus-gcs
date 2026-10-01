#pragma once

#include "UnitTest.h"

class QJsonArray;
class QSignalSpy;

class NexusPX4QualificationTest final : public UnitTest
{
    Q_OBJECT

private slots:
    void _testFullPX4Qualification();

private:
    static bool _acceptedAckSince(const QSignalSpy &spy, int command, int startIndex);
    static void _writeEvidence(const QJsonArray &stages);
};
