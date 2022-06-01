#include "Include/Baselib.h"
#include "Include/C/Baselib_Timer.h"

BASELIB_C_INTERFACE
{
    static const Baselib_Timer_TickToNanosecondConversionRatio s_conversionRatio = Baselib_Timer_GetTicksToNanosecondsConversionRatio();
    const double Baselib_Timer_TickToNanosecondsConversionFactor = static_cast<double>(s_conversionRatio.ticksToNanosecondsNumerator) / s_conversionRatio.ticksToNanosecondsDenominator;
}
