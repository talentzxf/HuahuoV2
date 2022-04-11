//
// Created by VincentZhang on 4/11/2022.
//

#ifndef PERSISTENTMANAGER_REGISTERRUNTIMEINITIALIZEANDCLEANUP_H
#define PERSISTENTMANAGER_REGISTERRUNTIMEINITIALIZEANDCLEANUP_H

class RegisterRuntimeInitializeAndCleanup
{
public:
    typedef void CallbackFunction (void* data);
    RegisterRuntimeInitializeAndCleanup(CallbackFunction* Initialize, CallbackFunction* Cleanup, int order = 0, void* data = 0);
    ~RegisterRuntimeInitializeAndCleanup();

    static void ExecuteInitializations();
    static void ExecuteCleanup();

private:

    void Register(CallbackFunction* Initialize, CallbackFunction* Cleanup, int order = 0, void* data = 0);
    void Unregister();
    static bool Sort(const RegisterRuntimeInitializeAndCleanup* lhs, const RegisterRuntimeInitializeAndCleanup* rhs);

    int                                                     m_Order;
    void*                                                   m_UserData;
    RegisterRuntimeInitializeAndCleanup::CallbackFunction*  m_Init;
    RegisterRuntimeInitializeAndCleanup::CallbackFunction*  m_Cleanup;
    bool                                                    m_InitCalled;
    RegisterRuntimeInitializeAndCleanup*                    m_Next;
    RegisterRuntimeInitializeAndCleanup*                    m_Prev;

    static RegisterRuntimeInitializeAndCleanup*             s_LastRegistered;
};

struct RuntimeInitializeAndCleanupHandler
{
    RuntimeInitializeAndCleanupHandler() { RegisterRuntimeInitializeAndCleanup::ExecuteInitializations(); }
    ~RuntimeInitializeAndCleanupHandler() { RegisterRuntimeInitializeAndCleanup::ExecuteCleanup(); }
};


#endif //PERSISTENTMANAGER_REGISTERRUNTIMEINITIALIZEANDCLEANUP_H
