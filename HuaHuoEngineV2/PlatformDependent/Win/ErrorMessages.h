#pragma once

namespace winutils
{
    void AddErrorMessage(const char* fmt, ...);
    const core::string& GetErrorMessages();
}
