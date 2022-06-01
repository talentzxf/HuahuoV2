#pragma  once
#include <stdint.h>
#include "TypeSystem/Type.h"

struct MessageData
{
    const HuaHuo::Type* type;
private:
    // Note: on Metro WinRT types cannot be located in union, so don't use union!
    intptr_t data;
//    ScriptingObjectPtr scriptingObjectData;
public:
    MessageData() : type(NULL), data(0)//, scriptingObjectData(SCRIPTING_NULL)
    {
    }

    template<typename T>
    MessageData(T inData)
    :data (0)
//        : type(TypeOf<typename dense_hash_map_traits::remove_pointer<T>::type>())
//        , data(0)
//        , scriptingObjectData(SCRIPTING_NULL)
    {
        AssertFormatMsg(sizeof(T) <= sizeof(data), "MessageData payload exceeded %zu > %zu", sizeof(T), sizeof(data));
        *reinterpret_cast<T*>(&data) = inData;
    }

    template<class T>
    T GetData() const
    {
//        // Check if GetData is used instead of GetScriptingObjectData
//        AssertMsg(type != GetScriptingObjectDataType(), "Cannot get ScriptingObjectPtr from MessageData with type == ScriptingObjectPtr. Explicitly use GetScriptingObjectData() instead.");
        return *reinterpret_cast<const T*>(&data);
    }
//
//    static const Unity::Type* GetScriptingObjectDataType() { return TypeOf<ScriptingObjectPtr>(); }
//
//    // Note: Required for Metro WinRT compatibility
//    ScriptingObjectPtr GetScriptingObjectData() const
//    {
//        AssertMsg(type == GetScriptingObjectDataType(), "Cannot get ScriptingObjectPtr from MessageData with type != ScriptingObjectPtr.");
//        return scriptingObjectData;
//    }
//
//    bool HasValidType() const
//    {
//        return type != NULL;
//    }
//
//    intptr_t& GetGenericDataRef()
//    {
//        // Check if GetGenericDataRef is used instead of GetScriptingObjectData
//        AssertMsg(type != GetScriptingObjectDataType(), "Cannot get reference to payload of type ScriptingObjectPtr from MessageData using GetGenericDataRef(). Use GetScriptingObjectData() instead.");
//        return data;
//    }
//
//    void SetScriptingObjectData(ScriptingObjectPtr inData)
//    {
//        scriptingObjectData = inData;
//        type = GetScriptingObjectDataType();
//    }
//
//    ScriptingObjectPtr GetScriptingObjectData()
//    {
//        AssertMsg(type == GetScriptingObjectDataType(), "Cannot get ScriptingObjectPtr from MessageData with type != MonoObject. Use GetData() or GeteGenericDataRef() instead.");
//        return scriptingObjectData;
//    }
};
