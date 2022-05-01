//
// Created by VincentZhang on 4/28/2022.
//

#include "GameObjectUtility.h"
#include "TypeSystem/ObjectDefines.h"
#include "Components/Transform/Transform.h"
#include "BaseClasses/PPtr.h"

#define ErrorFormatChecked(s, ...) \
do { if (s) { *s = Format (__VA_ARGS__); } } while(0)

void ActivateGameObject(GameObject& go, const std::string& name)
{
    go.Reset();
    go.SetName(name.c_str());
    go.AwakeFromLoad(kInstantiateOrCreateFromCodeAwakeFromLoad);
    go.Activate();
}

struct AddComponentData
{
    const HuaHuo::Type* m_Type;
    // ScriptingClassPtr m_Class;

    AddComponentData(const HuaHuo::Type* type/*, ScriptingClassPtr script*/)
            : m_Type(type)
            //, m_Class(script)
    {}
};

static bool CollectAllComponentsWithoutAdding(
        GameObject& go,
        const HuaHuo::Type* componentType,
//        ScriptingClassPtr klass,
        std::vector<AddComponentData>& components,
        std::vector<AddComponentData>& processed,
        std::string* error)
{
    // Make sure the user isn't calling AddComponent from OnDestroy().
    if (go.IsDestroying())
    {
        ErrorFormatChecked(error, "Can't add component to object that is being destroyed.");
        return false;
    }

    // Are we actually derived from go component?
    if (!componentType->IsDerivedFrom<BaseComponent>())
    {
        ErrorFormatChecked(error, "Can't add component because '%s' is not derived from Component.", componentType->GetName());
        return false;
    }

//    // We can't add a component if it has conflicting components.
//    // This is included when checking if we can add a component but we want to give a specific reason here i.e. a component conflict.
//    BaseComponent* conflictingComponent = FindConflictingComponent(go, componentType);
//    if (conflictingComponent)
//    {
//        ErrorFormatChecked(error, "Can't add component '%s' to %s because it conflicts with the existing '%s' derived component!",
//                           componentType->GetName(), go.GetName(), conflictingComponent->GetTypeName());
//        return false;
//    }
//
//    // Find conflict in the processed list.
//    const HuaHuo::Type* conflictType = FindConflictingAddComponentData(processed, componentType);
//    if (conflictType != NULL)
//    {
//        ErrorFormatChecked(error, "Can't add component '%s' to %s because it conflicts with the '%s' derived component!",
//                           componentType->GetName(), go.GetName(), conflictType->GetName());
//        return false;
//    }
//
//    // We can't add a component if we are already inserted and the component doesn't allow multiple insertion!
//    if (!CanAddComponent(go, componentType))
//    {
//        ErrorFormatChecked(error, "Can't add component '%s' to %s because such a component is already added to the game object!",
//                           componentType->GetName(), go.GetName());
//        return false;
//    }

#if UNITY_EDITOR
    if (IsPartOfImmutablePrefab(go) && IsPartOfPrefabAsset(go))
    {
        ErrorFormatChecked(error, "Can't add component '%s' to %s because the game object is a generated prefab and can only be modified through an AssetPostprocessor.",
            componentType->GetName(), go.GetName());
        return false;
    }

    bool isPrefab = IsPartOfAnyPrefab(go);
    if (isPrefab)
    {
        Transform& t = go.GetComponent<Transform>();
        if (componentType->IsDerivedFrom<Transform>() && (componentType != t.GetType()))
        {
            ErrorFormatChecked(error, "Changing Transform on a Prefab instance (%s) is not allowed.", go.GetName());
            return false;
        }
    }
#endif

    // this means user is trying to add abstract class directly (AddComponent("Collider"))
    if (componentType->IsAbstract())
    {
        ErrorFormatChecked(error, "Cannot add component of type '%s' because it is abstract. Add component of type that is derived from '%s' instead.", componentType->GetName(), componentType->GetName());
        return false;
    }

    // Add to the processed before the required components.
    processed.push_back(AddComponentData(componentType/*, klass*/));

//    if (!CollectRequiredComponents(go, componentType, components, processed, error))
//        return false;
//
//    // Find all required components for a script and add them before!
//    if (IManagedObjectHost::IsTypeAHost(*componentType) && klass != SCRIPTING_NULL)
//    {
//        MonoScriptPtr script = GetMonoScriptManager().FindRuntimeScript(klass);
//
//#if UNITY_EDITOR
//        if (!script)
//            script = GetMonoScriptManager().FindEditorScript(klass);
//#endif
//
//        if (script && !ValidateScriptComponent(script, error))
//            return false;
//
//        const char* className = script != NULL ? script->GetScriptClassName().c_str() : "<NULL>";
//
//        AssertFormatMsg(klass != SCRIPTING_NULL, "Failed to find script class '%s' while adding it to %s",
//                        className,
//                        go.GetName());
//
//        ScriptingClassPtr baseklass = NULL;
//        ScriptingClassPtr conflictingklass = NULL;
//        if (!CanAddScript(go, klass, &baseklass, &conflictingklass))
//        {
//            if (error)
//            {
//                if (baseklass == conflictingklass)
//                    *error = Format("Can't add '%s' to %s because a '%s' is already added to the game object!", className, go.GetName(), scripting_class_get_name(conflictingklass));
//                else
//                    *error = Format("Can't add '%s' to %s because a '%s' is already added to the game object!\nA GameObject can only contain one '%s' component.", className, go.GetName(), scripting_class_get_name(conflictingklass), scripting_class_get_name(baseklass));
//            }
//            return false;
//        }
//
//        if (!CollectRequiredScriptComponents(go, klass, components, processed, error))
//            return false;
//    }

    // Add the component after the required components.
    components.push_back(AddComponentData(componentType/*, klass*/));

    return true;
}

