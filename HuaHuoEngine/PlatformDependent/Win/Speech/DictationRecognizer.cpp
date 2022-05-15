#include "UnityPrefix.h"
#include "DictationRecognizer.h"

#if PLATFORM_WIN

#include "External/Windows10/src/ComHelpers.h"
#include "External/Windows10/src/WinRTCollections.h"
#include "PlatformDependent/Win/etw/ETW.h"
#include "PlatformDependent/Win/GetFormattedErrorString.h"
#include "Runtime/EventQueue/GlobalEventQueue.h"
#include "Runtime/Scripting/CommonScriptingClasses.h"
#include "Runtime/Scripting/Scripting.h"
#include "Runtime/Scripting/ScriptingUtility.h"

namespace UnityWinRTBase
{
    using namespace UnityWinRTBase::Windows::Foundation;
    using namespace UnityWinRTBase::Windows::Foundation::Collections;
    using namespace UnityWinRTBase::Windows::Media::SpeechRecognition;
}

namespace Unity
{
    struct DictationResultBuffer
    {
        Unity::DictationRecognizer* recognizer;
        UnityWinRTBase::HSTRING text;
        int confidence;

        void Destroy()
        {
            // nb:  WindowsDeleteString always returns S_OK
            UnityWinRTBase::WindowsDeleteString(text);
        }
    };

    struct DictationHypothesisBuffer
    {
        Unity::DictationRecognizer *recognizer;
        UnityWinRTBase::HSTRING text;
        int confidence;

        void Destroy()
        {
            UnityWinRTBase::WindowsDeleteString(text);
        }
    };

    struct DictationCompletedBuffer
    {
        Unity::DictationRecognizer *recognizer;
        UnityWinRTBase::SpeechRecognitionResultStatus cause;
    };

    // this is largely copied from the one in GestureRecognizer.cpp
    struct DictationErrorBuffer
    {
        Unity::DictationRecognizer *recognizer;
        char *errorMessage;
        int hresult;

        void Set(int hr, Unity::DictationRecognizer *gr, const char * format, va_list va_args)
        {
            hresult = hr;
            DebugAssertMsg(gr, "DictationErrorBuffer::Set was called with a null DictationRecognizer!");
            recognizer = gr;
            int len = vsnprintf(nullptr, 0, format, va_args);
            errorMessage = (char*)UNITY_MALLOC(kMemSpeech, len + 1);
            vsprintf(errorMessage, format, va_args);
        }

        void Destroy()
        {
            UNITY_FREE(kMemSpeech, (void*)errorMessage);
        }
    };

    struct DictationCompileComplete
    {
        Unity::DictationRecognizer *recognizer;
        bool compileSuccessful; // fatal otherwise
    };

    struct DictationAsyncComplete
    {
        Unity::DictationRecognizer *recognizer;
        UnityWinRTBase::AsyncStatus status;
    };

    struct DictationDestructionBuffer
    {
        DictationRecognizer* recognizer;
        ScriptingBackendNativeGCHandle gcHandle;

        void Destroy()
        {
            ScriptingGCHandle::FromScriptingBackendNativeGCHandle(gcHandle).ReleaseAndClear();
        }
    };

    static inline void WriteDictationEtwEvent(const char* msg, DictationRecognizer* instance)
    {
#if ENABLE_EVENT_TRACING_FOR_WINDOWS
        const char kDictationEtwCategory[] = "DictationRecognizer";
        EventWriteRuntimeEvent(kDictationEtwCategory, FormatString("(0x%llX) %s", (int64_t)instance, msg).c_str());
#endif
    }

    bool DictationRecognizer::VerifyAndReportHR(HRESULT hr, const char * format, ...)
    {
        Assert(SUCCEEDED(hr));
        if (!SUCCEEDED(hr))
        {
            va_list va_args;
            va_start(va_args, format);
            DictationErrorBuffer event;
            event.Set(hr, this, format, va_args);
            auto errorMsg = GetHResultErrorMessage(hr);
            ErrorStringMsg("%s [%s]", event.errorMessage, errorMsg.c_str());
            GlobalEventQueue::GetInstance().SendEvent(event);
        }
        return SUCCEEDED(hr);
    }

    void DictationRecognizer::ReportDictationError(const char * format, ...)
    {
        va_list va_args;
        va_start(va_args, format);
        DictationErrorBuffer event;
        event.Set(0, this, format, va_args);
        ErrorStringMsg(event.errorMessage);
        WriteDictationEtwEvent(event.errorMessage, this);
        GlobalEventQueue::GetInstance().SendEvent(event);
    }

