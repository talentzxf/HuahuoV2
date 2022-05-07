#pragma once

#include "Modules/ExportModules.h"
#include "Logging/LogAssert.h"
#include "Utilities/DeconstructFunctionType.h"

template<typename FunctionPointerType, typename FunctionPointerTypeWithUserData>
struct EXPORT_COREMODULE CallbackInfoBase
{
    union FunctionPointers
    {
        FunctionPointerType m_FuncPtr;
        FunctionPointerTypeWithUserData m_FuncPtrWithUserData;
    };

    FunctionPointers    m_Funcs;
    const void*         m_UserData;
    bool                m_WithUserData;

    void Clear()
    {
        m_Funcs.m_FuncPtr = NULL;
        m_UserData = NULL;
        m_WithUserData = false;
    }

    bool Matches(const FunctionPointers& funcs, const void* userData) const
    {
        return m_Funcs.m_FuncPtr == funcs.m_FuncPtr && m_UserData == userData;
    }

    void Register(FunctionPointerType funcPtr, FunctionPointerTypeWithUserData funcPtrWithUserData, const void* userData)
    {
        m_UserData = userData;
        if (funcPtr)
        {
            m_WithUserData = false;
            m_Funcs.m_FuncPtr = funcPtr;
        }
        else
        {
            m_WithUserData = true;
            m_Funcs.m_FuncPtrWithUserData = funcPtrWithUserData;
        }
    }

    void Register(FunctionPointerType callback)
    {
        Register(callback, NULL, NULL);
    }

    void Register(FunctionPointerTypeWithUserData callback, const void* userData)
    {
        Register(NULL, callback, userData);
    }

    void Unregister()
    {
        Clear();
    }

    bool IsNull()
    {
        return (m_Funcs.m_FuncPtr == NULL);
    }

    void Invoke()
    {
        if (!m_WithUserData)
            (*m_Funcs.m_FuncPtr)();
        else
            (*m_Funcs.m_FuncPtrWithUserData)(m_UserData);
    }
};

template<typename FunctionPointerType>
class EXPORT_COREMODULE CallbackInfoSubBase : public CallbackInfoBase<FunctionPointerType, typename DeconstructFunctionType<FunctionPointerType>::template PrependArgument<const void*>::ResultType>
{
public:
    template<typename T>
    struct FuncTypes
    {
        typedef typename DeconstructFunctionType<FunctionPointerType>::template PrependArgument<T*>::ResultType StaticFuncType;
    };
    typedef typename DeconstructFunctionType<FunctionPointerType>::template PrependArgument<const void*>::ResultType FunctionPointerTypeWithUserData;

    void Register(FunctionPointerType callback)
    {
        CallbackInfoBase<FunctionPointerType, FunctionPointerTypeWithUserData>::Register(callback);
    }

    template<typename T>
    void Register(typename FuncTypes<T>::StaticFuncType callback, T* userData)
    {
        CallbackInfoBase<FunctionPointerType, FunctionPointerTypeWithUserData>::Register(NULL, (FunctionPointerTypeWithUserData)callback, userData);
    }
};

template<typename FunctionPointerType, typename FunctionPointerTypeWithUserData>
class EXPORT_COREMODULE CallbackArrayBase
{
public:
    typedef CallbackInfoBase<FunctionPointerType, FunctionPointerTypeWithUserData> CallbackInfo;
    typedef typename CallbackInfo::FunctionPointers FunctionPointers;

    CallbackArrayBase()
    {
        Clear();
    }

    void Clear()
    {
        for (UInt32 i = 0; i < kMaxCallback; i++)
            m_Callbacks[i].Clear();
        m_NumRegistered = 0;
        m_CurrentCallbackArrayHasChanged = false;
    }

    void Register(FunctionPointerType callback)
    {
        Register(callback, NULL, NULL);
    }

