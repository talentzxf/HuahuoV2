//
// Created by VincentZhang on 4/6/2022.
//

#ifndef HUAHUOENGINE_OBJECTDEFINES_H
#define HUAHUOENGINE_OBJECTDEFINES_H

#include "RTTI.h"
#include "Utilities/TypeUtilities.h"
#include "RTTI.h"
#include "Internal/CoreMacros.h"
#include "BaseClasses/InstanceID.h"
#include "Logging/LogAssert.h"
#include "Memory/MemoryMacros.h"
#include "Type.h"
#include "Serialize/TransferFunctions/StreamedBinaryRead.h"
#include "Serialize/TransferFunctions/StreamedBinaryWrite.h"

template<UInt32 typeID>
inline void PerformRegisterClassCompileTimeChecks()
{
//    CompileTimeAssert(typeID > 0, "IMPLEMENT_REGISTER_CLASS: PersistentTypeID must be higher than 0.");
//    CompileTimeAssert(typeID < 0x80000000, "IMPLEMENT_REGISTER_CLASS: PersistentTypeID must be lower than 0x80000000.");
}

#if DEBUGMODE

void EXPORT_COREMODULE AddVerifyClassRegistration(PersistentTypeID persistentTypeID);
void EXPORT_COREMODULE CleanupVerifyClassRegistration();

template<int PersistentTypeID>
struct VerifyObjectIsRegisteredHelper
{
    static void VerifyClassRegistration(void*) { AddVerifyClassRegistration(PersistentTypeID); }
    static RegisterRuntimeInitializeAndCleanup s_Init;
};

#define IMPLEMENT_CLASS_VERIFY_OBJECT_IS_REGISTERED(PERSISTENT_TYPE_ID)  \
template<> RegisterRuntimeInitializeAndCleanup VerifyObjectIsRegisteredHelper<PERSISTENT_TYPE_ID>::s_Init(&VerifyObjectIsRegisteredHelper<PERSISTENT_TYPE_ID>::VerifyClassRegistration, NULL, 1); \
class MISSING_SEMICOLON_AFTER_IMPLEMENT_CLASS_VERIFY_OBJECT_IS_REGISTERED

#else

#define IMPLEMENT_CLASS_VERIFY_OBJECT_IS_REGISTERED(PERSISTENT_TYPE_ID) \
class MISSING_SEMICOLON_AFTER_IMPLEMENT_CLASS_VERIFY_OBJECT_IS_REGISTERED

#endif

#define NEW_OBJECT(CLASS_) reinterpret_cast<CLASS_*>(::Object::AllocateAndAssignInstanceID(BaseObjectInternal::NewObject<CLASS_>(kMemBaseObject, kCreateObjectDefault)))

// ----------------------------------------------------------------------------


// This macro creates a compile-time test with which you can retrieve
// a static method from a class while ignoring any instances in it's parent class.
// It will return NULL when the static method does not exist.
#define CREATE_GET_STATIC_METHOD_FROM_TYPE_HELPER(HELPER_NAME_, METHOD_DECLARATION_, METHOD_NAME_) \
template<class T> struct HELPER_NAME_ \
{ \
private: \
    template<ToPointerType<METHOD_DECLARATION_>::ResultType> struct TestInstance; \
    template<typename Q> static FalseType Test(...); \
    template<typename Q> static TrueType  Test(TestInstance<&Q::METHOD_NAME_>*, ...); \
    template<typename Q> static FalseType Test(typename EnableIf<IsSameType<TestInstance<&Q::METHOD_NAME_>, TestInstance<&Q::Super::METHOD_NAME_> >::result>::type*, void*); \
    enum { value = sizeof(Test<T>(0, 0)) == sizeof(TrueType) }; \
    struct TypeNoCallback { enum MyEnum { METHOD_NAME_ = 0 }; }; \
public: \
    static ToPointerType<METHOD_DECLARATION_>::ResultType GetMethod() { return (ToPointerType<METHOD_DECLARATION_>::ResultType)Conditional<value, T, TypeNoCallback>::type::METHOD_NAME_; } \
}; \
class MISSING_SEMICOLON_AFTER_CREATE_UNIQUE_METHOD_IN_CLASS_TEST

