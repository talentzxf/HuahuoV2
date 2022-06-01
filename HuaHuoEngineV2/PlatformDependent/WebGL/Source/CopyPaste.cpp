#include "UnityPrefix.h"
#include <string>
#include "Runtime/Utilities/CopyPaste.h"

static core::string copyPasteBuffer;

core::string GetCopyBuffer()
{
    return copyPasteBuffer;
}

void SetCopyBuffer(const core::string &utf8string)
{
    copyPasteBuffer = utf8string;
}
