//
// Created by VincentZhang on 5/8/2022.
//

#include "GameObjectHierarchyProperty.h"
#include "SceneManager/HuaHuoScene.h"

SceneRootTransformArray::SceneRootTransformArray() {

}

SceneRootTransformArray::~SceneRootTransformArray() {

}

void SceneRootTransformArray::SetCurrentScene(HuaHuoScene *pScene) {
    m_pScene = pScene;

    m_currentItr = m_pScene->RootBegin();
}

bool SceneRootTransformArray::MoveNext()
{
    if(m_currentItr != m_pScene->RootEnd()){
        m_currentItr++;
        return true;
    }
    return false;
}

Transform* SceneRootTransformArray::GetCurrentTransform() {
    return m_currentItr->GetData();
}