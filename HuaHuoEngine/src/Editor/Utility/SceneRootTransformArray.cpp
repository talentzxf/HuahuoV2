//
// Created by VincentZhang on 5/8/2022.
//

#include "SceneRootTransformArray.h"
#include "SceneManager/HuaHuoScene.h"

SceneRootTransformArray::SceneRootTransformArray(HuaHuoScene *pScene) {
    SetCurrentScene(pScene);
}

SceneRootTransformArray::~SceneRootTransformArray() {

}

void SceneRootTransformArray::SetCurrentScene(HuaHuoScene *pScene) {
    m_pScene = pScene;

    m_currentItr = m_pScene->RootBegin();
}

bool SceneRootTransformArray::MoveNext()
{
    if(m_currentItr->GetNext() != m_pScene->RootEnd()){
        m_currentItr++;
        return true;
    }
    return false;
}

Transform* SceneRootTransformArray::GetCurrentTransform() {
    return m_currentItr->GetData();
}