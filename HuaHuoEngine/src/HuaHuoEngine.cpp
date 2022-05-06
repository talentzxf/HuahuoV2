//
// Created by VincentZhang on 4/1/2022.
//

#include "HuaHuoEngine.h"
#include "BaseClasses/ClassRegistration.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "TypeSystem/TypeManager.h"
#include "Serialize/PathNamePersistentManager.h"
#include "BaseClasses/MessageHandler.h"

void HuaHuoEngine::InitEngine() {
    RegisterRuntimeInitializeAndCleanup::ExecuteInitializations();
    RegisterAllClasses();

    TypeManager::Get().InitializeAllTypes();

    MessageHandler::Get().Initialize(TypeOf<Object>());
    TypeManager::Get().CallInitializeTypes();

    TypeManager::Get().CallPostInitializeTypes();
    MessageHandler::Get().ResolveCallbacks();


    InitPathNamePersistentManager();
}

HuaHuoEngine *HuaHuoEngine::gInstance = new HuaHuoEngine();

HuaHuoEngine::HuaHuoEngine() {
}