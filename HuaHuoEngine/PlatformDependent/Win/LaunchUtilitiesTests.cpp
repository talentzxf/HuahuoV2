#include "UnityPrefix.h"

#if ENABLE_UNIT_TESTS_WITH_FAKES

#include "Runtime/Testing/Faking.h"
#include "Runtime/Testing/TestFixtures.h"
#include "Runtime/Testing/Testing.h"
#include "PathUnicodeConversion.h"

#include <ShellAPI.h>

#include "Runtime/Utilities/LaunchUtilities.h"

using namespace testing;

UNIT_TEST_SUITE(LaunchUtilities)
{
    struct ShellFixture
    {
        FAKE_SYSTEM_FUNCTION(ShellExecuteExW, BOOL(SHELLEXECUTEINFOW*));
        FAKE_SEQUENCE_POINT(LaunchApplication, ConvertPathSeparators);
    };

    TEST_FIXTURE(ShellFixture, LaunchApplicationWithUnixPathIsConvertedToWindowsPathSeparators)
    {
        ShellExecuteExW.Returns(true);

        dynamic_array<core::string> arr;
        arr.emplace_back("./test-path");

        LaunchApplication("test", arr, false, false);

        ShellExecuteExW.MustBeCalled();
        LaunchApplication_ConvertPathSeparators.MustBeCalled();
    }

    TEST_FIXTURE(ShellFixture, LaunchApplicationWithForwardSlashArgumentIsNotConvertedToWindowsPathSeparators)
    {
        ShellExecuteExW.Returns(true);

        dynamic_array<core::string> arr;
        arr.emplace_back("/e");

        LaunchApplication("test", arr, false, false);

        ShellExecuteExW.MustBeCalled();
        LaunchApplication_ConvertPathSeparators.MustNotBeCalled();
    }

    TEST_FIXTURE(ShellFixture, LaunchApplicationWithEmptyStringArgumentIsNotConvertedToWindowsPathSeparators)
    {
        ShellExecuteExW.Returns(true);

        dynamic_array<core::string> arr;
        arr.emplace_back("");

        LaunchApplication("test", arr, false, false);

        ShellExecuteExW.MustBeCalled();
        LaunchApplication_ConvertPathSeparators.MustNotBeCalled();
    }
}

#endif