    void Unregister(FunctionPointerType callback)
    {
        FunctionPointers funcs;
        funcs.m_FuncPtr = callback;
        Unregister(funcs, NULL);
    }

    bool IsRegistered(FunctionPointerType callback) const
    {
        FunctionPointers funcs;
        funcs.m_FuncPtr = callback;
        return IsRegistered(funcs, NULL);
    }

    void Register(FunctionPointerTypeWithUserData callback, const void* userData)
    {
        Register(NULL, callback, userData);
    }

    void Unregister(FunctionPointerTypeWithUserData callback, const void* userData)
    {
        FunctionPointers funcs;
        funcs.m_FuncPtrWithUserData = callback;
        Unregister(funcs, userData);
    }

    bool IsRegistered(FunctionPointerTypeWithUserData callback, const void* userData) const
    {
        FunctionPointers funcs;
        funcs.m_FuncPtrWithUserData = callback;
        return IsRegistered(funcs, userData);
    }

    bool AnyRegistered() const
    {
        return (m_NumRegistered > 0);
    }

    UInt32 GetNumRegistered() const
    {
        return m_NumRegistered;
    }

    void CleanupAfterInvoke()
    {
        if (m_CurrentCallbackArrayHasChanged)
        {
            for (UInt32 i = 0; i < m_NumRegistered; i++)
            {
                CallbackInfo& callbackInfo = m_Callbacks[i];
                if (callbackInfo.m_Funcs.m_FuncPtr == NULL)
                {
                    MoveFoward(i);
                    --i;
                    m_NumRegistered--;
                }
            }
            m_CurrentCallbackArrayHasChanged = false;
        }
    }

protected:

    void Register(FunctionPointerType funcPtr, FunctionPointerTypeWithUserData funcPtrWithUserData, const void* userData)
    {
        if (m_NumRegistered >= kMaxCallback)
        {
            ErrorString("Callback registration failed. Increase kMaxCallback.");
        }
        CallbackInfo& callbackInfo = m_Callbacks[m_NumRegistered++];
        callbackInfo.Register(funcPtr, funcPtrWithUserData, userData);
    }

    void Unregister(FunctionPointers& funcs, const void* userData)
    {
        for (UInt32 i = 0; i < m_NumRegistered; i++)
        {
            CallbackInfo& callbackInfo = m_Callbacks[i];
            if (callbackInfo.Matches(funcs, userData))
            {
                callbackInfo.Clear();
                if (m_CurrentCallbackArray != this)
                {
                    m_NumRegistered--;
                    MoveFoward(i);
                }
                else
                {
                    m_CurrentCallbackArrayHasChanged = true;
                }
                return;
            }
        }

        AssertString("Callback unregistration failed. Callback not registered.");
    }

    bool IsRegistered(FunctionPointers& funcs, const void* userData) const
    {
        for (UInt32 i = 0; i < m_NumRegistered; i++)
        {
            if (m_Callbacks[i].Matches(funcs, userData))
                return true;
        }
        return false;
    }

    void MoveFoward(UInt32 start)
    {
        for (UInt32 i = start; i < m_NumRegistered; i++)
        {
            CallbackInfo& destInfo = m_Callbacks[i];
            CallbackInfo& srcInfo = m_Callbacks[i + 1];
            destInfo.m_Funcs = srcInfo.m_Funcs;
            destInfo.m_UserData = srcInfo.m_UserData;
            destInfo.m_WithUserData = srcInfo.m_WithUserData;
        }
    }

    enum { kMaxCallback = 128 }; // increase as needed
    CallbackInfo m_Callbacks[kMaxCallback];
    UInt32 m_NumRegistered;
    CallbackArrayBase* m_CurrentCallbackArray;
    bool m_CurrentCallbackArrayHasChanged;
};