CREATE_GET_STATIC_METHOD_FROM_TYPE_HELPER(GetInitializeClassMethodFromType, void(), InitializeClass);
CREATE_GET_STATIC_METHOD_FROM_TYPE_HELPER(GetPostInitializeClassMethodFromType, void(), PostInitializeClass);
CREATE_GET_STATIC_METHOD_FROM_TYPE_HELPER(GetCleanupClassMethodFromType, void(), CleanupClass);

// ----------------------------------------------------------------------------

namespace BaseObjectInternal
{
    template<typename type>
    inline Object* NewObject(MemLabelId label, ObjectCreationMode mode)
    {
        return HUAHUO_NEW_AS_ROOT(type, label, "Objects", NULL) (mode);
    }
}

template<typename T, bool isAbstract>
struct ProduceHelper
{
    static Object* Produce(MemLabelId label, ObjectCreationMode mode) { AssertFormatMsg(false, "Can't produce abstract class %s", TypeOf<T>()->GetName()); return NULL; }
};

template<typename T>
struct ProduceHelper<T, false>
{
    static Object* Produce(MemLabelId label, ObjectCreationMode mode) { return BaseObjectInternal::NewObject<T>(label, mode); }
};

enum TypeFlags
{
    kTypeNoFlags = 0,
    kTypeIsAbstract = 1 << 0,
    kTypeIsSealed = 1 << 1,
    kTypeIsEditorOnly = 1 << 2,
    kTypeIsStripped = 1 << 3
};