static bool CollectComponentsWithoutAdding(
        GameObject& go,
        const HuaHuo::Type* componentType,
        // ScriptingClassPtr klass,
        std::vector<AddComponentData>& components,
        std::vector<AddComponentData>& processed,
        std::string* error,
        bool requiredComponentsOnly = false)
{
#if UNITY_EDITOR
    if (requiredComponentsOnly)
        return CollectRequiredComponentsWithoutAdding(go, componentType, klass, components, processed, error);
    else
#endif
    return CollectAllComponentsWithoutAdding(go, componentType, /*klass,*/ components, processed, error);
}

static BaseComponent* ProduceComponentFromCode(const HuaHuo::Type* type, std::string *error)
{
    BaseComponent* component = BaseComponent::Produce(type);

    if (component == NULL)
    {
        ErrorFormatChecked(error, "Failure to create component of type '%s' (0x%08X)", type->GetName(), type->GetPersistentTypeID());
        return NULL;
    }

    Assert(component->Is<BaseComponent>());

    component->Reset();

    return component;
}

static inline void InitializeComponentCreatedFromCode(BaseComponent* component)
{
    // AssertMsg(dynamic_pptr_cast<MonoBehaviour*>(component) == NULL, "InitializeMonoBehaviourCreatedFromCode should be called for MonoBehaviours");

    component->Reset();
    component->SmartReset();
#if UNITY_EDITOR
    InvokeAddComponentSetDefaultCallback(*component);
    component->CheckConsistency();
#endif
}

static inline void FinalizeComponentCreationFromCode(BaseComponent* component)
{
    component->AwakeFromLoad(kInstantiateOrCreateFromCodeAwakeFromLoad);
    component->SetDirty();
}

static BaseComponent* AddComponentUnchecked(GameObject& go, const HuaHuo::Type* componentType, /*ScriptingClassPtr klass, MonoScriptPtr script,*/ std::string* error/*, AwakeFromLoadQueue* queue = nullptr*/)
{
    if (componentType == NULL)
        return NULL;

    // Are we adding a transform-derived component?
    const bool isTransform = componentType->IsDerivedFrom<Transform>();
    if (isTransform)
    {
        // Cannot replace one transform with another!
        if (componentType == TypeOf<Transform>() && go.QueryComponentByExactType(TypeOf<Transform>()))
        {
            ErrorFormatChecked(error, "Can't add a %s component because one is already added.", componentType->GetName());
            return NULL;
        }
    }

    BaseComponent* component = ProduceComponentFromCode(componentType, error);

    if (component == NULL)
        return NULL;

    if (isTransform)
    {
        bool hasTransform = (go.QueryComponent<Transform>() != NULL);
        Transform* transform = dynamic_pptr_cast<Transform*>(component);

        if (hasTransform)
        {
            go.ReplaceTransformComponentInternal(transform/*, queue*/);
            transform->ResetReplacement();
        }
        else
        {
            go.AddFirstTransformComponentInternal(transform/*, queue*/);
            InitializeComponentCreatedFromCode(transform);
        }
    }
    else
    {
        go.AddComponentInternal(component, true/*, queue*/);

//        if (IManagedObjectHost::IsObjectsTypeAHost(component))
//        {
//            if (!SetupScriptForIManagedObjectHost(component, klass, script))
//                return NULL;
//
//            if (!InitializeIManagedObjectHostCreatedFromCode(component))
//                return NULL;
//        }
//        else
        {
            InitializeComponentCreatedFromCode(component);
        }
    }

    // if (queue == nullptr)
    {
        // go.SendMessage(kDidAddComponent, component);

        FinalizeComponentCreationFromCode(component);

        // InvokeAddComponentCallback(*component);
    }

    return component;
}

