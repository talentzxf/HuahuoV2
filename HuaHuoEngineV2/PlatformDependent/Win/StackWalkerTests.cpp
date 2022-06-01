#include "UnityPrefix.h"
#include "Configuration/UnityConfigure.h"
#include "Runtime/Testing/PerformanceTesting.h"

// Stackwalker is not supported for WinRT ARM, so don't compile the tests there
#if ENABLE_UNIT_TESTS_WITH_FAKES && !(PLATFORM_WINRT && defined(_ARM_))

#include "Runtime/Testing/Testing.h"
#include "Runtime/Testing/Faking.h"
#include "StackWalker.h"

#define STACKWALKER_NOINLINE __declspec(noinline)

INTEGRATION_TEST_SUITE(StackWalker)
{
    struct Fixture
    {
        FAKE_METHOD(StackWalker, OnOutput, void(LPCSTR));
        FAKE_METHOD(StackWalker, OnCallstackEntry, void(StackWalker::CallstackEntryType, StackWalker::CallstackEntry&));

        StackWalker m_stackWalker;
        dynamic_array<core::string> m_frameNames;

        Fixture()
        {
            StackWalker_OnOutput.Calls(this, &Fixture::OnOutput);
            StackWalker_OnCallstackEntry.Calls(this, &Fixture::OnCallstackEntry);
        }

        bool FoundFrameWithMethod(const char* name)
        {
            for (dynamic_array<core::string>::const_iterator iter = m_frameNames.begin(); iter != m_frameNames.end(); ++iter)
                if (strstr(iter->c_str(), name))
                    return true;
            return false;
        }

        void OnCallstackEntry(StackWalker* stackWalker, StackWalker::CallstackEntryType eType, StackWalker::CallstackEntry &entry)
        {
            m_frameNames.push_back(entry.name);
        }

        void OnOutput(StackWalker* stackWalker, LPCSTR szText)
        {
        }

        static int FilterFunction(StackWalker& stackWalker, EXCEPTION_POINTERS *ep)
        {
            stackWalker.ShowCallstack(NULL, ep->ContextRecord);
            return EXCEPTION_EXECUTE_HANDLER;
        }

        STACKWALKER_NOINLINE static int LeafFunctionWithRaiseException()
        {
            RaiseException(EXCEPTION_ACCESS_VIOLATION, NULL, NULL, NULL);
            return 10;
        }

        STACKWALKER_NOINLINE static int FunctionWithRaiseException(StackWalker& stackWalker)
        {
            __try
            {
                LeafFunctionWithRaiseException();
            }
            __except (FilterFunction(stackWalker, GetExceptionInformation()))
            {
                return 1;
            }

            return 0;
        }

        STACKWALKER_NOINLINE static int LeafFunctionWithAccessViolation()
        {
            const char* buf[] = { "A", "B", "C", NULL };
            for (int i = 0; i < sizeof(buf) / sizeof(buf[0]); i++)
            {
                // we will access the NULL entry above and cause an ACCESS VIOLATION.
                if (*buf[i] == 'D')
                    return 1;
            }
            return 0;
        }

        STACKWALKER_NOINLINE static int FunctionWithAccessViolation(StackWalker& stackWalker)
        {
            __try
            {
                return LeafFunctionWithAccessViolation();
            }
            __except (FilterFunction(stackWalker, GetExceptionInformation()))
            {
                return 1;
            }

            return 0;
        }
    };

    TEST_FIXTURE(Fixture, ShowCallstack_IncludesCurrentFunction)
    {
        CHECK(this->m_stackWalker.ShowCallstack());
        CHECK(this->FoundFrameWithMethod("ShowCallstack_IncludesCurrentFunction"));
    }

    TEST_FIXTURE(Fixture, ShowCallstack_CanWalkContextFromRaiseException)
    {
        FunctionWithRaiseException(m_stackWalker);

        CHECK(FoundFrameWithMethod("FunctionWithRaiseException"));
        CHECK(FoundFrameWithMethod("LeafFunctionWithRaiseException"));
    }

    TEST_FIXTURE(Fixture, ShowCallstack_CanWalkContextFromAccessViolation)
    {
        FunctionWithAccessViolation(m_stackWalker);

        CHECK(FoundFrameWithMethod("FunctionWithAccessViolation"));
        CHECK(FoundFrameWithMethod("LeafFunctionWithAccessViolation"));
    }
}

#if ENABLE_PERFORMANCE_TESTS

PERFORMANCE_TEST_SUITE(StackWalker)
{
    TEST(LoadModules)
    {
        PERFORMANCE_TEST_LOOP_TIMELIMIT(1000, 10000000)
        {
            StackWalker stackWalker;
            stackWalker.LoadModules();
        }
    }

    TEST(ShowCallstack)
    {
        StackWalker stackWalker;
        FAKE_METHOD(StackWalker, OnOutput, void(LPCSTR));
        FAKE_METHOD(StackWalker, OnCallstackEntry, void(StackWalker::CallstackEntryType, StackWalker::CallstackEntry&));

        // Call ShowCallstack once to force deferred modules to get loaded
        stackWalker.ShowCallstack();

        PERFORMANCE_TEST_LOOP(1000)
        {
            stackWalker.ShowCallstack();
        }
    }
}

#endif

#endif