template<typename FunctionPointerType>
class EXPORT_COREMODULE CallbackArraySubBase : public CallbackArrayBase<FunctionPointerType, typename DeconstructFunctionType<FunctionPointerType>::template PrependArgument<const void*>::ResultType>
{
public:
    template<typename T>
    struct FuncTypes
    {
        typedef typename DeconstructFunctionType<FunctionPointerType>::template PrependArgument<T*>::ResultType StaticFuncType;
    };
    typedef typename DeconstructFunctionType<FunctionPointerType>::template PrependArgument<const void*>::ResultType FunctionPointerTypeWithUserData;

    void Register(FunctionPointerType callback)
    {
        CallbackArrayBase<FunctionPointerType, FunctionPointerTypeWithUserData>::Register(callback);
    }

    void Unregister(FunctionPointerType callback)
    {
        CallbackArrayBase<FunctionPointerType, FunctionPointerTypeWithUserData>::Unregister(callback);
    }

    bool IsRegistered(FunctionPointerType callback) const
    {
        return CallbackArrayBase<FunctionPointerType, FunctionPointerTypeWithUserData>::IsRegistered(callback);
    }

    template<typename T>
    void Register(typename FuncTypes<T>::StaticFuncType callback, T* userData)
    {
        CallbackArrayBase<FunctionPointerType, FunctionPointerTypeWithUserData>::Register((FunctionPointerTypeWithUserData)callback, userData);
    }

    template<typename T>
    void Unregister(typename FuncTypes<T>::StaticFuncType callback, T* userData)
    {
        CallbackArrayBase<FunctionPointerType, FunctionPointerTypeWithUserData>::Unregister((FunctionPointerTypeWithUserData)callback, userData);
    }

    template<typename T>
    bool IsRegistered(typename FuncTypes<T>::StaticFuncType callback, T* userData) const
    {
        return CallbackArrayBase<FunctionPointerType, FunctionPointerTypeWithUserData>::IsRegistered((FunctionPointerTypeWithUserData)callback, userData);
    }
};

class EXPORT_COREMODULE CallbackArrayReturnsAnyTrue : public CallbackArraySubBase<bool(*)()>
{
public:
    bool Invoke()
    {
        bool result = false;
        this->m_CurrentCallbackArray = this;
        for (UInt32 i = 0; i < this->m_NumRegistered; i++)
        {
            CallbackInfo& callbackInfo = this->m_Callbacks[i];
            if (callbackInfo.m_Funcs.m_FuncPtr == NULL)
                continue;
            if (!callbackInfo.m_WithUserData)
                result = (*callbackInfo.m_Funcs.m_FuncPtr)();
            else
                result = (*callbackInfo.m_Funcs.m_FuncPtrWithUserData)(callbackInfo.m_UserData);

            if (result)
                break;
        }
        this->CleanupAfterInvoke();
        this->m_CurrentCallbackArray = NULL;
        return result;
    }
};

class EXPORT_COREMODULE CallbackArray : public CallbackArraySubBase<void(*)()>
{
public:
    void Invoke()
    {
        this->m_CurrentCallbackArray = this;
        for (UInt32 i = 0; i < this->m_NumRegistered; i++)
        {
            CallbackInfo& callbackInfo = this->m_Callbacks[i];
            if (callbackInfo.m_Funcs.m_FuncPtr)
                callbackInfo.Invoke();
        }
        this->CleanupAfterInvoke();
        this->m_CurrentCallbackArray = NULL;
    }
};

#define CALLBACK_INFO_INVOKE(...) \
    if (!this->m_WithUserData) \
        (*this->m_Funcs.m_FuncPtr)(__VA_ARGS__); \
    else \
        (*this->m_Funcs.m_FuncPtrWithUserData)(this->m_UserData, __VA_ARGS__); \