    inline float TimeSpanToSeconds(UnityWinRTBase::Windows::Foundation::TimeSpan ts)
    {
        // TimeSpan is measured in inconvenient 100ns ticks
        return ts.Duration / (10.0f * 1000.0f * 1000.0f);
    }

    inline void SecondsToTimeSpan(UnityWinRTBase::Windows::Foundation::TimeSpan &outTs, float seconds)
    {
        outTs.Duration = (INT64)(seconds * 10.0f * 1000.0f * 1000.0f);
    }
} // Namespace Unity

REGISTER_EVENT_ID_WITH_CLEANUP(0xBBC1C377E707B94AULL, 0xBA7B3A8059E9A1EDULL, Unity::DictationResultBuffer)
REGISTER_EVENT_ID_WITH_CLEANUP(0x71325B306276B442ULL, 0x976AB25C6FBAC6A7ULL, Unity::DictationHypothesisBuffer)
REGISTER_EVENT_ID(0x398A87DE20656A41ULL, 0x9C17579FF8AD71BFULL, Unity::DictationCompletedBuffer)
REGISTER_EVENT_ID_WITH_CLEANUP(0xB25E4F21DA1C1247ULL, 0x8CD766AABE777E5AULL, Unity::DictationErrorBuffer)
REGISTER_EVENT_ID(0xF2604B5C07A3E047ULL, 0x87D95E05968B22D8ULL, Unity::DictationCompileComplete)
REGISTER_EVENT_ID(0x83E9879B57AA7045ULL, 0x9A4757C26CA07536ULL, Unity::DictationAsyncComplete)
REGISTER_EVENT_ID_WITH_CLEANUP(0xE7A634EBD0FF44D7ULL, 0xB5B79C0A8EAD812EULL, Unity::DictationDestructionBuffer)

namespace Unity
{
#define USE_FAST_DEBUG_PRINTS HL_SUPPORTS_SPATIALMAPPING
#if USE_FAST_DEBUG_PRINTS
    // faster debug print utilty.
    void PrintDebugLine(const char *format, ...);
#define DEBUG_PRINT PrintDebugLine
#else
#define DEBUG_PRINT printf_console
#endif

    struct DictationSessionEventHandler :
        public  UnityWinRTBase::ComClass<
            UnityWinRTBase::ITypedEventHandler<UnityWinRTBase::SpeechContinuousRecognitionSession*, UnityWinRTBase::SpeechContinuousRecognitionResultGeneratedEventArgs*>,
            UnityWinRTBase::ITypedEventHandler<UnityWinRTBase::SpeechContinuousRecognitionSession*, UnityWinRTBase::SpeechContinuousRecognitionCompletedEventArgs*>,
            UnityWinRTBase::ITypedEventHandler<UnityWinRTBase::SpeechRecognizer*, UnityWinRTBase::SpeechRecognitionHypothesisGeneratedEventArgs*>,
            UnityWinRTBase::IAsyncOperationCompletedHandler<UnityWinRTBase::SpeechRecognitionCompilationResult*>,
            UnityWinRTBase::IAsyncActionCompletedHandler>
    {
    private:
        win::ComPtr<DictationRecognizer> m_DictationRecognizer;

    public:
        inline DictationSessionEventHandler(DictationRecognizer* dictRecog) :
            m_DictationRecognizer(dictRecog)
        {
            SetLabel(kMemSpeech);
        }

        // ResultGenerated handler
        virtual HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::ISpeechContinuousRecognitionSession* sender, UnityWinRTBase::ISpeechContinuousRecognitionResultGeneratedEventArgs* args) override
        {
            WriteDictationEtwEvent("Result generated on worker thread", m_DictationRecognizer);

            win::ComPtr<UnityWinRTBase::ISpeechRecognitionResult> result;
            auto hr = args->get_Result(&result);
            Assert(SUCCEEDED(hr));

            UnityWinRTBase::HSTRING text;
            hr = result->get_Text(&text);
            Assert(SUCCEEDED(hr));

            UnityWinRTBase::SpeechRecognitionConfidence confidence;
            hr = result->get_Confidence(&confidence);
            Assert(SUCCEEDED(hr));

            DictationResultBuffer resultBuffer = { m_DictationRecognizer, text, static_cast<int>(confidence) };
            GlobalEventQueue::GetInstance().SendEvent(resultBuffer);

            // note:  we used to ProcessNextTask here which would chug through
            // tasks almost as soon as they could be handled.  unfortunately
            // since we've pulled out all the locks and other "is this ptr ok"
            // checks, we have to kick it back to the main thread where it
            // should be safe.  this introduces a one frame latency between
            // receiving the completion of the previous task and starting the
            // next one.
            return S_OK;
        }

