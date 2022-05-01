//
// Created by VincentZhang on 4/21/2022.
//

#ifndef PERSISTENTMANAGER_SERIALIZETRAITS_H
#define PERSISTENTMANAGER_SERIALIZETRAITS_H
#include "SerializeTraitsBase.h"
#include "Containers/CommonString.h"
#include <vector>
#include "Utilities/vector_utility.h"

#define DEFINE_GET_TYPESTRING_BASIC_TYPE(x)     \
    inline static const char* GetTypeString (void* p = NULL)    { return CommonString(x); } \
    inline static bool MightContainPPtr ()  { return false; }\
    inline static bool AllowTransferOptimization () { return true; }

template<>
struct SerializeTraits<float> : public SerializeTraitsBaseForBasicType<float>
{
    DEFINE_GET_TYPESTRING_BASIC_TYPE(float)
};

template<>
struct SerializeTraits<double> : public SerializeTraitsBaseForBasicType<double>
{
    DEFINE_GET_TYPESTRING_BASIC_TYPE(double)
};

template<>
struct SerializeTraits<SInt32> : public SerializeTraitsBaseForBasicType<SInt32>
{
    // We use "int" rather than "SInt32" here for backwards-compatibility reasons.
    // "SInt32" and "int" used to be two different types (as were "UInt32" and "unsigned int")
    // that we now serialize through same path.  We use "int" instead of "SInt32" as the common
    // identifier as it was more common.
    DEFINE_GET_TYPESTRING_BASIC_TYPE(int)
};

template<>
struct SerializeTraits<UInt32> : public SerializeTraitsBaseForBasicType<UInt32>
{
    DEFINE_GET_TYPESTRING_BASIC_TYPE(unsigned_int)  // See definition of "int" above.
};

template<>
struct SerializeTraits<SInt64> : public SerializeTraitsBaseForBasicType<SInt64>
{
    DEFINE_GET_TYPESTRING_BASIC_TYPE(SInt64)
};
template<>
struct SerializeTraits<UInt64> : public SerializeTraitsBaseForBasicType<UInt64>
{
    DEFINE_GET_TYPESTRING_BASIC_TYPE(UInt64)
};

template<>
struct SerializeTraits<SInt16> : public SerializeTraitsBaseForBasicType<SInt16>
{
    DEFINE_GET_TYPESTRING_BASIC_TYPE(SInt16)
};

template<>
struct SerializeTraits<UInt16> : public SerializeTraitsBaseForBasicType<UInt16>
{
    DEFINE_GET_TYPESTRING_BASIC_TYPE(UInt16)
};

template<>
struct SerializeTraits<SInt8> : public SerializeTraitsBaseForBasicType<SInt8>
{
    DEFINE_GET_TYPESTRING_BASIC_TYPE(SInt8)
};

template<>
struct SerializeTraits<UInt8> : public SerializeTraitsBaseForBasicType<UInt8>
{
    DEFINE_GET_TYPESTRING_BASIC_TYPE(UInt8)
};

template<>
struct SerializeTraits<char> : public SerializeTraitsBaseForBasicType<char>
{
    DEFINE_GET_TYPESTRING_BASIC_TYPE(char)
};

template<>
struct SerializeTraits<bool> : public SerializeTraitsBase<bool>
{
    typedef bool value_type;
    DEFINE_GET_TYPESTRING_BASIC_TYPE(bool)

    static int GetByteSize()   { return 1; }
    static bool AllowTypeConversion() { return false; }

    template<class TransferFunction> inline
    static void Transfer(value_type& data, TransferFunction& transfer)
    {
        Assert(sizeof(bool) == 1);
        UInt8& temp = reinterpret_cast<UInt8&>(data);
        transfer.TransferBasicData(temp);

        // When running in debug mode in OS X (-O0 in gcc),
        // bool values which are not exactly 0x01 are treated as false.
        // We don't want this. Cast UInt8 to bool to fix this.
#if DEBUGMODE
        if (transfer.IsReading())
            data = (temp != 0);
        // You constructor or Reset function is not setting the bool value to a defined value!
        Assert(!((transfer.IsReading() || transfer.IsWriting()) && (temp != 0 && temp != 1)));
#endif
    }
};



#define DEFINE_GET_TYPESTRING_MAP_CONTAINER(x)                      \
inline static const char* GetTypeString (void* p = NULL)    { return CommonString(x); }\
inline static bool MightContainPPtr ()  { return SerializeTraits<FirstClass>::MightContainPPtr() || SerializeTraits<SecondClass>::MightContainPPtr(); } \
inline static bool AllowTransferOptimization () { return false; }

template<class TStringType>
class SerializeTraitsForStringTypes : public SerializeTraitsBase<TStringType>
{
public:

    typedef TStringType value_type;
    inline static const char* GetTypeString(value_type* p = NULL)  { return CommonString(string); }
    inline static bool MightContainPPtr()  { return false; }
    inline static bool AllowTransferOptimization() { return false; }
    static bool AllowTypeConversion() { return false; }

    static void EnsureValid(value_type& data, TransferMetaFlags metaFlag)
    {
        const char* s_InvalidUtf8String = "INVALID_UTF8_STRING"; //if serialized string not in UTF_8 format, this string will be substituted instead
        if ((metaFlag & kDontValidateUTF8) == 0 && !IsStringValidUTF8(data))
        {
            data = s_InvalidUtf8String; //serialized strings should be stored in UTF-8 format
        }
    }

    template<class TransferFunction> inline
    static void Transfer(value_type& data, TransferFunction& transfer)
    {
        transfer.TransferSTLStyleArray(data, kHideInEditorMask);
        transfer.Align();
    }

    static bool IsContinousMemoryArray()   { return true; }

    static void ResizeSTLStyleArray(value_type& data, int rs)
    {
        data.resize(rs);
    }
};


// Remove once we have container label propergation
// Note: For core:string* types, also check Runtime\Serialize\TransferFunctions\TextSerializeTraits.h where template specialization is added via TextSerializeTraits
//template<int MemLabelId> class SerializeTraits<core::string_with_label<MemLabelId> > : public SerializeTraitsForStringTypes<core::string_with_label<MemLabelId> > {};
template<> class SerializeTraits<std::string> : public SerializeTraitsForStringTypes<std::string> {};

// Do not add this serialization function. All serialized strings should use core::string instead of core::string
//template<class Traits, class Allocator>
//class SerializeTraits<std::basic_string<char,Traits,Allocator> > : public SerializeTraitsBase<std::basic_string<char,Traits,Allocator> >

template<class T, class Allocator>
class SerializeTraits<std::vector<T, Allocator> > : public SerializeTraitsBase<std::vector<T, Allocator> >
{
public:

    typedef std::vector<T, Allocator>    value_type;
    DEFINE_GET_TYPESTRING_CONTAINER(vector);

    template<class TransferFunction> inline
    static void Transfer(value_type& data, TransferFunction& transfer)
    {
        transfer.TransferSTLStyleArray(data);
        transfer.Align();
    }

    static bool IsContinousMemoryArray()   { return true; }
    static void ResizeSTLStyleArray(value_type& data, int rs)        { resize_trimmed(data, rs); }
};

#endif //PERSISTENTMANAGER_SERIALIZETRAITS_H
