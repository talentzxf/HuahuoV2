#include "UnityPrefix.h"
#include "ErrorMessages.h"
#include "Runtime/Utilities/RuntimeStatic.h"

#include <cstdarg>

static RuntimeStatic<core::string> gErrorMessages(kMemSTL);

void winutils::AddErrorMessage(const char* fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    char buffer[2048];
    vsnprintf(buffer, 2048, fmt, args);

    // print it
    printf_console("%s\n", buffer);

    if (!gErrorMessages->empty())
        (*gErrorMessages) += "\r\n";
    (*gErrorMessages) += buffer;
}

const core::string& winutils::GetErrorMessages()
{
    return *gErrorMessages;
}