        // hypothesis updated handler
        virtual HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::ISpeechRecognizer* sender, UnityWinRTBase::ISpeechRecognitionHypothesisGeneratedEventArgs* args) override
        {
            WriteDictationEtwEvent("Hypothesis generated on worker thread", m_DictationRecognizer);

            win::ComPtr<UnityWinRTBase::ISpeechRecognitionHypothesis> hypothesis;
            auto hr = args->get_Hypothesis(&hypothesis);
            if (!m_DictationRecognizer->VerifyAndReportHR(hr, "ERROR:  unexpected error retrieving hypothesis!"))
            {
                // no data?  bail early.
                return S_OK;
            }

            UnityWinRTBase::HSTRING text;
            hr = hypothesis->get_Text(&text);
            if (!m_DictationRecognizer->VerifyAndReportHR(hr, "ERROR:  unexpected error retrieving hypothesis text!"))
            {
                // no data?  still bail early.
                return S_OK;
            }

            DictationHypothesisBuffer resBuf = { m_DictationRecognizer, text };
            GlobalEventQueue::GetInstance().SendEvent(resBuf);
            return S_OK;
        }

        // Completed handler
        virtual HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::ISpeechContinuousRecognitionSession* sender, UnityWinRTBase::ISpeechContinuousRecognitionCompletedEventArgs* args) override
        {
            WriteDictationEtwEvent("Dictation completed on worker thread", m_DictationRecognizer);

            UnityWinRTBase::SpeechRecognitionResultStatus status;
            auto hr = args->get_Status(&status);
            if (m_DictationRecognizer->VerifyAndReportHR(hr, "ERROR:  unable to retrieve speech recognition status!"))
            {
                // don't propagate status if we don't think we got a valid one
                DictationCompletedBuffer compBuf = { m_DictationRecognizer, status };
                GlobalEventQueue::GetInstance().SendEvent(compBuf);
            }
            return S_OK;
        }

        // compile complete callback
        HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::Windows::Foundation::IAsyncOperation<UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionCompilationResult*> *asyncInfo, UnityWinRTBase::Windows::Foundation::AsyncStatus status)
        {
            WriteDictationEtwEvent("Constraint compilation completed on worker thread", m_DictationRecognizer);

            // note:  kick the "next task" thing back to the main thread unless
            // we can find a non-locking, zero-latency solution.
            m_DictationRecognizer->m_Tasks.MarkCurrentTaskAsQueuedToMainThread();
            DictationCompileComplete cc = { m_DictationRecognizer, status == UnityWinRTBase::Windows::Foundation::Completed };
            GlobalEventQueue::GetInstance().SendEvent(cc);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::IAsyncAction* sender, UnityWinRTBase::AsyncStatus status)
        {
            WriteDictationEtwEvent("Async action completed on worker thread", m_DictationRecognizer);

            m_DictationRecognizer->m_Tasks.MarkCurrentTaskAsQueuedToMainThread();
            DictationAsyncComplete ac = { m_DictationRecognizer, status };
            GlobalEventQueue::GetInstance().SendEvent(ac);
            return S_OK;
        }
    };


#define VerifyCreationHR(hr, message) \
            if (FAILED(hr)) \
            { /* We can't really throw exception here as we depend on RAII and Mono doesn't unroll the stack */ \
              /* So instead we just allocate a temp core::string as an out parameter so the caller can throw exception instead */ \
                outErrorMessage = FormatString(message, hr); \
                return; \
            }

    DictationRecognizer::DictationRecognizer(ScriptingObjectPtr managedCounterpart, ConfidenceLevel minimumConfidence, DictationTopicConstraint dictationTopicConstraint,
                                             core::string& outErrorMessage) :
        m_DictationSessionEventHandler(nullptr)
        , m_ManagedCounterpart(managedCounterpart, GCHANDLE_WEAK)
        , m_MinimumConfidence(minimumConfidence)
        , m_Tasks(*this)
        , m_Status(kSpeechSystemStopped)
        , m_IsStopping(false)
