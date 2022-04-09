//
// Created by VincentZhang on 4/8/2022.
//

#ifndef PERSISTENTMANAGER_TYPE_H
#define PERSISTENTMANAGER_TYPE_H

#include "RTTI.h"
#include "BaseClasses/BaseTypes.h"
class Object;

// ----------------------------------------------------------------------------
// PUBLIC API

namespace HuaHuo { class Type; }

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
