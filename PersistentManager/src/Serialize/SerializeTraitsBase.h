//
// Created by VincentZhang on 4/22/2022.
//

#ifndef PERSISTENTMANAGER_SERIALIZETRAITSBASE_H
#define PERSISTENTMANAGER_SERIALIZETRAITSBASE_H
#include <cstdlib>
#include "SerializationMetaFlags.h"

template<class T>
class SerializeTraitsBase
{
public:

    typedef T value_type;

    static int GetByteSize()   {return sizeof(value_type); }
    static size_t GetAlignOf()  {return alignof(value_type); }

    // By default safe binary read may convert any type to any other type.
    // On the other hand Builtin types (float, int etc) need to be explicitly converted, see SerializeTraitsBaseForBasicType.
    static bool AllowTypeConversion() { return true; }

    static void EnsureValid(T & data, TransferMetaFlags metaFlag) {}

    template<class TransferFunction>
    inline static bool IsTransferringEmptyManagedReferenceClass(T& data, TransferFunction& transfer) { return false; }
};

template<class T>
class SerializeTraitsBaseForBasicType : public SerializeTraitsBase<T>
{
public:
    typedef T value_type;

    // Builtin types (float, int etc) need to be explicitly converted via converters. eg. int -> float is not binary compatible and goes through a specific callback function that converts the data.
    static bool AllowTypeConversion() { return false; }

    template<class TransferFunction> inline
    static void Transfer(value_type& data, TransferFunction& transfer)
    {
        transfer.TransferBasicData(data);
    }
};

template<class T>
class SerializeTraits : public SerializeTraitsBase<T>
{
public:

    typedef T value_type;

    inline static const char* GetTypeString(void* /*ptr*/) { return value_type::GetTypeString(); }
    inline static bool MightContainPPtr() { return value_type::MightContainPPtr(); }

    /// AllowTransferOptimization can be used for type that have the same memory format as serialized format.
    /// Eg. a float or a Vector3f.
    /// StreamedBinaryRead will collapse the read into a direct read when reading an array with values that have AllowTransferOptimization.
    static bool AllowTransferOptimization() { return T::AllowTransferOptimization(); }

    template<class TransferFunction> inline
    static void Transfer(value_type& data, TransferFunction& transfer)
    {
        data.Transfer(transfer);
    }
};
#endif //PERSISTENTMANAGER_SERIALIZETRAITSBASE_H