#if UNITY_EDITOR
        , m_IsQueuedForDestruction(false)
#endif
    {
        outErrorMessage.clear();

        win::ComPtr<UnityWinRTBase::IInspectable> recognizerInspectable;
        win::ComPtr<UnityWinRTBase::ISpeechRecognizer2> recognizer2;

        HRESULT hr = UnityWinRTBase::RoActivateInstance(UnityWinRTBase::HStringReference(L"Windows.Media.SpeechRecognition.SpeechRecognizer"), &recognizerInspectable);
        if (FAILED(hr))
        {
            outErrorMessage = hr == REGDB_E_CLASSNOTREG
                ? "Speech recognition is not supported on this machine."
                : FormatString("Failed to activate Windows.Media.SpeechRecognition.SpeechRecognizer (hr = 0x%X)", hr);
            return;
        }

        hr = recognizerInspectable.As(&m_Recognizer);
        VerifyCreationHR(hr, "Failed to cast IInspectable to ISpeechRecognizer (hr = 0x%X)");

        hr = m_Recognizer.As(&recognizer2);
        VerifyCreationHR(hr, "Speech recognition is not supported on this machine.");

        hr = recognizer2->get_ContinuousRecognitionSession(&m_ContinuousRecognitionSession);
        VerifyCreationHR(hr, "Failed to get continuous recognition session from ISpeechRecognizer2 (hr = 0x%X)");

        m_DictationSessionEventHandler.Attach(UNITY_NEW(DictationSessionEventHandler, kMemSpeech)(this));

        hr = recognizer2->add_HypothesisGenerated(m_DictationSessionEventHandler, &m_OnHypothesisGeneratedToken);
        VerifyCreationHR(hr, "Failed to register ISpeechRecognizer2::HypothesisGenerated callback (hr = 0x%X)");

        hr = m_ContinuousRecognitionSession->add_ResultGenerated(m_DictationSessionEventHandler, &m_OnResultGeneratedToken);
        VerifyCreationHR(hr, "Failed to register ISpeechContinuousRecognitionSession::ResultGenerated callback (hr = 0x%X)");

        hr = m_ContinuousRecognitionSession->add_Completed(m_DictationSessionEventHandler, &m_OnCompletedToken);
        VerifyCreationHR(hr, "Failed to register ISpeechContinuousRecognitionSession::Completed callback (hr = 0x%X)");

        win::ComPtr<UnityWinRTBase::ISpeechRecognitionTopicConstraintFactory> constraintFactory;
        hr = UnityWinRTBase::RoGetActivationFactory(UnityWinRTBase::HStringReference(L"Windows.Media.SpeechRecognition.SpeechRecognitionTopicConstraint"),
            __uuidof(UnityWinRTBase::ISpeechRecognitionTopicConstraintFactory), &constraintFactory);
        VerifyCreationHR(hr, "Failed to get ISpeechRecognitionTopicConstraintFactory (hr = 0x%X)");

        win::ComPtr<UnityWinRTBase::ISpeechRecognitionTopicConstraint> topicConstraint;
        hr = constraintFactory->Create(static_cast<UnityWinRTBase::SpeechRecognitionScenario>(dictationTopicConstraint), nullptr, &topicConstraint);
        VerifyCreationHR(hr, "Failed to create ISpeechRecognitionTopicConstraint (hr = 0x%X)");

        win::ComPtr<UnityWinRTBase::ISpeechRecognitionConstraint> speechRecognitionConstraint;
        hr = topicConstraint.As(&speechRecognitionConstraint);
        VerifyCreationHR(hr, "Failed to cast ISpeechRecognitionTopicConstraint to ISpeechRecognitionConstraint (hr = 0x%X)");

        win::ComPtr<UnityWinRTBase::IVector<UnityWinRTBase::ISpeechRecognitionConstraint*> > speechRecognitionConstraints;
        hr = m_Recognizer->get_Constraints(&speechRecognitionConstraints);
        VerifyCreationHR(hr, "Failed to get speech recognition constraints (hr = 0x%X)");

        hr = speechRecognitionConstraints->Append(speechRecognitionConstraint);
        VerifyCreationHR(hr, "Failed to append dictation topic constraint to ISpeechRecognizer::Constraints (hr = 0x%X)");

        hr = m_Recognizer->get_Timeouts(&m_Timeouts);
        VerifyCreationHR(hr, "Failed to read ISpeechRecognizer::Timeouts (hr = 0x%X)");

        // register our event queue handlers
        GlobalEventQueue::GetInstance().AddHandler(m_ResultEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_HypothesisEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_CompletionEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_ErrorEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_CompileCompleteDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_AsyncCompleteDelegate.SetObject(this));

        m_Tasks.Initialize();

        static UnityEventQueue::StaticFunctionEventHandler<DictationDestructionBuffer> s_DestroyEventDelegate(&DictationRecognizer::HandleEvent);
        static bool s_IsDestroyEventDelegateRegistered = false;

        if (!s_IsDestroyEventDelegateRegistered)
        {
            s_IsDestroyEventDelegateRegistered = true;
            GlobalEventQueue::GetInstance().AddHandler(&s_DestroyEventDelegate);
        }

        PhraseRecognitionSystem::GetInstance().AddDictationRecognizer(this);

        // kick off a task to compile our constraints such as they are.
        m_Tasks.AddTask(&DictationRecognizer::CompileAsync);
    }

    DictationRecognizer::~DictationRecognizer()
    {
        WriteDictationEtwEvent("Dictation recognizer deleted", this);
    }

    void DictationRecognizer::Destroy()
    {
        WriteDictationEtwEvent("Starting to destroy dictation recognizer", this);
        PhraseRecognitionSystem::GetInstance().RemoveDictationRecognizer(this);

        auto hr = m_ContinuousRecognitionSession->remove_ResultGenerated(m_OnResultGeneratedToken);
        Assert(SUCCEEDED(hr));

        hr = m_ContinuousRecognitionSession->remove_Completed(m_OnCompletedToken);
        Assert(SUCCEEDED(hr));

        win::ComPtr<UnityWinRTBase::ISpeechRecognizer2> recognizer2;
        hr = m_Recognizer.As(&recognizer2);
        Assert(SUCCEEDED(hr));

        hr = recognizer2->remove_HypothesisGenerated(m_OnHypothesisGeneratedToken);
        Assert(SUCCEEDED(hr));

        // de-register unity events
        if (nullptr != m_ResultEventDelegate.GetHandler())
        {
            GlobalEventQueue::GetInstance().RemoveHandler(&m_ResultEventDelegate);
        }
        if (nullptr != m_HypothesisEventDelegate.GetHandler())
        {
            GlobalEventQueue::GetInstance().RemoveHandler(&m_HypothesisEventDelegate);
        }
        if (nullptr != m_CompletionEventDelegate.GetHandler())
        {
            GlobalEventQueue::GetInstance().RemoveHandler(&m_CompletionEventDelegate);
        }
        if (nullptr != m_ErrorEventDelegate.GetHandler())
        {
            GlobalEventQueue::GetInstance().RemoveHandler(&m_ErrorEventDelegate);
        }
        if (nullptr != m_CompileCompleteDelegate.GetHandler())
        {
            GlobalEventQueue::GetInstance().RemoveHandler(&m_CompileCompleteDelegate);
        }
        if (nullptr != m_AsyncCompleteDelegate.GetHandler())
        {
            GlobalEventQueue::GetInstance().RemoveHandler(&m_AsyncCompleteDelegate);
        }

        if (m_Status == kSpeechSystemRunning)
        {
            m_Tasks.ShutdownWithCleanup(&DictationRecognizer::StopAsync, false);
        }
        else
        {
            m_Tasks.Shutdown();
        }

        m_ManagedCounterpart.ReleaseAndClear();

        m_Recognizer = nullptr;
        m_DictationSessionEventHandler = nullptr;
        Release();

        WriteDictationEtwEvent("Dictation recognizer destroyed", this);
    }

    void DictationRecognizer::DestroyThreaded()
    {
        WriteDictationEtwEvent("Queuing up dictation recognizer destruction", this);
        ScriptingGCHandle strongGCHandle;

        if (m_ManagedCounterpart.HasTarget())
        {
            ScriptingObjectPtr managedObject = m_ManagedCounterpart.Resolve();
            strongGCHandle.AcquireStrong(managedObject);
        }

#if UNITY_EDITOR
        m_IsQueuedForDestruction = true;
#endif
        DictationDestructionBuffer buffer = { this, ScriptingGCHandle::ToScriptingBackendNativeGCHandle(strongGCHandle) };
        GlobalEventQueue::GetInstance().SendEvent(buffer);
    }

    float DictationRecognizer::GetAutoSilenceTimeoutSeconds()
    {
        float retVal = 0.0f;

        if (m_Status == kSpeechSystemFailed)
            return retVal;

        UnityWinRTBase::Windows::Foundation::TimeSpan val;
        auto hr = m_ContinuousRecognitionSession->get_AutoStopSilenceTimeout(&val);
        if (VerifyAndReportHR(hr, "WARNING:  unable to read auto stop silence timeout from continuous recognition session!"))
        {
            retVal = TimeSpanToSeconds(val);
        }

        return retVal;
    }

    void DictationRecognizer::SetAutoSilenceTimeoutSeconds(float newVal)
    {
        if (m_Status == kSpeechSystemFailed)
            return;

        UnityWinRTBase::Windows::Foundation::TimeSpan val;
        SecondsToTimeSpan(val, newVal);
        auto hr = m_ContinuousRecognitionSession->put_AutoStopSilenceTimeout(val);
        VerifyAndReportHR(hr, "WARNING:  unable to write auto stop silence timeout to continuous recognition session!");
    }

    float DictationRecognizer::GetInitialSilenceTimeoutSeconds()
    {
        float retVal = 0.0f;

        if (m_Status == kSpeechSystemFailed)
            return retVal;

        UnityWinRTBase::Windows::Foundation::TimeSpan val;
        auto hr = m_Timeouts->get_InitialSilenceTimeout(&val);
        if (VerifyAndReportHR(hr, "WARNING:  unable to read initial silence timeout!"))
        {
            retVal = TimeSpanToSeconds(val);
        }

        return retVal;
    }

    void DictationRecognizer::SetInitialSilenceTimeoutSeconds(float newVal)
    {
        if (m_Status == kSpeechSystemFailed)
            return;

        UnityWinRTBase::Windows::Foundation::TimeSpan val;
        SecondsToTimeSpan(val, newVal);
        auto hr = m_Timeouts->put_InitialSilenceTimeout(val);
        VerifyAndReportHR(hr, "WARNING:  unable to write auto stop silence timeout to continuous recognition session!");
    }

    void DictationRecognizer::HandleEvent(const DictationResultBuffer &event)
    {
        if (this != event.recognizer)
            return;

        WriteDictationEtwEvent("Dictation result generated on main thread", this);

        // nb:  0 is high confidence with lower confidences having higher values
        if (event.confidence > m_MinimumConfidence)
            return;

        if (!m_ManagedCounterpart.HasTarget())
            return;

        uint32_t textLength;
        const wchar_t* text = UnityWinRTBase::WindowsGetStringRawBuffer(event.text, &textLength);

        ScriptingObjectPtr managedObject = m_ManagedCounterpart.Resolve();
        ScriptingInvocation invocation(managedObject, GetCoreScriptingClasses().dictationRecognizer_InvokeResultGeneratedEvent);
        invocation.AddString(scripting_string_new(text));
        invocation.AddInt(event.confidence);
        invocation.Invoke();
    }

    void DictationRecognizer::HandleEvent(const DictationHypothesisBuffer &event)
    {
        if (this != event.recognizer)
            return;

        WriteDictationEtwEvent("Dictation hypothesis generated on main thread", this);

        if (!m_ManagedCounterpart.HasTarget())
            return;

        uint32_t textLength;
        const wchar_t* text = UnityWinRTBase::WindowsGetStringRawBuffer(event.text, &textLength);

        ScriptingObjectPtr managedObject = m_ManagedCounterpart.Resolve();
        ScriptingInvocation invocation(managedObject, GetCoreScriptingClasses().dictationRecognizer_InvokeHypothesisGeneratedEvent);
        invocation.AddString(scripting_string_new(text));
        invocation.Invoke();
    }

    void DictationRecognizer::HandleEvent(const DictationCompletedBuffer &event)
    {
        if (this != event.recognizer)
            return;

        WriteDictationEtwEvent("Dictation completed on main thread", this);

        if (!m_ManagedCounterpart.HasTarget())
            return;

        DictationCompletionCause cause = kDCCauseUnknownError;
        switch (event.cause)
        {
            // translate to our enum.  could reflect these instead and use
            // across multiple vectors.
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_Success:              cause = kDCCauseComplete; break;
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_AudioQualityFailure:  cause = kDCCauseAudioQualityFailure; break;
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_UserCanceled:         cause = kDCCauseCanceled; break;
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_TimeoutExceeded:      cause = kDCCauseTimeout; break;
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_NetworkFailure:       cause = kDCCauseNetworkFailure; break;
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_MicrophoneUnavailable: cause = kDCCauseMicrophoneUnavailable; break;

            // shouldn't get these or they're ambiguous
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_PauseLimitExceeded:
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_TopicLanguageNotSupported:
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_GrammarLanguageMismatch:
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_GrammarCompilationFailure:
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_Unknown:
            default:
                cause = kDCCauseUnknownError;
                break;
        }

        switch (event.cause)
        {
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_Success:
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_UserCanceled:
            case UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus_TimeoutExceeded:
                m_Status = kSpeechSystemStopped;
                break;

            default:
                m_Status = kSpeechSystemFailed;
                break;
        }

        ScriptingObjectPtr managedObject = m_ManagedCounterpart.Resolve();
        ScriptingInvocation invocation(managedObject, GetCoreScriptingClasses().dictationRecognizer_InvokeCompletedEvent);
        invocation.AddInt(cause);
        invocation.Invoke();
    }

    void DictationRecognizer::HandleEvent(const DictationErrorBuffer &event)
    {
        if (this != event.recognizer)
            return;

        WriteDictationEtwEvent("Dictation error arrived on main thread", this);

        if (!m_ManagedCounterpart.HasTarget())
            return;

        m_Status = kSpeechSystemFailed;

        ScriptingObjectPtr managedObject = m_ManagedCounterpart.Resolve();
        ScriptingInvocation invocation(managedObject, GetCoreScriptingClasses().dictationRecognizer_InvokeErrorEvent);
        invocation.AddString(event.errorMessage);
        invocation.AddInt(event.hresult);
        invocation.Invoke();
    }

    void DictationRecognizer::HandleEvent(const DictationCompileComplete &event)
    {
        if (event.recognizer != this)
            return;

        if (!event.compileSuccessful)
        {
            // if the compile failed there's not a lot we can do so post an error
            m_Status = kSpeechSystemFailed;
            ReportDictationError("ERROR:  DictationRecognizer failed compilation step.  Re-create the object and try again.");
            return;
        }

        WriteDictationEtwEvent("Constraint compilation completed on main thread", this);
        m_Tasks.ProcessNextTask();
    }

    void DictationRecognizer::HandleEvent(const DictationAsyncComplete &event)
    {
        if (event.recognizer != this)
            return;

        if (m_Tasks.GetCurrentOperation() == &DictationRecognizer::StartAsync)
        {
            if (event.status != UnityWinRTBase::Windows::Foundation::Completed)
            {
                WriteDictationEtwEvent("StartAsync failed on main thread", this);
                m_Status = kSpeechSystemFailed;
            }
            else
            {
                WriteDictationEtwEvent("StartAsync completed on main thread", this);
                m_Status = kSpeechSystemRunning;
            }
        }
        else if (m_Tasks.GetCurrentOperation() == &DictationRecognizer::StopAsync)
        {
            if (event.status != UnityWinRTBase::Windows::Foundation::Completed)
            {
                WriteDictationEtwEvent("StopAsync failed on main thread", this);
                m_Status = kSpeechSystemFailed;
            }
            else
            {
                WriteDictationEtwEvent("StopAsync completed on main thread", this);
                m_Status = kSpeechSystemStopped;
            }

            m_IsStopping = false;
        }

        m_Tasks.ProcessNextTask();
    }

    void DictationRecognizer::HandleEvent(const DictationDestructionBuffer& event)
    {
        event.recognizer->Destroy();
    }

