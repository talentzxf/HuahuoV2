#pragma once

#include "Include/Cpp/Atomic.h"
#include <cmath>


/*  CLOCK_MONOTONIC is the most reliable clock type on Android. However, it
    does not tick when the device is sleeping (case 867885, case 1037712).

    CLOCK_BOOTTIME includes that time, but is very unreliable. Some older
    devices had this time ticking back or jumping back and forth (case 970945)

    To fix this issue we combine both clocks to produce a CLOCK_MONOTONIC-based
    clock that ticks even when the device is disabled.
*/
class TimeSinceStartupMonotonicBoottimeClockCombiner
{
public:

    TimeSinceStartupMonotonicBoottimeClockCombiner(double brokenBoottimeDetectionHysteresis = 0.001,
                                                   double adjustmentHysteresisWhenBootimeGood = 0.001, double adjustmentHysteresisWhenBootimeBroken = 8) :
        m_MonotonicStartTime(-HUGE_VAL),
        m_BoottimeStartTime(-HUGE_VAL),
        m_BoottimeAdjustment(0.0),
        m_IsBoottimeBroken(false),
        m_BrokenBoottimeDetectionHysteresis(brokenBoottimeDetectionHysteresis),
        m_AdjustmentHysteresisWhenBootimeGood(adjustmentHysteresisWhenBootimeGood),
        m_AdjustmentHysteresisWhenBootimeBroken(adjustmentHysteresisWhenBootimeBroken)
    {
    }

    // this function is thread-safe
    double GetTimeSinceStartup(double currentMonotonicTime, double currentBoottimeTime)
    {
        const double monotonicSinceStart = currentMonotonicTime - GetStartTime(m_MonotonicStartTime, currentMonotonicTime);
        const double boottimeSinceStart = currentBoottimeTime - GetStartTime(m_BoottimeStartTime, currentBoottimeTime);

        /*  In theory, boottime can only go faster than monotonic, so whenever we detect
            this condition we assume that device was asleep and we must adjust the returned
            time by the amount of time that the boottime jumped forwards.

            In the real world, boottime can go slower than monotonic or even backwards.
            We work around this by only taking into account the total difference between
            boottime and monotonic times and only adjusting monotonic time when this difference
            increases.

            There's also a problem that on some devices the boottime continuously jumps
            forwards and backwards by ~4 seconds. This means that a naive implementation would
            often do more than one time jump after device sleeps, depending on which part
            of the jump "cycle" we landed. We work around this by introducing hysteresis of
            hysteresisSeconds seconds and adjusting monotonic time only when this adjustment
            changes by more than hysteresisSeconds amount, but only on broken devices.

            On devices with broken CLOCK_BOOTTIME behavior this would ignore device sleeps of
            hysteresisSeconds or less, which is small compromise to make.
        */

        const bool boottimeHasJumpedBack = boottimeSinceStart - monotonicSinceStart < -m_BrokenBoottimeDetectionHysteresis;
        if (boottimeHasJumpedBack)
            m_IsBoottimeBroken.store(true, baselib::memory_order_relaxed);

        const bool isBoottimeBroken = m_IsBoottimeBroken.load(baselib::memory_order_relaxed);
        const double hysteresisSeconds = isBoottimeBroken ? m_AdjustmentHysteresisWhenBootimeBroken : m_AdjustmentHysteresisWhenBootimeGood;

        // the code below is just thread-safe version of the following:
        // if (boottimeSinceStart - monotonicSinceStart > m_BoottimeAdjustment + hysteresisSeconds)
        //     m_BoottimeAdjustment = boottimeSinceStart - monotonicSinceStart;
        double boottimeAdjustmentValue = m_BoottimeAdjustment.load(baselib::memory_order_relaxed);
        while (boottimeSinceStart - monotonicSinceStart > boottimeAdjustmentValue + hysteresisSeconds)
        {
            baselib::atomic<double> wantedAdjustment(boottimeSinceStart - monotonicSinceStart);
            if (m_BoottimeAdjustment.compare_exchange_weak(boottimeAdjustmentValue, wantedAdjustment, baselib::memory_order_relaxed, baselib::memory_order_relaxed))
            {
                boottimeAdjustmentValue = wantedAdjustment;
                break;
            }
        }

        return monotonicSinceStart + boottimeAdjustmentValue;
    }

private:
    // Initializes the start time if it is not set and returns it.
    double GetStartTime(baselib::atomic<double>& startTime, double currentTime)
    {
        double startTimeValue = startTime.load(baselib::memory_order_relaxed);

        if (startTimeValue != -HUGE_VAL)
            return startTimeValue;

        // We don't want to replace startTime if it has been already changed.
        while (!startTime.compare_exchange_weak(startTimeValue, currentTime, baselib::memory_order_relaxed, baselib::memory_order_relaxed))
        {
            if (startTimeValue != -HUGE_VAL)
                return startTimeValue;
        }
        return currentTime;
    }

    baselib::atomic<double> m_MonotonicStartTime;
    baselib::atomic<double> m_BoottimeStartTime;
    baselib::atomic<double> m_BoottimeAdjustment;
    baselib::atomic<bool> m_IsBoottimeBroken;

    const double m_BrokenBoottimeDetectionHysteresis;
    const double m_AdjustmentHysteresisWhenBootimeGood;
    const double m_AdjustmentHysteresisWhenBootimeBroken;
};
