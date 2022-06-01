//
// Created by VincentZhang on 4/11/2022.
//

#include "RegisterRuntimeInitializeAndCleanup.h"
#include "RegisterRuntimeInitializeAndCleanup.h"
#include <cstdio>
#include <vector>

RegisterRuntimeInitializeAndCleanup* RegisterRuntimeInitializeAndCleanup::s_LastRegistered = NULL;

bool RegisterRuntimeInitializeAndCleanup::Sort(const RegisterRuntimeInitializeAndCleanup* lhs, const RegisterRuntimeInitializeAndCleanup* rhs)
{
    if (lhs->m_Order == rhs->m_Order)
        return lhs < rhs;
    return lhs->m_Order < rhs->m_Order;
}

RegisterRuntimeInitializeAndCleanup::RegisterRuntimeInitializeAndCleanup(CallbackFunction* Initialize, CallbackFunction* Cleanup, int order, void* data)
{
    Register(Initialize, Cleanup, order, data);
}

RegisterRuntimeInitializeAndCleanup::~RegisterRuntimeInitializeAndCleanup()
{
    Unregister();
}

void RegisterRuntimeInitializeAndCleanup::Register(RegisterRuntimeInitializeAndCleanup::CallbackFunction* Initialize, RegisterRuntimeInitializeAndCleanup::CallbackFunction* Cleanup, int order, void* data)
{
    m_Init = Initialize;
    m_Cleanup = Cleanup;
    m_UserData = data;
    m_Order = order;
    m_InitCalled = false;
    m_Next = s_LastRegistered;
    m_Prev = NULL;
    if (s_LastRegistered)
        s_LastRegistered->m_Prev = this;
    s_LastRegistered = this;
}

void RegisterRuntimeInitializeAndCleanup::Unregister()
{
    if (m_Prev)
        m_Prev->m_Next = m_Next;
    else
        s_LastRegistered = m_Next;

    if (m_Next)
        m_Next->m_Prev = m_Prev;
}

/**
 This can be called multiple times but each init method is only called once by marking it with initCalled.
 The use case for this is when we have modules loaded after this has been called once in the case of modular runtime.
*/
void RegisterRuntimeInitializeAndCleanup::ExecuteInitializations()
{
    std::vector<RegisterRuntimeInitializeAndCleanup*> callbacks;
    RegisterRuntimeInitializeAndCleanup* current = s_LastRegistered;
    while (current)
    {
        callbacks.push_back(current);
        current = current->m_Next;
    }

    std::sort(callbacks.begin(), callbacks.end(), RegisterRuntimeInitializeAndCleanup::Sort);
    int count = callbacks.size();
    for (int i = 0; i < count; i++)
    {
        if (callbacks[i]->m_Init && !callbacks[i]->m_InitCalled)
            callbacks[i]->m_Init(callbacks[i]->m_UserData);

        callbacks[i]->m_InitCalled = true;
    }
}

void RegisterRuntimeInitializeAndCleanup::ExecuteCleanup()
{
    // Note: Don't use kMemTempAlloc because TLS is deleted in the loop below
    std::vector<RegisterRuntimeInitializeAndCleanup*> callbacks;
    RegisterRuntimeInitializeAndCleanup* current = s_LastRegistered;
    while (current)
    {
        callbacks.push_back(current);
        current = current->m_Next;
    }

    std::sort(callbacks.begin(), callbacks.end(), RegisterRuntimeInitializeAndCleanup::Sort);

    int count = callbacks.size();
    for (int i = count - 1; i >= 0; i--)
    {
        if (callbacks[i]->m_Cleanup && callbacks[i]->m_InitCalled)
            callbacks[i]->m_Cleanup(callbacks[i]->m_UserData);

        callbacks[i]->m_InitCalled = false;
    }
}
