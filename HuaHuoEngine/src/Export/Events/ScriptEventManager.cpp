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
    g_ScriptEventManager = NEW(ScriptEventManager);
}

void ScriptEventManager::CleanupClass(void *) {
    Assert(g_ScriptEventManager != NULL);
    DELETE(g_ScriptEventManager);
    g_ScriptEventManager = NULL;
}

ScriptEventManager::ScriptEventManager(){

}
ScriptEventManager::~ScriptEventManager(){
    for(auto& item : m_ScriptEventHandlerLists){
        DELETE(item.second);
    }
}

ScriptEventManager* GetScriptEventManager(){
    Assert(g_ScriptEventManager != NULL);
    return g_ScriptEventManager;
}

void ScriptEventManager::RegisterEventHandler(std::string eventType, ScriptEventHandler* pHandler){
    if(!m_ScriptEventHandlerLists.contains(eventType)){
        m_ScriptEventHandlerLists[eventType] = NEW(ScriptEventHandlerList);
    }

    m_ScriptEventHandlerLists[eventType]->push_back(pHandler);
}

void ScriptEventManager::TriggerEvent(std::string eventType, ScriptEventHandlerArgs* args){
    if(m_ScriptEventHandlerLists.contains(eventType)){
        ScriptEventHandlerList* handlerList = m_ScriptEventHandlerLists[eventType];
        for(auto item : *handlerList){
            (*item).handleEvent(args);
        }
    }
}