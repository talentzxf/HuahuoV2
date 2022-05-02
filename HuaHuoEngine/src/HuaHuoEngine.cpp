//
// Created by VincentZhang on 4/1/2022.
//

#include "HuaHuoEngine.h"
#include "BaseClasses/ClassRegistration.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "TypeSystem/TypeManager.h"
#include "Serialize/PathNamePersistentManager.h"

void HuaHuoEngine::InitEngine() {
    RegisterRuntimeInitializeAndCleanup::ExecuteInitializations();
    RegisterAllClasses();

    TypeManager::Get().InitializeAllTypes();

    TypeManager::Get().CallInitializeTypes();

    InitPathNamePersistentManager();
}

HuaHuoEngine *HuaHuoEngine::gInstance = new HuaHuoEngine();

HuaHuoEngine::HuaHuoEngine() {
}