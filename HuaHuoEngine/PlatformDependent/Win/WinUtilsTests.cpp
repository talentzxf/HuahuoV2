#include "UnityPrefix.h"

#if ENABLE_UNIT_TESTS

#include "Runtime/Testing/Testing.h"
#include "WinUtils.h"

UNIT_TEST_SUITE(WinUtils)
{
    struct Fixture
    {
        template<size_t N> void CheckChopUpCommandLine(const char* commandLine, const char* (&expected)[N])
        {
            // As ChopUpCommandLine modifies the input string in place, we must pass in a buffer that can be modified
            char* buffer = new char[1024];
            strncpy_s(buffer, 1024, commandLine, _TRUNCATE);

            std::vector<const char*> res = winutils::ChopUpCommandLine(buffer);

            CHECK_EQUAL(N, res.size());

            if (N == res.size())
            {
                for (int i = 0; i < N; i++)
                {
                    CHECK_EQUAL(expected[i], res[i]);
                }
            }

            delete[] buffer;
        }
    };

    TEST_FIXTURE(Fixture, EnsureSimplestCommandLineIsChoppedUpProperly)
    {
        const char* expected[] = { "unity.exe" };
        CheckChopUpCommandLine("unity.exe", expected);
    }

    TEST_FIXTURE(Fixture, EnsureSimpleCommandLineIsChoppedUpProperly)
    {
        const char* expected[] = { "C:\\Some\\path\\to\\unity.exe" };
        CheckChopUpCommandLine("C:\\Some\\path\\to\\unity.exe", expected);
    }

    TEST_FIXTURE(Fixture, EnsureSimpleCommandLineWithSpacesInPathIsChoppedUpProperly)
    {
        const char* expected[] = { "C:\\Some\\path\\with some spaces\\to\\unity.exe" };
        CheckChopUpCommandLine("\"C:\\Some\\path\\with some spaces\\to\\unity.exe\"", expected);
    }

    // Bug 823993
    TEST_FIXTURE(Fixture, EnsureSimpleCommandLineWithSpacesAndWeirdlyPlacedQuotesInPathIsChoppedUpProperly)
    {
        const char* expected[] = { "C:\\Some\\path\\with some spaces\\to\\unity.exe" };
        CheckChopUpCommandLine("\"C:\\Some\\path\\with some spaces\\to\\\"unity.exe", expected);
    }

    TEST_FIXTURE(Fixture, EnsureSimplestCommandLineWithParamsIsChoppedUpProperly)
    {
        const char* expected[] = { "unity.exe", "-batchmode" };
        CheckChopUpCommandLine("unity.exe -batchmode", expected);
    }

    TEST_FIXTURE(Fixture, EnsureCommandLineWithParamsIsChoppedUpProperly)
    {
        const char* expected[] = { "C:\\Some\\path\\to\\unity.exe", "-batchmode" };
        CheckChopUpCommandLine("C:\\Some\\path\\to\\unity.exe -batchmode", expected);
    }

    TEST_FIXTURE(Fixture, EnsureCommandLineWithSpacesInPathWithParamsIsChoppedUpProperly)
    {
        const char* expected[] = { "C:\\Some\\path\\with some spaces\\to\\unity.exe", "-batchmode" };
        CheckChopUpCommandLine("\"C:\\Some\\path\\with some spaces\\to\\unity.exe\" -batchmode", expected);
    }

    // Bug 823993
    TEST_FIXTURE(Fixture, EnsureCommandLineWithSpacesAndWeirdlyPlacedQuotesInPathWithParamsIsChoppedUpProperly)
    {
        const char* expected[] = { "C:\\Some\\path\\with some spaces\\to\\unity.exe", "-batchmode" };
        CheckChopUpCommandLine("\"C:\\Some\\path\\with some spaces\\to\\\"unity.exe -batchmode", expected);
    }

    TEST_FIXTURE(Fixture, EnsureSimplestCommandLineWithQuotedParamsIsChoppedUpProperly)
    {
        const char* expected[] = { "unity.exe", "-createProject", "c:\\path to\\new\\project\\" };
        CheckChopUpCommandLine("unity.exe -createProject \"c:\\path to\\new\\project\\\"", expected);
    }

    TEST_FIXTURE(Fixture, EnsureCommandLineWithQuotedParamsIsChoppedUpProperly)
    {
        const char* expected[] = { "C:\\Some\\path\\to\\unity.exe", "-createProject", "c:\\path to\\new\\project\\" };
        CheckChopUpCommandLine("C:\\Some\\path\\to\\unity.exe -createProject \"c:\\path to\\new\\project\\\"", expected);
    }

    TEST_FIXTURE(Fixture, EnsureCommandLineWithSpacesInPathWithQuotedParamsIsChoppedUpProperly)
    {
        const char* expected[] = { "C:\\Some\\path\\with some spaces\\to\\unity.exe", "-createProject", "c:\\path to\\new\\project\\" };
        CheckChopUpCommandLine("\"C:\\Some\\path\\with some spaces\\to\\unity.exe\" -createProject \"c:\\path to\\new\\project\\\"", expected);
    }

    // Bug 823993
    TEST_FIXTURE(Fixture, EnsureCommandLineWithSpacesAndWeirdlyPlacedQuotesInPathWithQuotedParamsIsChoppedUpProperly)
    {
        const char* expected[] = { "C:\\Some\\path\\with some spaces\\to\\unity.exe", "-createProject", "c:\\path to\\new\\project\\" };
        CheckChopUpCommandLine("\"C:\\Some\\path\\with some spaces\\to\\\"unity.exe -createProject \"c:\\path to\\new\\project\\\"", expected);
    }
}

#endif // ENABLE_UNIT_TESTS
