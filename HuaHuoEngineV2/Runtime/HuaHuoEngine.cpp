//
// Created by VincentZhang on 4/1/2022.
//

#include "HuaHuoEngine.h"
#include "BaseClasses/ClassRegistration.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "TypeSystem/TypeManager.h"
#include "Serialize/PathNamePersistentManager.h"
#include "BaseClasses/MessageHandler.h"
#include "Layer.h"

#ifdef HUAHUO_EDITOR

#include "BaseClasses/ManagerContextLoading.h"
#include "BaseClasses/ManagerContext.h"
#include "File/AsyncReadManager.h"

#endif

void HuaHuoEngine::InitEngine() {
    RegisterRuntimeInitializeAndCleanup::ExecuteInitializations();
    RegisterAllClasses();

    TypeManager::Get().InitializeAllTypes();

    MessageHandler::Get().Initialize(TypeOf<Object>());
    TypeManager::Get().CallInitializeTypes();

    TypeManager::Get().CallPostInitializeTypes();
    MessageHandler::Get().ResolveCallbacks();

    ManagerContextInitializeClasses();
#ifdef HUAHUO_EDITOR
    CreateMissingGlobalGameManagers();
#endif

    InitPathNamePersistentManager();
    InitializeAsyncReadManager();

}

HuaHuoEngine *HuaHuoEngine::gInstance = new HuaHuoEngine();

HuaHuoEngine::HuaHuoEngine() {
}

BaseShape *HuaHuoEngine::DuplicateShape(BaseShape *object) {
    if (object == NULL || object->GetType() == NULL)
        return NULL;

    if (!object->GetType()->IsDerivedFrom<BaseShape>())
        return NULL;

    BaseShape *clonedObject = (BaseShape *) CloneObject(*object);
    clonedObject->GetLayer()->AddShapeInternal(clonedObject);

    return clonedObject;
}