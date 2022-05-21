//
// Created by VincentZhang on 5/21/2022.
//

#ifndef HUAHUOENGINE_ENUMTRAITS_H
#define HUAHUOENGINE_ENUMTRAITS_H


// EnumTraits contains methods for accessing useful information about enums at runtime.
//
// The default implementation of all the methods in this namespace assume that you declared an enum with
// REFLECTABLE_ENUM. That said, they're also templated, so if you need to make them work for an enum
// that is *not* using REFLECTABLE_ENUM, you should specialize them for that particular enum type.

#include <cctype> // for isspace()
#include "Utilities/TypeUtilities.h"
#include "Logging/LogAssert.h"
#include <cstring>

struct ReflectableEnum;
template<typename TIterator, typename TString>
TIterator FindStringInRange(TIterator begin, TIterator end, TString str, bool ignoreCase);

namespace EnumTraits
{
    // Returns the total number of entries in the given enum.
    template<typename TEnumImpl>
    size_t Count()
    {
        return TEnumImpl::Count();
    }

    // Retrieve the names of all the values in the enum. The indices of the names are guaranteed
    // to match the order in which they are declared.
    template<typename TEnumImpl>
    const char* const* GetNames()
    {
        return TEnumImpl::Names();
    }

    // Retrieve the numeric values of all valid enum entries. The indices of the values are guaranteed
    // to match the order in which they are declared.
    template<typename TEnumImpl>
    const int* GetValues()
    {
        return TEnumImpl::Values();
    }

    // Retrieve the annotation strings associated with the enum entries. The indices of the values are guaranteed
    // to match the order in which they are delared. Entries with no annotation will contain NULL.
    template<typename TEnumImpl>
    const char* const* GetAnnotations()
    {
        return TEnumImpl::Annotations();
    }

    template<typename TEnumImpl>
    bool IsFlags()
    {
        return TEnumImpl::IsFlags();
    }

    namespace internal
    {
        template<typename E>
        struct IsReflectableEnum : public IsConvertible<E, ReflectableEnum> {};

        template<typename E, bool IRE>
        struct DefaultTraitsHelper;

        template<typename E>
        struct DefaultTraitsHelper<E, true>
        {
            static int ToInt(E value) { return static_cast<typename E::ActualEnumType>(value); }
            static E FromIntUnchecked(int value) { return static_cast<typename E::ActualEnumType>(value); }
        };

        template<typename E>
        struct DefaultTraitsHelper<E, false>
        {
            static int ToInt(E value) { return static_cast<int>(value); }
            static E FromIntUnchecked(int value) { return static_cast<E>(value); }
        };
    }

    // "Unwraps" an enum value into a raw integer, for use when indexing things etc.
    template<typename TEnumImpl>
    int ToInt(TEnumImpl value)
    {
        return internal::DefaultTraitsHelper<TEnumImpl, internal::IsReflectableEnum<TEnumImpl>::value>::ToInt(value);
    }

    // Check whether the given integer corresponds to some defined enum value, or in the case of a flags enum, a combined set of values
    template<typename TEnumImpl>
    bool IsValidValue(int value)
    {
        const int* begin = GetValues<TEnumImpl>();
        const int* end = begin + Count<TEnumImpl>();
        if (IsFlags<TEnumImpl>())
        {
            int mask = value;
            for (const int* i = begin; i != end; ++i)
                mask &= ~(*i);
            return mask == 0;
        }
        else
        {
            return std::find(begin, end, value) != end;
        }
    }

    template<typename TEnumImpl>
    TEnumImpl FromIntUnchecked(int value)
    {
        return internal::DefaultTraitsHelper<TEnumImpl, internal::IsReflectableEnum<TEnumImpl>::value>::FromIntUnchecked(value);
    }

    template<typename TEnumImpl>
    bool TryFromInt(int value, TEnumImpl& outResult)
    {
        if (!IsValidValue<TEnumImpl>(value))
            return false;
        outResult = FromIntUnchecked<TEnumImpl>(value);
        return true;
    }

    // Converts an integer into an enum value.
    template<typename TEnumImpl>
    TEnumImpl FromInt(int value)
    {
        TEnumImpl result = FromIntUnchecked<TEnumImpl>(0);
        if (!TryFromInt<TEnumImpl>(value, result))
            AssertString("Trying to construct an enum instance from an invalid integer value");
        return result;
    }

    // Check whether the given string is the name of an entry in the enum.
    template<typename TEnumImpl>
    bool IsValidName(const char* name, bool ignoreCase)
    {
        const char* const* begin = GetNames<TEnumImpl>();
        const char* const* end = begin + Count<TEnumImpl>();
        return FindStringInRange(begin, end, name, ignoreCase) != end;
    }

    // Same again but letting people omit the 'ignore case' argument. Can't use default
    // parameters for this because it prevents us from being able to forward-declare it.
    template<typename TEnumImpl>
    bool IsValidName(const char* name)
    {
        return IsValidName<TEnumImpl>(name, true);
    }