#if UNITY_EDITOR

    void DictationRecognizer::OnBeforeDomainUnload()
    {
        m_ManagedCounterpart.ReleaseAndClear();
    }

#endif

    void DictationRecognizer::StartRecognition(ScriptingExceptionPtr* exception)
    {
        if (m_Status == kSpeechSystemRunning)
        {
            WarningString("Warning: DictationRecognizer.Start() was called when DictationRecognizer was already running.");
            return;
        }

        if (PhraseRecognitionSystem::GetInstance().GetStatus() == kSpeechSystemRunning)
        {
            *exception = Scripting::CreateUnityException("Cannot start dictation recognition session while PhraseRecognitionSystem is running.");
            return;
        }

        const auto& dictationRecognizers = PhraseRecognitionSystem::GetInstance().GetDictationRecognizers();

        for (size_t i = 0; i < dictationRecognizers.size(); i++)
        {
#if UNITY_EDITOR
            if (dictationRecognizers[i]->IsQueuedForDestruction())
                continue;
#endif

            if (dictationRecognizers[i]->GetStatus() == kSpeechSystemRunning)
            {
                *exception = Scripting::CreateUnityException("Cannot start dictation recognition session while another dictation recognition session is in progress!");
                return;
            }
        }

        if (m_Status != kSpeechSystemFailed)
            m_Tasks.AddTask(&DictationRecognizer::StartAsync);
    }

    void DictationRecognizer::StopRecognition()
    {
        // nb:  if we aren't recognizing the MS APIs throw an error code that would
        // otherwise clog up our error event handler.  assuming the recognizing flag
        // is set correctly this should prevent that.
        if (m_Status == kSpeechSystemRunning)
        {
            m_Tasks.AddTask(&DictationRecognizer::StopAsync);
        }
        else
        {
            WarningString("Warning: DictationRecognizer.Stop() was called when DictationRecognizer was not running.");
        }
    }

    void DictationRecognizer::StartAsync()
    {
        // We may arrive here if the task was queued up while the speech system was not running and was started in the meantime.
        if (m_Status != kSpeechSystemStopped)
        {
            m_Tasks.ProcessNextTask();
            return;
        }

        WriteDictationEtwEvent("Starting StartAsync", this);

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        HRESULT SPERR_SPEECH_PRIVACY_POLICY_NOT_ACCEPTED = 0x80045509;
        auto hr = m_ContinuousRecognitionSession->StartAsync(&asyncAction);

        if (hr == SPERR_SPEECH_PRIVACY_POLICY_NOT_ACCEPTED)
        {
            VerifyAndReportHR(hr, "ERROR:  Dictation support is not enabled on this device (see 'Get to know me' in Settings > Privacy > Speech, inking, & typing)");
        }
        else if (VerifyAndReportHR(hr, "ERROR:  StartAsync on the continuous recognition session failed unexpectedly!"))
        {
            hr = asyncAction->put_Completed((UnityWinRTBase::IAsyncActionCompletedHandler*)m_DictationSessionEventHandler);

            if (VerifyAndReportHR(hr, "WARNING:  unexpected failure setting callback from StartAsync"))
                return;
        }
        m_Tasks.ProcessNextTask();
    }

    void DictationRecognizer::StopAsync()
    {
        // We may arrive here if the task was queued up while the speech system was still running and was stopped in the meantime.
        if (m_Status != kSpeechSystemRunning || m_IsStopping)
        {
            m_Tasks.ProcessNextTask();
            return;
        }

        m_IsStopping = true;
        WriteDictationEtwEvent("Starting StopAsync", this);

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        auto hr = m_ContinuousRecognitionSession->StopAsync(&asyncAction);
        if (VerifyAndReportHR(hr, "ERROR:  StopAsync on the continuous recognition session failed unexpectedly!"))
        {
            hr = asyncAction->put_Completed((UnityWinRTBase::IAsyncActionCompletedHandler*)m_DictationSessionEventHandler);

            if (!VerifyAndReportHR(hr, "WARNING:  unexpected failure setting callback from StopAsync"))
                m_Tasks.ProcessNextTask();
        }
        else
        {
            m_Tasks.ProcessNextTask();
        }
    }

    void DictationRecognizer::CompileAsync()
    {
        // caller should be checking our state
        Assert(m_Status != kSpeechSystemFailed);
        WriteDictationEtwEvent("Starting CompileAsync", this);

        win::ComPtr<UnityWinRTBase::IAsyncOperation<UnityWinRTBase::SpeechRecognitionCompilationResult*> > asyncOperation;
        auto hr = m_Recognizer->CompileConstraintsAsync(&asyncOperation);
        if (VerifyAndReportHR(hr, "ERROR:  CompileConstraintsAsync failed unexpectedly!"))
        {
            hr = asyncOperation->put_Completed(m_DictationSessionEventHandler);

            if (!VerifyAndReportHR(hr, "ERROR:  unexpected failure setting callback from CompileAsync"))
                m_Tasks.ProcessNextTask();
        }
        else
        {
            m_Tasks.ProcessNextTask();
        }
    }
}

#endif // PLATFORM_WIN
