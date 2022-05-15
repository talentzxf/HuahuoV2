#include "UnityPrefix.h"
#include "JSBridge.h"
#include "Runtime/Utilities/URLUtility.h"

void OpenURL(const core::string& urlString)
{
    JS_Eval_OpenURL(urlString.c_str());
}
