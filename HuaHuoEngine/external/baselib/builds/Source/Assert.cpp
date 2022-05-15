#include <Include/Baselib.h>

#include <cstdio>
#include <cstdarg>

BASELIB_C_INTERFACE
{
    void detail_AssertLog(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        std::vfprintf(stderr, format, args);
        std::fflush(stderr);
        va_end(args);
    }
}
