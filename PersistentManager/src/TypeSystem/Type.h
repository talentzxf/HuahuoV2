//
// Created by VincentZhang on 4/8/2022.
//

#ifndef PERSISTENTMANAGER_TYPE_H
#define PERSISTENTMANAGER_TYPE_H

#include "RTTI.h"
#include "BaseClasses/BaseTypes.h"
#include <vector>
#include <string>
#include "Logging/LogAssert.h"
class Object;


// ----------------------------------------------------------------------------
// PUBLIC API

namespace HuaHuo { class Type; }


namespace HuaHuo
{
    template<typename TAttribute> class AllAttributes;
    class TypeAttributes;

    class Type
    {
    public:
        enum CaseSensitivityOptions
        {
            kCaseSensitive,
            kCaseInSensitive,
        };

        enum TypeFilterOptions
        {
            kAllTypes,
            kOnlyNonAbstract,
        };

        enum
        {
            UndefinedPersistentTypeID = RTTI::UndefinedPersistentTypeID
        };

        typedef Object* FactoryFunction (ObjectCreationMode mode);

        static UInt32 GetTypeCount() { return RTTI::GetRuntimeTypes().Count; }

        static const Type* GetTypeByRuntimeTypeIndex(RuntimeTypeIndex index)
        {
            DebugAssertMsg(index < GetTypeCount(), "Runtime type index is out of bounds of registered types");
            return reinterpret_cast<Type*>(RTTI::GetRuntimeTypes().Types[index]);
        }

        static const Type* FindTypeByName(const char* name, CaseSensitivityOptions options = kCaseSensitive);
        static const Type* FindTypeByPersistentTypeID(PersistentTypeID id);
        static const Type* GetDeserializationStubForPersistentTypeID(PersistentTypeID typeID);

        bool HasValidRuntimeTypeIndex() const { return m_internal.derivedFromInfo.typeIndex != RTTI::DefaultTypeIndex; }

        PersistentTypeID GetPersistentTypeID() const { return PersistentTypeID(m_internal.persistentTypeID); }

        RuntimeTypeIndex GetRuntimeTypeIndex() const { return m_internal.derivedFromInfo.typeIndex; }
        UInt32 GetDescendantCount() const { return m_internal.derivedFromInfo.descendantCount; }

        const char* GetName() const { return m_internal.className; }
        const char* GetNamespace() const { return m_internal.classNamespace; }
        const char* GetModule() const { return m_internal.module; }
        std::string GetFullName() const { return m_internal.GetFullName(); }

        const Type* GetBaseClass() const { return reinterpret_cast<const Type*>(m_internal.base); }
        void FindAllDerivedClasses(std::vector<const Type*>& result, TypeFilterOptions options) const;

        template<typename TAttribute>
        bool HasAttribute() const
        {
            return HasAttribute<TAttribute>(GetRuntimeTypeIndex());
        }

//        template<typename TAttribute>
//        static bool HasAttribute(RuntimeTypeIndex typeIndex)
//        {
//            return detail::AttributeMapContainer<TAttribute>::s_map.HasAttribute(typeIndex);
//        }

//        template<typename TAttribute>
//        const TAttribute* FindAttribute() const
//        {
//            CompileTimeAssert(!core::is_empty<TAttribute>::value, "Attribute is an empty attribute type. Use HasAttribute() instead. FindAttribute is for finding a non-empty attribute instance");
//            const Type* attributeType = TypeOf<TAttribute>();
//            const ConstVariantRef* arr = m_internal.attributes;
//            for (size_t i = 0; i < m_internal.attributeCount; ++i)
//            {
//                if (attributeType == arr[i].GetType())
//                    return &(arr[i].Get<TAttribute>());
//            }
//            return NULL;
//        }

        size_t GetAttributeCount() const { return m_internal.attributeCount; }
        const ConstVariantRef& GetAttribute(size_t idx) const { return m_internal.attributes[idx]; }

        void GetAttributes(TypeAttributes& result) const;

        template<typename TAttribute>
        static AllAttributes<TAttribute> GetAllAttributes()
        {
            return AllAttributes<TAttribute>();
        }

        int GetSize() const { return m_internal.size; }
        FactoryFunction* GetFactory() const { return m_internal.factory; }

        bool IsSealed() const { return m_internal.isSealed; }
        bool IsAbstract() const { return m_internal.isAbstract; }
        bool IsEditorOnly() const { return m_internal.isEditorOnly; }
        bool IsStripped() const { return m_internal.isStripped; }

        bool IsBaseOf(RuntimeTypeIndex derived) const { return RTTI::IsDerivedFrom(derived, m_internal); }
        bool IsBaseOf(const Type* derived) const
        {
            AssertMsg(derived != NULL, "IsBaseOf : Derived type cannot be null");
            return RTTI::IsDerivedFrom(derived->m_internal, m_internal);
        }

        template<typename T> bool IsBaseOf() const { return RTTI::IsDerivedFrom(TypeOf<T>()->m_internal, m_internal); }

        bool IsDerivedFrom(const Type* base) const
        {
            AssertMsg(base != NULL, "IsDerivedFrom : base type cannot be null");
            return RTTI::IsDerivedFrom(m_internal, base->m_internal);
        }

        template<typename T> bool IsDerivedFrom() const { return RTTI::IsDerivedFrom(m_internal, TypeOf<T>()->m_internal); }

    private:
        RTTI m_internal;
    };

#if ENABLE_UNIT_TESTS
    // Define operator << for const Type*, which will be automatically picked up and used by the unit testing framework to
    // e.g. make parametric tests display nice type names in their titles instead of just the addresses of type objects
    UnitTest::MemoryOutStream& operator<<(UnitTest::MemoryOutStream& stream, const Type* type);
#endif
}


// As is TypeContainer<T>::rtti will not be guaranteed to be unique with dylibs/dlls
// On OSX and Linux it will need a visibility("default") attribute and on windows
// the linking of these symbols needs to be handled manually by using a custom section and
// remapping on startup (using __declspec(allocate("...") and #pragma section)
template<typename T>
struct TypeContainer
{
    static RTTI rtti;
};

// RTTI is deliberately aggregate initialized (see the comment on the definition of RTTI)
#define RTTI_DEFAULT_INITIALIZER_LIST { NULL, NULL, "[UNREGISTERED]", "", "undefined", RTTI::UndefinedPersistentTypeID, -1, \
{ (UInt32)RTTI::DefaultTypeIndex, (UInt32)RTTI::DefaultDescendentCount}, false, false, false, false, NULL, 0 }

template<typename T>
RTTI TypeContainer<T>::rtti = RTTI_DEFAULT_INITIALIZER_LIST;

// The template argument of TypeOf must be a complete type
// Whenever possible this should be used over WeakTypeOf to avoid errors where you mistakenly forward define
// a type in the wrong namespace or with a misspelled name and end up referencing the wrong type
template<typename T>
const HuaHuo::Type* TypeOf()
{
    // CompileTimeAssert(sizeof(T) > 0, "TypeOf<> cannot be used with incomplete type");
    return reinterpret_cast<HuaHuo::Type*>(&TypeContainer<T>::rtti);
}

// TypeRegistrationDesc is deliberately aggregate initialized (necessary for IMPLEMENT_REGISTER_CLASS to work at file scope)
#define TYPEREGISTRATIONDESC_DEFAULT_INITIALIZER_LIST { RTTI_DEFAULT_INITIALIZER_LIST, NULL, NULL, NULL, NULL }
#endif //PERSISTENTMANAGER_TYPE_H