#define CALLBACK_INVOKE_LOOP(...) \
    this->m_CurrentCallbackArray = this; \
    for(UInt32 i = 0; i < this->m_NumRegistered; i++) \
    { \
        if(this->m_Callbacks[i].m_Funcs.m_FuncPtr == NULL) \
            continue; \
        if (!this->m_Callbacks[i].m_WithUserData) \
            (*this->m_Callbacks[i].m_Funcs.m_FuncPtr)(__VA_ARGS__); \
        else \
            (*this->m_Callbacks[i].m_Funcs.m_FuncPtrWithUserData)(this->m_Callbacks[i].m_UserData, __VA_ARGS__); \
    } \
    this->CleanupAfterInvoke(); \
    this->m_CurrentCallbackArray = NULL;


template<class P1>
class EXPORT_COREMODULE CallbackArray1 : public CallbackArraySubBase<void(*)(P1)>
{
public:
    void Invoke(P1 t1)
    {
        CALLBACK_INVOKE_LOOP(t1);
    }
};

template<class P1>
class EXPORT_COREMODULE CallbackInfo1 : public CallbackInfoSubBase<void(*)(P1)>
{
public:
    void Invoke(P1 t1)
    {
        CALLBACK_INFO_INVOKE(t1);
    }
};

template<class P1, class P2>
class EXPORT_COREMODULE CallbackArray2 : public CallbackArraySubBase<void(*)(P1, P2)>
{
public:
    void Invoke(P1 t1, P2 t2)
    {
        CALLBACK_INVOKE_LOOP(t1, t2);
    }
};

template<class P1, class P2>
class EXPORT_COREMODULE CallbackInfo2 : public CallbackInfoSubBase<void(*)(P1, P2)>
{
public:
    void Invoke(P1 t1, P2 t2)
    {
        CALLBACK_INFO_INVOKE(t1, t2);
    }
};

template<class P1, class P2, class P3>
class EXPORT_COREMODULE CallbackArray3 : public CallbackArraySubBase<void(*)(P1, P2, P3)>
{
public:
    void Invoke(P1 t1, P2 t2, P3 t3)
    {
        CALLBACK_INVOKE_LOOP(t1, t2, t3);
    }
};

template<class P1, class P2, class P3>
class EXPORT_COREMODULE CallbackInfo3 : public CallbackInfoSubBase<void(*)(P1, P2, P3)>
{
public:
    void Invoke(P1 t1, P2 t2, P3 t3)
    {
        CALLBACK_INFO_INVOKE(t1, t2, t3);
    }
};

template<class P1, class P2, class P3, class P4>
class EXPORT_COREMODULE CallbackArray4 : public CallbackArraySubBase<void(*)(P1, P2, P3, P4)>
{
public:
    void Invoke(P1 t1, P2 t2, P3 t3, P4 t4)
    {
        CALLBACK_INVOKE_LOOP(t1, t2, t3, t4);
    }
};

template<class P1, class P2, class P3, class P4>
class EXPORT_COREMODULE CallbackInfo4 : public CallbackInfoSubBase<void(*)(P1, P2, P3, P4)>
{
public:
    void Invoke(P1 t1, P2 t2, P3 t3, P4 t4)
    {
        CALLBACK_INFO_INVOKE(t1, t2, t3, t4);
    }
};

template<class P1, class P2, class P3, class P4, class P5>
class EXPORT_COREMODULE CallbackArray5 : public CallbackArraySubBase<void(*)(P1, P2, P3, P4, P5)>
{
public:
    void Invoke(P1 t1, P2 t2, P3 t3, P4 t4, P5 t5)
    {
        CALLBACK_INVOKE_LOOP(t1, t2, t3, t4, t5);
    }
};

template<class P1, class P2, class P3, class P4, class P5>
class EXPORT_COREMODULE CallbackInfo5 : public CallbackInfoSubBase<void(*)(P1, P2, P3, P4, P5)>
{
public:
    void Invoke(P1 t1, P2 t2, P3 t3, P4 t4, P5 t5)
    {
        CALLBACK_INFO_INVOKE(t1, t2, t3, t4, t5);
    }
};
