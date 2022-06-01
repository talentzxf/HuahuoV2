#include "CommonString.h"

namespace HuaHuo
{ namespace CommonString
{
    // make the string buffer
    const char gStringBuffer[] =
#define COMMON_STRING_ENTRY(a, b) #b "\0"
#include "CommonStrings.h"
#undef COMMON_STRING_ENTRY
    ;

    const char* const BufferBegin = gStringBuffer;
    const char* const BufferEnd = gStringBuffer + sizeof(gStringBuffer);

    // compile-time offsets (into the buffer)
    template<int StringId> struct Info;

    template<> struct Info<IdBase>
    {
        enum
        {
            Offset = -1,
            Size = 1
        };
    };

#define COMMON_STRING_ENTRY(a, b)                                        \
    template <> struct Info<Id_##a>                                     \
    {                                                                   \
        enum                                                            \
        {                                                               \
            Offset = Info<Id_##a - 1>::Offset + Info<Id_##a - 1>::Size, \
            Size = sizeof(#b)                                           \
        };                                                              \
    };
#include "CommonStrings.h"
#undef COMMON_STRING_ENTRY

    template<> struct Info<IdEmpty>
    {
        enum
        {
            Offset = Info<IdEmpty - 1>::Offset + Info<IdEmpty - 1>::Size,
            Size = 0
        };
    };

#define COMMON_STRING_ENTRY(a, b) const char* const gLiteral_##a = gStringBuffer + Info<Id_##a>::Offset;
#include "CommonStrings.h"
#undef COMMON_STRING_ENTRY
    const char* const gLiteralEmpty = gStringBuffer + Info<IdEmpty>::Offset;
} }