BaseComponent* AddComponent(GameObject& go, const HuaHuo::Type* componentType, /*ScriptingClassPtr klass,*/ std::string* error, /*AwakeFromLoadQueue* queue,*/ const char* componentName)
{
    // PROFILER_AUTO(gAddComponentProf, &go);

//    std::vector<AddComponentData> components(kMemTempAlloc);
//    std::vector<AddComponentData> processed(kMemTempAlloc);

    std::vector<AddComponentData> components;
    std::vector<AddComponentData> processed;

    if (componentType == NULL)
        return NULL;

    if (!CollectComponentsWithoutAdding(go, componentType, /*klass,*/ components, processed, error/*, queue*/))
        return NULL;

    if (components.size() <= 0)
        return NULL;

    // Just simply add component without check.
    BaseComponent* addedComponent = NULL;
    for (unsigned int i = 0; i < components.size(); ++i)
    {
        AddComponentData& component = components[i];

        if (error)
        {
            std::string tempError;
            addedComponent = AddComponentUnchecked(go, component.m_Type, /*component.m_Class,*/ &tempError/*,queue*/);
            if (!tempError.empty())
                *error += tempError;
        }
        else
        {
            addedComponent = AddComponentUnchecked(go, component.m_Type, /* component.m_Class,*/ NULL/*, queue*/);
        }

#if UNITY_EDITOR
        if (queue)
        {
            const char* createdComponentName = !component.m_Class ? component.m_Type->GetName() : GetMonoScriptManager().FindRuntimeScript(component.m_Class)->GetScriptClassName().c_str();

            if (addedComponent)
                WarningStringObject(Format("Creating missing %s component for %s in %s.", createdComponentName, componentName, go.GetName()), go);
            else
                ErrorStringObject(Format("Unable to add missing %s component for %s in %s.", createdComponentName, componentName, go.GetName()), go);
        }
#else
        //DebugAssert(queue == nullptr);
#endif
    }

    return addedComponent;
}

BaseComponent* AddComponent(GameObject& go, const char* name, std::string* error/*, AwakeFromLoadQueue* queue*/)
{
    const char* fullName = name;

//    if (BeginsWith(name, "UnityEngine."))
//        name += strlen("UnityEngine.");

    const HuaHuo::Type* componentType = HuaHuo::Type::FindTypeByName(name);
    if (componentType != NULL && componentType->IsDerivedFrom<BaseComponent>())
    {
        // return AddComponent(go, componentType, NULL, error, queue, queue ? fullName : NULL);
        return AddComponent(go, componentType, error, /*queue,*/ fullName);
    }
//    else
//    {
//        MonoScriptPtr script = GetMonoScriptManager().FindRuntimeScript(fullName);
//
//        if (script)
//        {
//            return AddComponent(go, TypeOf<MonoBehaviour>(), script->GetClass(), error, queue, queue ? script->GetScriptClassName().c_str() : NULL);
//        }
//
//        if (error)
//        {
//            if (componentType == NULL)
//                *error = Format("Can't add component because class '%s' doesn't exist!", name);
//            else
//                *error = Format("Can't add component because '%s' is not derived from Component.", name);
//        }
//
//        return NULL;
//    }
}

// varargs can only be passed around by passing the va_list, so caller is responsible for calling va_start/va_end
void AddComponentsFromVAList(GameObject& go, const char* componentName, va_list componentList)
{
    if (componentName == NULL)
        return;

    std::string error;
    if (AddComponent(go, componentName, &error) == NULL)
        ErrorString(error);

    while (true)
    {
        const char* cur = va_arg(componentList, const char*);
        if (cur == NULL)
            break;
        if (AddComponent(go, cur, &error) == NULL)
            ErrorString(error);
    }
}

GameObject& CreateGameObject(const std::string& name, const char* componentName, ...)
{
    // Create game object with name!
    GameObject &go = *NEW_OBJECT(GameObject);

    ActivateGameObject(go, name);

    // Add components with class names!
    va_list ap;
    va_start(ap, componentName);
    AddComponentsFromVAList(go, componentName, ap);
    va_end(ap);

    return go;
}

GameObject& CreateGameObjectWithVAList(const std::string& name, const char* componentName, va_list list)
{
    va_list componentList;
    va_copy(componentList, list);
    GameObject &go = *NEW_OBJECT(GameObject);

    ActivateGameObject(go, name);
    AddComponentsFromVAList(go, componentName, componentList);
    va_end(componentList);

    return go;
}
