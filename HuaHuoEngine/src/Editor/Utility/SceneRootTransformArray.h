//
// Created by VincentZhang on 5/8/2022.
//

#ifndef HUAHUOENGINE_GAMEOBJECTHIERARCHYPROPERTY_H
#define HUAHUOENGINE_GAMEOBJECTHIERARCHYPROPERTY_H
#include "SceneManager/HuaHuoScene.h"
#include <stack>

class SceneRootTransformArray {
private:
    HuaHuoScene* m_pScene;
    RootTransformList::iterator m_currentItr;
public:
    SceneRootTransformArray();
    virtual ~SceneRootTransformArray();

    void SetCurrentScene(HuaHuoScene* pScene);
    bool MoveNext();
    Transform* GetCurrentTransform();
};


#endif //HUAHUOENGINE_GAMEOBJECTHIERARCHYPROPERTY_H
