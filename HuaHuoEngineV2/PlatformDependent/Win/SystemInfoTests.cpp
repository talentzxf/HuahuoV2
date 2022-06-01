#if ENABLE_UNIT_TESTS_WITH_FAKES

#include "Runtime/Testing/Testing.h"

namespace systeminfo
{
    core::string FormatMacAddress(BYTE* addr, ULONG length);
    core::string InsertColonEveryTwoChars(const core::string& macAddr);
}

UNIT_TEST_SUITE(WindowsSystemInfo)
{
    core::string FormatMacAddress(const dynamic_array<unsigned char>& addr)
    {
        return systeminfo::FormatMacAddress((BYTE*)addr.data(), addr.size());
    }

    TEST(FormatMacAddress)
    {
        CHECK_EQUAL("", FormatMacAddress({}));
        CHECK_EQUAL("01", FormatMacAddress({ 1 }));
        CHECK_EQUAL("01:02", FormatMacAddress({ 1, 2 }));
        CHECK_EQUAL("01:02:03:04:05:06", FormatMacAddress({ 1, 2, 3, 4, 5, 6 }));
        // max is 8 bytes (MAX_ADAPTER_ADDRESS_LENGTH)
        CHECK_EQUAL("01:02:03:04:05:06:07:08", FormatMacAddress({ 1, 2, 3, 4, 5, 6, 7, 8, 9 }));

        CHECK_EQUAL("11:ff:dd:01:11:01", FormatMacAddress({ 0x11, 0xff, 0xdd, 0x01, 0x11, 0x01 }));
        CHECK_EQUAL("61:62:63:64:65:66", FormatMacAddress({ 'a', 'b', 'c', 'd', 'e', 'f' }));
    }

    TEST(InsertColonEveryTwoChars)
    {
        using systeminfo::InsertColonEveryTwoChars;

        CHECK_EQUAL("", InsertColonEveryTwoChars(""));
        CHECK_EQUAL("a", InsertColonEveryTwoChars("a"));
        CHECK_EQUAL("ab", InsertColonEveryTwoChars("ab"));
        CHECK_EQUAL("ab:c", InsertColonEveryTwoChars("abc"));
        CHECK_EQUAL("ab:cd", InsertColonEveryTwoChars("abcd"));
        CHECK_EQUAL("ab:cd:e", InsertColonEveryTwoChars("abcde"));
    }
}

#endif
