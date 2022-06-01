#include "GameObjectExport.h"
#include "BaseClasses/GameObject.h"
#include "Misc/GameObjectUtility.h"

GameObject* MonoCreateGameObject(const char* name)
{
    std::string cname;
    if (!name)
    {
        cname = "New Game Object";
    }
    else
    {
        cname = name;
    }
    return &CreateGameObject(cname, "Transform", NULL);
}
//
//ScriptingObjectPtr MonoAddComponent(GameObject& go, const char* name)
//{
//    core::string error;
//    Unity::Component* component = AddComponent(go, name, &error);
//
//    if (component)
//        return Scripting::ScriptingWrapperFor(component);
//    else
//    {
//        LogStringObject(error, &go);
//        return SCRIPTING_NULL;
//    }
//}
//
//ScriptingObjectPtr MonoAddComponentWithType(GameObject& go, ScriptingSystemTypeObjectPtr systemTypeInstance)
//{
//    core::string error;
//
//    Unity::Component* component = NULL;
//    ScriptingClassPtr klass = scripting_class_from_systemtypeinstance(systemTypeInstance);
//
//    if (klass == SCRIPTING_NULL)
//    {
//        WarningStringObject("AddComponent asking for invalid type", &go);
//
//        return SCRIPTING_NULL;
//    }
//
//    if (klass == GetCoreScriptingClasses().monoBehaviour)
//    {
//        ErrorStringObject("AddComponent with MonoBehaviour is not allowed. Create a class that derives from MonoBehaviour and add it instead.", &go);
//        return SCRIPTING_NULL;
//    }
//
//    InstanceID instanceID = go.GetInstanceID();
//
//    ScriptingClassPtr baseManagedObjectKlass = IManagedObjectHost::FindOldestAncestorExtendingANativeClass(klass);
//    if (baseManagedObjectKlass != SCRIPTING_NULL)
//    {
//        MonoScript* script = GetMonoScriptManager().FindRuntimeScript(klass);
//#if UNITY_EDITOR
//        if (!script)
//            script = GetMonoScriptManager().FindEditorScript(klass);
//#endif
//        if (!script)
//            CreateMonoScriptFromScriptingType(klass);
//
//        const Unity::Type* componentType = Unity::Type::FindTypeByName(scripting_class_get_name(baseManagedObjectKlass));
//        if (componentType == NULL)
//        {
//            WarningStringObject("AddComponent asking for invalid type using [ExtensionOfNativeClass] in its ancestor but cannot be found in the assembly", &go);
//            return SCRIPTING_NULL;
//        }
//
//        component = AddComponent(go, componentType, klass, &error);
//    }
//    else
//    {
//        const Unity::Type* componentType = Unity::Type::FindTypeByName(scripting_class_get_name(klass));
//        if (componentType == NULL)
//        {
//            WarningStringObject(Format("AddComponent asking for \"%s\" which is not a Unity engine type.", scripting_class_get_name(klass)), &go);
//            return SCRIPTING_NULL;
//        }
//
//        component = AddComponent(go, componentType, NULL, &error);
//    }
//
//    if (component)
//        return Scripting::ScriptingWrapperFor(component);
//    else
//    {
//        // Check if the object is still valid ( could have been destroyed in Awake.)
//        // Don't log empty messages
//        if (!error.empty())
//            LogStringObject(error, PPtr<Object>(instanceID));
//        return SCRIPTING_NULL;
//    }
//}