    template<typename TEnumImpl>
    bool TryFromString(const char* name, bool ignoreCase, TEnumImpl& outResult)
    {
        const char* comma;
        if (IsFlags<TEnumImpl>() && (comma = strchr(name, ',')) != NULL)
        {
            // The string contains a comma which means it is probably multiple values that we should OR together

            if (comma == name)
                return false; // string starts with a comma, invalid

            const char* end = comma - 1;
            while (end > name && std::isspace(*end))
                --end;

            if (end <= name)
                return false; // string was only whitespace followed by a comma

            size_t firstLen = end - name + 1;
            char* buffer = static_cast<char*>(alloca(firstLen + 1));
            memcpy(buffer, name, firstLen);
            buffer[firstLen] = '\0';

            // Convert the first name
            if (!TryFromString(buffer, ignoreCase, outResult))
                return false;

            const char* nextStart = comma + 1;
            while (*nextStart != 0 && std::isspace(*nextStart))
                ++nextStart;

            if (*nextStart != 0)
            {
                TEnumImpl remainder = FromIntUnchecked<TEnumImpl>(0);
                if (!TryFromString(nextStart, ignoreCase, remainder))
                    return false;

                outResult = FromIntUnchecked<TEnumImpl>(outResult | remainder);
            }

            return true;
        }

        const char* const* names = GetNames<TEnumImpl>();
        const size_t count = Count<TEnumImpl>();

        size_t index = std::distance(names, FindStringInRange(names, names + count, name, ignoreCase));
        if (index >= count)
            return false;

        outResult = FromIntUnchecked<TEnumImpl>(GetValues<TEnumImpl>()[index]);
        return true;
    }

    template<typename TEnumImpl>
    bool TryFromString(const char* name, TEnumImpl& outResult)
    {
        return TryFromString(name, true, outResult);
    }

    // Given a string, return the corresponding enum value.
    // You should make sure that the string is a valid value before calling.
    template<typename TEnumImpl>
    TEnumImpl FromString(const char* name, bool ignoreCase)
    {
        TEnumImpl result = FromIntUnchecked<TEnumImpl>(0);
        if (!TryFromString(name, ignoreCase, result))
            AssertString(Format("Unrecognised name \"%s\" for enum.", name));
        return result;
    }

    // As above - work around not being able to use default parameters
    template<typename TEnumImpl>
    TEnumImpl FromString(const char* name)
    {
        return FromString<TEnumImpl>(name, true);
    }

    // Stringify the given enum value.
    template<typename TEnumImpl>
    std::string ToString(TEnumImpl value)
    {
        const int* begin = GetValues<TEnumImpl>();
        const size_t count = Count<TEnumImpl>();
        const int* end = begin + count;
        const char* const* names = GetNames<TEnumImpl>();

        std::string result;

        if (IsFlags<TEnumImpl>())
        {
            int intValue = value;
            for (size_t index = 0; index < count; ++index)
            {
                // Actually test the last value first, as we will tend to define enums in increasing value order
                int enumValue = begin[count - index - 1];

                // If the enum value is an exact match for this one, just return it immediately.
                if (enumValue == (int)value)
                {
                    result.assign(names[count - index - 1]);
                    return result;
                }

                if (enumValue == 0)
                    continue;

                if ((intValue & enumValue) == enumValue)
                {
                    // this value is included
                    const char* name = names[count - index - 1];

                    if (result.size() == 0)
                        result.assign(name);
                    else
                    {
                        result.insert(0, ", ");
                        result.insert(0, name);
                    }

                    intValue &= ~enumValue;
                }
            }

            return result;
        }
        else
        {
            size_t index = std::distance(begin, std::find(begin, end, value));

            if (index >= count)
            {
                AssertString(Format("Unrecognised value %i for enum", (int)value));
                result.assign("<UNKNOWN>");
                return result;
            }

            result.assign(names[index]);
            return result;
        }
    }

    // Non-templated struct to hold all the reflection information about an enum. Useful to
    // use as a type token, or just to do stuff like name/value checks without needing to
    // reference the actual enum type itself.
    struct ReflectionInfo
    {
        size_t count;
        const char* const* names;
        const int* values;
        const char* const* annotations;
        bool isFlags;

        const char* GetNameForValue(int value) const;
    };

    template<typename TEnumImpl>
    const ReflectionInfo& GetReflectionInfo()
    {
        static ReflectionInfo info;
        static bool isInitialized = false;
        if (!isInitialized)
        {
            info.count = Count<TEnumImpl>();
            info.names = GetNames<TEnumImpl>();
            info.values = GetValues<TEnumImpl>();
            info.annotations = GetAnnotations<TEnumImpl>();
            info.isFlags = IsFlags<TEnumImpl>();
            isInitialized = true;
        }
        return info;
    }

    // Structure containing compile-time properties for an enum type
    template<typename TEnumImpl>
    struct StaticTraits
    {
        // The type that should be used for constant expressions.
        // This is used to e.g. declare static const values such that the optimizer is able to understand them correctly.
        typedef typename TEnumImpl::ActualEnumType ConstantExpressionType;

        // The number of entries in the enum, as a compile-time constant.
        enum { Count = TEnumImpl::_StaticCount };
    };
}


#endif //HUAHUOENGINE_ENUMTRAITS_H
