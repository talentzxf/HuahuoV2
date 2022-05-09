//
// Created by VincentZhang on 5/8/2022.
//

#ifndef HUAHUOENGINE_SCRIPTEVENTMANAGER_H
#define HUAHUOENGINE_SCRIPTEVENTMANAGER_H
#include "EventDefs.h"
#include <string>
#include <list>
#include <map>

class ScriptEventHandlerArgs{

};

class ScriptEventHandler{
    friend class ScriptEventManager;
protected:
    virtual void handleEvent(ScriptEventHandlerArgs* args) = 0;
};

class ScriptEventManager {
    typedef std::list<ScriptEventHandler*> ScriptEventHandlerList;
    typedef ScriptEventHandlerList* ScriptEventHandlerListPtr;
private:
    std::map<std::string, ScriptEventHandlerList*> m_ScriptEventHandlerLists;
public:
    ScriptEventManager();
    virtual ~ScriptEventManager();
public:
    static void InitClass(void*);
    static void CleanupClass(void *);

    void RegisterEventHandler(std::string eventType, ScriptEventHandler* pHandler);
    void TriggerEvent(std::string eventType, ScriptEventHandlerArgs* args);
};

ScriptEventManager* GetScriptEventManager();
#endif //HUAHUOENGINE_SCRIPTEVENTMANAGER_H