#define REGISTER_CLASS(TYPE_NAME_) \
public:                            \
    typedef ThisType Super;  \
    typedef TYPE_NAME_ ThisType;   \
    static const char* GetPPtrTypeString () { return "PPtr<"#TYPE_NAME_">"; } \
    static TYPE_NAME_* Produce(InstanceID instanceID = InstanceID_None, MemLabelId memLabel = kMemBaseObject, ObjectCreationMode mode = kCreateObjectDefault) \
        { return Object::Produce<TYPE_NAME_>(instanceID, memLabel, mode); } \
    static TYPE_NAME_* Produce(const HuaHuo::Type* type, InstanceID instanceID = InstanceID_None, MemLabelId memLabel = kMemBaseObject, ObjectCreationMode mode = kCreateObjectDefault) \
        { return Object::Produce<TYPE_NAME_>(type, instanceID, memLabel, mode); } \
private: \
    virtual const HuaHuo::Type* const GetTypeVirtualInternal() const override { return TypeOf<TYPE_NAME_>(); } \
protected:                         \
    ~TYPE_NAME_ (){ }              \
    void ThreadedCleanup(); \
public: \
    class MISSING_SEMICOLON_AFTER_REGISTER_CLASS_MACRO

typedef void TypeCallback();
struct TypeRegistrationDesc {
    RTTI init;
    RTTI* type;
    TypeCallback * initCallback;
    TypeCallback * postInitCallback;
    TypeCallback * cleanupCallback;
};

template<typename T>
void RegisterHuaHuoClass(const char* module);

// This is the default no-op registration of attributes for a type.
// It is called by RegisterHuaHuoClass<T>() and should be specialized if
// a specific type has any attributes. This is most easily done using
// the REGISTER_TYPE_ATTRIBUTES macro defined in Attribute.h.
template<typename T>
const ConstVariantRef* RegisterAttributes(size_t& attributeCountOut)
{
    attributeCountOut = 0;
    return NULL;
}

// ----------------------------------------------------------------------------

// Register (possibly multiple) traits for a class that is registered after this Macro using REGISTER_CLASS.
// Valid values are kTypeNoFlags, kTypeIsAbstract, kTypeIsSealed, kTypeIsEditorOnly
#define REGISTER_CLASS_TRAITS(...) public: struct kTypeFlags { enum { value = UNSIGNED_FLAGS(__VA_ARGS__) }; }; \
public: \
    class MISSING_SEMICOLON_AFTER_REGISTER_CLASS_TRAITS_MACRO

#define REGISTER_CLASS_WITH_MODULE_METADATA(typeName, moduleName)

#define IMPLEMENT_REGISTER_CLASS(...) PP_VARG_SELECT_OVERLOAD(IMPLEMENT_REGISTER_CLASS_, (__VA_ARGS__))

#define IMPLEMENT_REGISTER_CLASS_2(TYPE_NAME_, PERSISTENT_TYPE_ID) IMPLEMENT_REGISTER_CLASS_3(,TYPE_NAME_, PERSISTENT_TYPE_ID)

#define IMPLEMENT_REGISTER_CLASS_3(NAMESPACE_, TYPE_NAME_, PERSISTENT_TYPE_ID) \
/* If defined get the TypeFlags for our type, otherwise return kTypeNoFlags. Ignores inherited TypeFlags. */ \
enum { k##NAMESPACE_##TYPE_NAME_##TypeFlags = SelectOnTypeEquality<NAMESPACE_::TYPE_NAME_::ThisType::kTypeFlags, NAMESPACE_::TYPE_NAME_::Super::kTypeFlags, kTypeNoFlags, NAMESPACE_::TYPE_NAME_::ThisType::kTypeFlags::value>::result }; \
IMPLEMENT_CLASS_VERIFY_OBJECT_IS_REGISTERED(PERSISTENT_TYPE_ID); \
static TypeRegistrationDesc NAMESPACE_##TYPE_NAME_##_TypeRegistrationDesc = \
{ \
    { /* RTTI */ \
        &TypeContainer<NAMESPACE_::TYPE_NAME_::Super>::rtti, \
        NULL, \
        #TYPE_NAME_, \
        #NAMESPACE_, \
        "undefined", \
        PERSISTENT_TYPE_ID, \
        sizeof(NAMESPACE_::TYPE_NAME_), \
        { \
            static_cast<RuntimeTypeIndex>(RTTI::DefaultTypeIndex), \
            RTTI::DefaultDescendentCount, \
        }, \
        (k##NAMESPACE_##TYPE_NAME_##TypeFlags & kTypeIsAbstract) != 0, \
        (k##NAMESPACE_##TYPE_NAME_##TypeFlags & kTypeIsSealed) != 0, \
        (k##NAMESPACE_##TYPE_NAME_##TypeFlags & kTypeIsEditorOnly) != 0, \
        false, \
        NULL, \
        0 \
    }, \
    &TypeContainer<NAMESPACE_::TYPE_NAME_>::rtti, \
    NULL, \
    NULL, \
    NULL, \
}; \
template<> void RegisterHuaHuoClass<NAMESPACE_::TYPE_NAME_>(const char* module) \
{ \
    PerformRegisterClassCompileTimeChecks<PERSISTENT_TYPE_ID>(); \
    \
    /* These parts of the struct are initialized within this method to make sure the linker removes */ \
    /* these methods when RegisterHuaHuoClass is not called. This is essential for HuaHuo's stripping to work! */ \
    NAMESPACE_##TYPE_NAME_##_TypeRegistrationDesc.init.factory = ProduceHelper<NAMESPACE_::TYPE_NAME_, (k##NAMESPACE_##TYPE_NAME_##TypeFlags & kTypeIsAbstract) != 0 >::Produce; \
    NAMESPACE_##TYPE_NAME_##_TypeRegistrationDesc.init.module = module; \
    NAMESPACE_##TYPE_NAME_##_TypeRegistrationDesc.initCallback = GetInitializeClassMethodFromType<NAMESPACE_::TYPE_NAME_>::GetMethod(); \
    NAMESPACE_##TYPE_NAME_##_TypeRegistrationDesc.postInitCallback = GetPostInitializeClassMethodFromType<NAMESPACE_::TYPE_NAME_>::GetMethod(); \
    NAMESPACE_##TYPE_NAME_##_TypeRegistrationDesc.cleanupCallback = GetCleanupClassMethodFromType<NAMESPACE_::TYPE_NAME_>::GetMethod(); \
    \
    TypeRegistrationDesc& desc = NAMESPACE_##TYPE_NAME_##_TypeRegistrationDesc ; \
    desc.init.attributes = RegisterAttributes<NAMESPACE_::TYPE_NAME_>(desc.init.attributeCount); \
    void GlobalRegisterType(const TypeRegistrationDesc& desc); \
    GlobalRegisterType(desc); \
    REGISTER_CLASS_WITH_MODULE_METADATA(NAMESPACE_::TYPE_NAME_, module); \
} \
class MISSING_SEMICOLON_AFTER_IMPLEMENT_REGISTER_CLASS_MACRO

#define DECLARE_OBJECT_SERIALIZE(...) /* the "..." will be removed in the near future */ \
    public: \
        static const char* GetTypeString () { return TypeOf<ThisType>()->GetName(); } \
        static bool MightContainPPtr () { return true; } \
        static bool AllowTransferOptimization () { return false; } \
        template<class TransferFunction> void Transfer (TransferFunction& transfer); \
        /* virtual void VirtualRedirectTransfer (GenerateTypeTreeTransfer& transfer) override; */ \
        virtual void VirtualRedirectTransfer (StreamedBinaryRead& transfer) override; \
        virtual void VirtualRedirectTransfer (StreamedBinaryWrite& transfer) override; \
        /* virtual void VirtualRedirectTransfer (RemapPPtrTransfer& transfer) override; */\
    public: \
        class MISSING_SEMICOLON_AFTER_DECLARE_OBJECT_SERIALIZE; /* semicolon will be removed in the near future */

#define INSTANTIATE_TEMPLATE_TRANSFER_WITH_DECL(FULL_TYPENAME_, DECL_, FUNCTION_NAME_, FUNCTION_RETURN_TYPE_) \
    /* template DECL_ FUNCTION_RETURN_TYPE_ FULL_TYPENAME_::FUNCTION_NAME_(GenerateTypeTreeTransfer& transfer); */\
    template DECL_ FUNCTION_RETURN_TYPE_ FULL_TYPENAME_::FUNCTION_NAME_(StreamedBinaryRead& transfer); \
    template DECL_ FUNCTION_RETURN_TYPE_ FULL_TYPENAME_::FUNCTION_NAME_(StreamedBinaryWrite& transfer); \
    /* template DECL_ FUNCTION_RETURN_TYPE_ FULL_TYPENAME_::FUNCTION_NAME_(RemapPPtrTransfer& transfer); */ \
    class MISSING_SEMICOLON_AFTER_INSTANTIATE_TEMPLATE_TRANSFER; /* semicolon will be removed in the near future */

#define IMPLEMENT_OBJECT_SERIALIZE_WITH_DECL(PREFIX_, DECL_) \
    /* DECL_ void PREFIX_ VirtualRedirectTransfer (GenerateTypeTreeTransfer& transfer)     { transfer.TransferBase (*this); } */\
    DECL_ void PREFIX_ VirtualRedirectTransfer (StreamedBinaryRead& transfer)    { /*SET_ALLOC_OWNER(GetMemoryLabel());*/ transfer.TransferBase (*this); } \
    DECL_ void PREFIX_ VirtualRedirectTransfer (StreamedBinaryWrite& transfer)   { transfer.TransferBase (*this); } \
    /* DECL_ void PREFIX_ VirtualRedirectTransfer (RemapPPtrTransfer& transfer)            { transfer.TransferBase (*this); }*/\
    class MISSING_SEMICOLON_AFTER_IMPLEMENT_OBJECT_SERIALIZE; /* semicolon will be removed in the near future */

#define IMPLEMENT_OBJECT_SERIALIZE(TYPE_) IMPLEMENT_OBJECT_SERIALIZE_WITH_DECL(TYPE_::, )

#define INSTANTIATE_TEMPLATE_TRANSFER_EXPORTED(FULL_TYPENAME_) INSTANTIATE_TEMPLATE_TRANSFER_WITH_DECL(FULL_TYPENAME_, EXPORTDLL,Transfer,void)
#define INSTANTIATE_TEMPLATE_TRANSFER(FULL_TYPENAME_) INSTANTIATE_TEMPLATE_TRANSFER_WITH_DECL(FULL_TYPENAME_, ,Transfer,void)
#define INSTANTIATE_TEMPLATE_TRANSFER_FUNCTION(FULL_TYPENAME_, FUNCTION_NAME_) INSTANTIATE_TEMPLATE_TRANSFER_WITH_DECL(FULL_TYPENAME_, ,FUNCTION_NAME_,void)

#endif //HUAHUOENGINE_OBJECTDEFINES_H
