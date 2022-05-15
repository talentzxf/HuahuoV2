#include "UnityPrefix.h"

#if ENABLE_UNIT_TESTS

#include "Runtime/Testing/Testing.h"
#include "Runtime/Testing/ParametricTest.h"
#include "Runtime/Utilities/Word.h"
#include "FileInformation.h"
#include <processenv.h>

INTEGRATION_TEST_SUITE(FileInformation)
{
    TEST(FileHasInformation)
    {
        // This test check that a file (cmd.exe) has extra file information (version etc.)
        // cmd.exe is used as this is expected to always be available
        wchar_t cmdPath[1024];
        const DWORD size = ::GetEnvironmentVariableW(L"ComSpec", cmdPath, sizeof(cmdPath));

        CHECK(size > 0 && size < sizeof(cmdPath));
        if (!(size > 0 && size < sizeof(cmdPath))) // check required to continue
            return;

        FileInformation fileInfo(cmdPath);
        CHECK(fileInfo.ValidFileInformation());
        if (!fileInfo.ValidFileInformation()) // check required to continue
            return;

        // cmd.exe major versions expected to be higher than 0
        CHECK(fileInfo.GetFileVersion().v1 > 0);
        CHECK(fileInfo.GetProductVersion().v1 > 0);

        // expects Windows within product name
        CHECK_NOT_NULL(fileInfo.GetProductName());
        if (!fileInfo.GetProductName()) // check required to continue
            return;

        CHECK_MSG(StrStr(fileInfo.GetProductName(), "Windows") > 0, fileInfo.GetProductName());

        // expects Microsoft within company name
        CHECK_NOT_NULL(fileInfo.GetCompanyName());
        if (!fileInfo.GetCompanyName()) // check required to continue
            return;

        CHECK_MSG(StrStr(fileInfo.GetCompanyName(), "Microsoft") > 0, fileInfo.GetCompanyName());
    }

    void FileInformationNotValidTestCaseSource(Testing::TestCaseEmitter<const wchar_t*>& testCase)
    {
        testCase.WithName("UnknownFile").WithValues(L"c:\\randomfile42.exe");
        testCase.WithName("InvalidFilePath").WithValues(L"specialdrive:\\randomfile42.exe");
    }

    PARAMETRIC_TEST(FileInformationNotValid, (const wchar_t* filepath), FileInformationNotValidTestCaseSource)
    {
        FileInformation fileInfo(filepath);

        CHECK(!fileInfo.ValidFileInformation());
        CHECK_EQUAL(0, fileInfo.GetFileVersion().v1);
        CHECK_EQUAL(0, fileInfo.GetFileVersion().v2);
        CHECK_EQUAL(0, fileInfo.GetFileVersion().v3);
        CHECK_EQUAL(0, fileInfo.GetFileVersion().v4);

        CHECK_EQUAL(0, fileInfo.GetProductVersion().v1);
        CHECK_EQUAL(0, fileInfo.GetProductVersion().v2);
        CHECK_EQUAL(0, fileInfo.GetProductVersion().v3);
        CHECK_EQUAL(0, fileInfo.GetProductVersion().v4);

        CHECK_NULL(fileInfo.GetProductName());
        CHECK_NULL(fileInfo.GetCompanyName());
    }
}

#endif // ENABLE_UNIT_TESTS
