#include "UnityPrefix.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Transform/Transform.h"
#include "Runtime/Scripting/ScriptingUtility.h"
#include "Runtime/Scripting/ScriptingManager.h"
#include "Runtime/Scripting/Scripting.h"
#include "Runtime/Scripting/ScriptingExportUtility.h"

#include <emscripten.h>

void SendMessageInternal(const char* object, const char* method, ScriptingObjectPtr value)
{
    Transform* transform = FindActiveTransformWithPath(object);
    if (transform)
    {
        bool didSend = Scripting::SendScriptingMessage(transform->GetGameObject(), method, value);
        if (!didSend)
            printf_console("SendMessage: object %s does not have receiver for function %s!\n", object, method);
    }
    else
    {
        printf_console("SendMessage: object %s not found!\n", object);
    }
}

extern "C" void SendMessageFloat(const char* object, const char* method, float value);
extern "C" void SendMessageString(const char* object, const char* method, const char* value);
extern "C" void SendMessage(const char* object, const char* method);
extern "C" void SetFullscreen(int fullscreen);

void EMSCRIPTEN_KEEPALIVE SendMessageFloat(const char* object, const char* method, float value)
{
    ScriptingObjectPtr paramObj = scripting_object_new(GetScriptingManager().GetCommonClasses().floatSingle);
    ExtractManagedObjectData<float>(paramObj) = value;
    SendMessageInternal(object, method, paramObj);
}

void EMSCRIPTEN_KEEPALIVE SendMessageString(const char* object, const char* method, const char* value)
{
    SendMessageInternal(object, method, scripting_string_new(value).ToScriptingObject());
}

void EMSCRIPTEN_KEEPALIVE SendMessage(const char* object, const char* method)
{
    SendMessageInternal(object, method, SCRIPTING_NULL);
}

void EMSCRIPTEN_KEEPALIVE SetFullscreen(int fullscreen)
{
    if (GetScreenManagerPtr())
        GetScreenManager().SetIsFullscreenImmediate(fullscreen);
}
