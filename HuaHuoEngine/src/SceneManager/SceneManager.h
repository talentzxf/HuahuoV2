//
// Created by VincentZhang on 5/7/2022.
//

#ifndef HUAHUOENGINE_SCENEMANAGER_H
#define HUAHUOENGINE_SCENEMANAGER_H
#include "Modules/ExportModules.h"
#include "HuaHuoScene.h"
#include <vector>

class RuntimeSceneManager ;
typedef RuntimeSceneManager SceneManager;

class EXPORT_COREMODULE RuntimeSceneManager {
public:
    explicit RuntimeSceneManager(/*MemLabelRef label*/);
    virtual ~RuntimeSceneManager();

    HuaHuoScene* CreateScene();
    HuaHuoScene* GetActiveScene();
    bool SetActiveScene(HuaHuoScene* scene);

protected:
    typedef std::vector<HuaHuoScene*> ScenePtrArray;

    ScenePtrArray       m_Scenes;
    ScenePtrArray       m_ScenesWaitingForLoading;

    HuaHuoScene*         m_ActiveScene;
};

RuntimeSceneManager& GetSceneManager();
RuntimeSceneManager& GetRuntimeSceneManager();
RuntimeSceneManager* GetSceneManagerPtr();


#endif //HUAHUOENGINE_SCENEMANAGER_H
