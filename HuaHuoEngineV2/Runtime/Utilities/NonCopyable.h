#pragma once

#include "Modules/ExportModules.h"

class EXPORT_COREMODULE NonCopyable
{
public:
    NonCopyable() {}

private:
    NonCopyable(const NonCopyable&);
    NonCopyable& operator=(const NonCopyable&);
};
