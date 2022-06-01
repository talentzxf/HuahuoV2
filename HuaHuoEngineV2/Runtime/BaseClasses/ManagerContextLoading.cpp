//
// Created by VincentZhang on 5/17/2022.
//

#include "ManagerContext.h"
#include "GameManager.h"

void CreateMissingGlobalGameManagers()
{
    const ManagerContext& context = GetManagerContext();

    for (int i = 0; i < ManagerContext::kGlobalManagerCount; i++)
    {
        Assert(context.m_ManagerTypes[i] != NULL);

        if (context.m_Managers[i] == NULL)
            SetManagerPtrInContext(i, CreateGameManager(context.m_ManagerTypes[i]));
    }
}