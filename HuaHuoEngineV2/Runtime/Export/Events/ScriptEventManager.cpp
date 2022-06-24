//
// Created by VincentZhang on 5/8/2022.
//

#include "ScriptEventManager.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "Memory/MemoryMacros.h"
#include "Logging/LogAssert.h"
#include <cstdlib>

ScriptEventManager* g_ScriptEventManager = NULL;
RegisterRuntimeInitializeAndCleanup s_ScriptEventManagerInitAndCleanup(ScriptEventManager::InitClass, ScriptEventManager::CleanupClass, -1);

void ScriptEventManager::InitClass(void *) {
    Assert(g_ScriptEventManager == NULL);
    g_ScriptEventManager = HUAHUO_NEW_AS_ROOT(ScriptEventManager, kMemScriptEventManager, "Managers", "RuntimeSceneManager")();
}

void ScriptEventManager::CleanupClass(void *) {
    Assert(g_ScriptEventManager != NULL);
    HUAHUO_DELETE(g_ScriptEventManager, kMemScriptEventManager);
    g_ScriptEventManager = NULL;
}

ScriptEventManager::ScriptEventManager(MemLabelId memLabelId){

}
ScriptEventManager::~ScriptEventManager(){
    for(auto& item : m_ScriptEventHandlerLists){
        HUAHUO_DELETE(item.second, kMemScriptEventManager);
    }
}

ScriptEventManager* GetScriptEventManager(){
    Assert(g_ScriptEventManager != NULL);
    return g_ScriptEventManager;
}

void ScriptEventManager::RegisterEventHandler(std::string eventType, ScriptEventHandler* pHandler){
    if(!m_ScriptEventHandlerLists.contains(eventType)){
        m_ScriptEventHandlerLists[eventType] = HUAHUO_NEW(ScriptEventHandlerList, kMemScriptEventManager);
    }

    m_ScriptEventHandlerLists[eventType]->push_back(pHandler);
}

void ScriptEventManager::TriggerEvent(std::string eventType, ScriptEventHandlerArgs* args){
    printf("Triggering event:%s\n", eventType.c_str());

    if(m_ScriptEventHandlerLists.contains(eventType)){
        ScriptEventHandlerList* handlerList = m_ScriptEventHandlerLists[eventType];
        for(auto item : *handlerList){
            (*item).handleEvent(args);
        }
    }
}

bool ScriptEventManager::IsEventRegistered(std::string eventType){
    if(m_ScriptEventHandlerLists.contains(eventType)){
        return true;
    }
    return false;
}