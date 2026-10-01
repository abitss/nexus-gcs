#pragma once

#include "UnitTest.h"

class QSignalSpy;

class NexusPX4SITLRuntimeTest final : public UnitTest
{
    Q_OBJECT

private slots:
    void _testPX4SITLFlightLifecycle();

private:
    static bool _acceptedAckSince(const QSignalSpy& spy, int command, int startIndex);
};
