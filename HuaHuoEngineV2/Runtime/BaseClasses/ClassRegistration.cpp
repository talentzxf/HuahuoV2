//
// Created by VincentZhang on 4/11/2022.
//

#include "ClassRegistration.h"
#include "TypeSystem/TypeManager.h"
#include "TypeSystem/ObjectDefines.h"
#include "GameObject.h"
#include "GameManager.h"
#include "Input/TimeManager.h"
#include "ObjectStore.h"
#include "Shapes/LineShape.h"
#include "Shapes/CircleShape.h"
#include "KeyFrames/FrameState.h"
#include "KeyFrames/ShapeTransformComponent.h"
#include "Shapes/RectangleShape.h"
#include "Shapes/ImageShape.h"
#include "Shapes/AudioShape.h"
#include "Shapes/AbstractMediaShape.h"
#include "Shapes/ElementShape.h"
#include "Shapes/TextShape.h"
#include "Shapes/CurveShape.h"
#include "Shapes/MirrorShape.h"
#include "Shapes/NailShape.h"
#include "Shapes/ParticleSystem.h"
#include "KeyFrames/CustomFrameState.h"
#include "KeyFrames/CustomComponent.h"
#include "KeyFrames/EventGraphComponent.h"
#include "ResourceManager.h"

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
    RegisterHuaHuoClass<LevelGameManager>("Core");
    RegisterHuaHuoClass<GlobalGameManager>("Core");
    RegisterHuaHuoClass<GameManager>("Core");
    RegisterHuaHuoClass<TimeManager>("Core");

    RegisterHuaHuoClass<ObjectStoreManager>("ObjectStore");
    RegisterHuaHuoClass<ObjectStore>("ObjectStore");
    RegisterHuaHuoClass<Layer>("ObjectStore");
    RegisterHuaHuoClass<BaseShape>("ObjectStore");
    RegisterHuaHuoClass<LineShape>("ObjectStore");
    RegisterHuaHuoClass<RectangleShape>("ObjectStore");
    RegisterHuaHuoClass<CircleShape>("ObjectStore");
    RegisterHuaHuoClass<AbstractMediaShape>("ObjectStore");
    RegisterHuaHuoClass<ImageShape>("ObjectStore");
    RegisterHuaHuoClass<TextShape>("ObjectStore");
    RegisterHuaHuoClass<MirrorShape>("ObjectStore");
    RegisterHuaHuoClass<AudioShape>("ObjectStore");
    RegisterHuaHuoClass<ElementShape>("ObjectStore");
    RegisterHuaHuoClass<NailShape>("ObjectStore");
    RegisterHuaHuoClass<ParticleSystem>("ObjectStore");

    RegisterHuaHuoClass<TimeLineCellManager>("ObjectStore");
    RegisterHuaHuoClass<NailManager>("ObjectStore");

    RegisterHuaHuoClass<AbstractFrameState>("ObjectStore");
    RegisterHuaHuoClass<ShapeTransformComponent>("ObjectStore");
    RegisterHuaHuoClass<ShapeSegmentFrameState>("ObjectStore");
    RegisterHuaHuoClass<CurveShape>("ObjectStore");
    RegisterHuaHuoClass<CustomFrameState>("ObjectStore");
    RegisterHuaHuoClass<CustomComponent>("ObjectStore");
    RegisterHuaHuoClass<EventGraphComponent>("ObjectStore");
    RegisterHuaHuoClass<BinaryResource>("ObjectStore");

    RegisterHuaHuoClass<ResourceManager>("ObjectStore");
}