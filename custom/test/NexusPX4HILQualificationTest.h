#pragma once
#include "UnitTest.h"

class QJsonArray;
class QSignalSpy;

class NexusPX4HILQualificationTest final : public UnitTest
{
    Q_OBJECT
private slots:
    void _testHILLifecycle();
private:
    static bool _acceptedAckSince(const QSignalSpy &spy, int command, int startIndex);
};
