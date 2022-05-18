//
// Created by VincentZhang on 4/11/2022.
//

#include "ClassRegistration.h"
#include "TypeSystem/TypeManager.h"
#include "TypeSystem/ObjectDefines.h"
#include "GameObject.h"
#include "Camera/Camera.h"
#include "GameManager.h"
#include "Camera/GraphicsSettings.h"
#include "Input/TimeManager.h"
#include "Graphics/Renderer.h"

void RegisterBuiltinTypes()
{
#define HUAHUO_KERNEL_CLASS_1(x)
#define HUAHUO_KERNEL_CLASS_2(ns, x)
#define HUAHUO_KERNEL_BUILTIN(x, persistentTypeID) \
    { \
        TypeManager::Get().RegisterNonObjectType(static_cast<PersistentTypeID>(persistentTypeID), &TypeContainer<x>::rtti, #x, ""); \
    }
#define HUAHUO_KERNEL_NONHIERARCHY_CLASS(x, persistentTypeID) HUAHUO_KERNEL_BUILTIN(x, persistentTypeID)
#define HUAHUO_KERNEL_STRUCT(x, persistentTypeID) HUAHUO_KERNEL_BUILTIN(x, persistentTypeID)
#include "ClassRegistration.inc.h"
#undef HUAHUO_KERNEL_STRUCT
#undef HUAHUO_KERNEL_NONHIERARCHY_CLASS
#undef HUAHUO_KERNEL_BUILTIN
#undef HUAHUO_KERNEL_CLASS_1
#undef HUAHUO_KERNEL_CLASS_2
}

class Transform;
class BaseComponent;
template <> void RegisterHuaHuoClass<Transform>(const char* module);
template <> void RegisterHuaHuoClass<BaseComponent>(const char* module);
void RegisterAllClasses(){
    RegisterBuiltinTypes();

    // VZ:
    RegisterHuaHuoClass<Transform>("Core");
    RegisterHuaHuoClass<BaseComponent>("Core");
    RegisterHuaHuoClass<GameObject>("Core");
    RegisterHuaHuoClass<Camera>("Core");
    RegisterHuaHuoClass<Behaviour>("Core");
    RegisterHuaHuoClass<LevelGameManager>("Core");
    RegisterHuaHuoClass<GlobalGameManager>("Core");
    RegisterHuaHuoClass<GameManager>("Core");
    RegisterHuaHuoClass<GraphicsSettings>("Core");
    RegisterHuaHuoClass<TimeManager>("Core");
    RegisterHuaHuoClass<Renderer>("Core");
}