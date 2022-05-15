#include "UnityPrefix.h"
#include "Runtime/Core/Callbacks/GlobalCallbacks.h"
#include "Runtime/Interfaces/IVRDevice.h"
#include "Runtime/Misc/Player.h"
#include "Runtime/Scripting/CommonScriptingClasses.h"
#include "Runtime/Mono/MonoManager.h"
#include "PhraseRecognitionSystem.h"

#if PLATFORM_WIN

#include "External/Windows10/src/ComHelpers.h"
#include "DictationRecognizer.h"
#include "PhraseRecognizer.h"
#include "PlatformDependent/Win/etw/ETW.h"
#include "Runtime/Utilities/DiagnosticSwitch.h"
#include "Runtime/Threads/CurrentThread.h"

DEFINE_DIAGNOSTIC_SWITCH(bool, gDiagRemoteSpeechEnabled, "Allows relaying of speech during Holographic Emulation", false);

const UINT32 kMaxAlternates = 32;

namespace UnityWinRTBase
{
    using namespace UnityWinRTBase::Windows::Foundation;
    using namespace UnityWinRTBase::Windows::Foundation::Collections;
    using namespace UnityWinRTBase::Windows::Media::SpeechRecognition;
}

inline bool IsRemoteSpeechActive()
{
    return GetIXRRemoteSpeech() && GetIXRRemoteSpeech()->IsActive();
}

namespace Unity
{
    struct SpeechRecognitionAsyncActionBuffer
    {
        int version;
        HRESULT hr;
    };

    struct SpeechCompilationCompletedBuffer
    {
        int version;
        UnityWinRTBase::SpeechRecognitionResultStatus resultStatus;
    };

    struct SpeechRecognitionCompletedBuffer
    {
        int version;
        UnityWinRTBase::SpeechRecognitionResultStatus resultStatus;
    };

    struct SpeechRecognitionResultBuffer
    {
        int version;
        UnityWinRTBase::ISpeechRecognitionResult* result;

        void Destroy()
        {
            result->Release();
        }
    };
}

REGISTER_EVENT_ID(0x8F2BBCF6D0D3452DULL, 0xAB51906569CAA8D2ULL, Unity::SpeechRecognitionAsyncActionBuffer)
REGISTER_EVENT_ID(0x40AEBEDC7C9744B2ULL, 0xB42AB8234303D476ULL, Unity::SpeechCompilationCompletedBuffer)
REGISTER_EVENT_ID(0x8AAA6C6C407D4411ULL, 0x94818557AC53A8B8ULL, Unity::SpeechRecognitionCompletedBuffer)
REGISTER_EVENT_ID_WITH_CLEANUP(0x0A6EB1800C084AF5ULL, 0xAEA4DC847B84F919ULL, Unity::SpeechRecognitionResultBuffer)

namespace Unity
{
    static inline void WriteSpeechEtwEvent(const char* msg)
    {
#if ENABLE_EVENT_TRACING_FOR_WINDOWS
        const char kPhraseRecognitionSystemEtwCategory[] = "PhraseRecognitionSystem";
        EventWriteRuntimeEvent(kPhraseRecognitionSystemEtwCategory, msg);
#endif
    }

    static inline SpeechError SpeechRecognitionResultStatusToSpeechError(UnityWinRTBase::SpeechRecognitionResultStatus status)
    {
        switch (status)
        {
            case UnityWinRTBase::SpeechRecognitionResultStatus_Success:
                return kNoError;

            case UnityWinRTBase::SpeechRecognitionResultStatus_TopicLanguageNotSupported:
                return kTopicLanguageNotSupported;

            case UnityWinRTBase::SpeechRecognitionResultStatus_GrammarLanguageMismatch:
                return kGrammarLanguageMismatch;

            case UnityWinRTBase::SpeechRecognitionResultStatus_GrammarCompilationFailure:
                return kGrammarCompilationFailure;

            case UnityWinRTBase::SpeechRecognitionResultStatus_AudioQualityFailure:
                return kAudioQualityFailure;

            case UnityWinRTBase::SpeechRecognitionResultStatus_TimeoutExceeded:
                return kTimeoutExceeded;

            case UnityWinRTBase::SpeechRecognitionResultStatus_NetworkFailure:
                return kNetworkFailure;

            case UnityWinRTBase::SpeechRecognitionResultStatus_MicrophoneUnavailable:
                return kMicrophoneUnavailable;
        }

        return kUnknownError;
    }

#define VerifyHR(hr, failureMessage) \
    do \
    { \
        if (FAILED(hr)) \
        { \
            AssertFormatMsg(false, failureMessage, hr); \
            WriteSpeechEtwEvent(Format("Operation failed: %s", Format(failureMessage, hr).c_str()).c_str()); \
            DispatchSpeechError(kUnknownError); \
            ChangeSystemStatus(kSpeechSystemFailed); \
            return; \
        } \
    } \
    while (false)

#define VerifyAsyncHR(hr, failureMessage) \
    do \
    { \
        if (FAILED(hr)) \
        { \
            AssertFormatMsg(false, failureMessage, hr); \
            WriteSpeechEtwEvent(Format("Starting async operation failed: %s", Format(failureMessage, hr).c_str()).c_str()); \
            DispatchSpeechError(kUnknownError); \
            ChangeSystemStatus(kSpeechSystemFailed); \
            m_Tasks.ProcessNextTask(); \
            return; \
        } \
    } \
    while (false)

    // Our current speech recognition implementation depends on SpeechRecognizer implementing ISpeechRecognizer2 interface
    bool PhraseRecognitionSystem::IsSupported()
    {
        static bool s_HasCheckedIfSupported = false;
        static bool s_IsSupported = false;

        if (!s_HasCheckedIfSupported)
        {
            s_HasCheckedIfSupported = true;

            win::ComPtr<UnityWinRTBase::IInspectable> recognizerInspectable;
            win::ComPtr<UnityWinRTBase::ISpeechRecognizer2> recognizer2;

            HRESULT hr = UnityWinRTBase::RoActivateInstance(UnityWinRTBase::HStringReference(L"Windows.Media.SpeechRecognition.SpeechRecognizer"), &recognizerInspectable);
            if (SUCCEEDED(hr))
            {
                hr = recognizerInspectable.As(&recognizer2);
                s_IsSupported = SUCCEEDED(hr);
            }
        }

        return s_IsSupported;
    }

    PhraseRecognitionSystem& PhraseRecognitionSystem::GetInstance()
    {
        static PhraseRecognitionSystem* s_System = UNITY_NEW(PhraseRecognitionSystem, kMemSpeech);
        return *s_System;
    }

    PhraseRecognitionSystem::PhraseRecognitionSystem() :
        m_HasUncompiledConstraints(false),
        m_PendingRestart(false),
        m_PhraseRecognizers(kMemSpeech),
        m_DictationRecognizers(kMemSpeech),
        m_EnabledPhraseRecognizerCount(0),
        m_FakePhraseRecognizer(nullptr),
        m_Status(kSpeechSystemStopped),
        m_Tasks(*this),
        m_Version(0),
        m_ShouldRestartOnResuming(false)
    {
        m_CompletedToken.value = 0;
        m_ResultGeneratedToken.value = 0;

        RegisterGlobalCallbacks();
    }

    PhraseRecognitionSystem::~PhraseRecognitionSystem()
    {
        UnregisterGlobalCallbacks();
    }

    void PhraseRecognitionSystem::ReportSystemError(UnityWinRTBase::SpeechRecognitionResultStatus status)
    {
#if ENABLE_EVENT_TRACING_FOR_WINDOWS
        core::string etwMessage = FormatString("SpeechRecognitionError: (SpeechRecognitionResultStatus)%d", (int)status);
        WriteSpeechEtwEvent(etwMessage.c_str());
#endif

        SpeechError error = SpeechRecognitionResultStatusToSpeechError(status);
        DispatchSpeechError(error);
        ChangeSystemStatus(kSpeechSystemFailed);
    }

    void PhraseRecognitionSystem::DispatchSpeechError(SpeechError error)
    {
        Assert(CurrentThread::IsMainThread());

        ScriptingInvocation scriptingInvocation(GetCoreScriptingClasses().phraseRecognitionSystem_InvokeErrorEvent);
        scriptingInvocation.AddInt(error);
        scriptingInvocation.Invoke();
    }

    void PhraseRecognitionSystem::ChangeSystemStatus(SpeechSystemStatus newStatus)
    {
#if ENABLE_EVENT_TRACING_FOR_WINDOWS
        const char* etwMessage = "";
        switch (newStatus)
        {
            case kSpeechSystemStopped:
                etwMessage = "Status changed: kSpeechSystemStopped";
                break;

            case kSpeechSystemRunning:
                etwMessage = "Status changed: kSpeechSystemRunning";
                break;

            case kSpeechSystemFailed:
                etwMessage = "Status changed: kSpeechSystemFailed";
                break;
        }
        WriteSpeechEtwEvent(etwMessage);
#endif

        Assert(CurrentThread::IsMainThread());
        m_Status = newStatus;

        if (GetMonoManagerPtr() == nullptr)
            return;

        ScriptingInvocation scriptingInvocation(GetCoreScriptingClasses().phraseRecognitionSystem_InvokeStatusChangedEvent);
        scriptingInvocation.AddInt(newStatus);
        scriptingInvocation.Invoke();
    }

    void PhraseRecognitionSystem::Restart(ScriptingExceptionPtr* exception)
    {
        if (m_Status == kSpeechSystemRunning)
            return;

        if (GetIXRRemoteSpeech())
        {
            GetIXRRemoteSpeech()->Restart();
        }

        if (!IsSupported())
        {
            *exception = Scripting::CreateUnityException("Speech recognition is not supported on this machine.");
            return;
        }

        for (size_t i = 0; i < m_DictationRecognizers.size(); i++)
        {
            if (m_DictationRecognizers[i]->GetStatus() == kSpeechSystemRunning)
            {
                *exception = Scripting::CreateUnityException("Cannot start speech recognition system while a dictation recognition session is in progress!");
                return;
            }
        }

        if (m_Status == kSpeechSystemFailed)
            Shutdown();

        DebugAssert(m_Status == kSpeechSystemStopped);
        WriteSpeechEtwEvent("Initialization starting");

        win::ComPtr<UnityWinRTBase::IInspectable> recognizerInspectable;
        win::ComPtr<UnityWinRTBase::ISpeechRecognizer2> recognizer2;

        HRESULT hr = UnityWinRTBase::RoActivateInstance(UnityWinRTBase::HStringReference(L"Windows.Media.SpeechRecognition.SpeechRecognizer"), &recognizerInspectable);
        VerifyHR(hr, "Failed to create an instance of Windows.Media.SpeechRecognition.SpeechRecognizer (hr = 0x%X)");

        hr = recognizerInspectable.As(&m_Recognizer);
        VerifyHR(hr, "Failed to cast IInspectable to ISpeechRecognizer (hr = 0x%X)");

        hr = m_Recognizer.As(&recognizer2);
        VerifyHR(hr, "Failed to cast IInspectable to ISpeechRecognizer2 (hr = 0x%X)");

        hr = recognizer2->get_ContinuousRecognitionSession(&m_ContinuousRecognitionSession);
        VerifyHR(hr, "Failed to get ContinuousRecognitionSession from ISpeechRecognizer2 (hr = 0x%X)");

        m_Version++;
        m_EventHandler.Attach(UNITY_NEW(PhraseRecognitionSystemEventHandler, kMemSpeech)(this, m_Version));
        m_Tasks.Initialize();

        hr = m_ContinuousRecognitionSession->add_Completed(m_EventHandler, &m_CompletedToken);
        VerifyHR(hr, "Failed to subscribe to ISpeechContinuousRecognitionSession::Completed event (hr = 0x%X)");

        hr = m_ContinuousRecognitionSession->add_ResultGenerated(m_EventHandler, &m_ResultGeneratedToken);
        VerifyHR(hr, "Failed to subscribe to ISpeechContinuousRecognitionSession::Completed event (hr = 0x%X)");

        m_HasUncompiledConstraints = m_PhraseRecognizers.size() > 0;

        GlobalEventQueue::GetInstance().AddHandler(m_AsyncActionEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_CompilationCompletedEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_RecognitionCompletedEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_RecognitionResultEventDelegate.SetObject(this));

        ChangeSystemStatus(kSpeechSystemRunning);
        m_PendingRestart = true;

        core::string errorMessage;
        const wchar_t* const kFakeRecognizerKeyword = L"qrtcxzcdddddjksfmjmadlsfbnsdjkclmfasxnemkzwlaxfngvzmklsdcnvmk";
        m_FakePhraseRecognizer = PhraseRecognizer::Create(SCRIPTING_NULL, &kFakeRecognizerKeyword, 1, kConfidenceLevel_High, errorMessage);

        if (m_FakePhraseRecognizer == nullptr)
        {
            AssertFormatMsg(errorMessage.empty(), "Failed creating FakePhraseRecognizer: %s", errorMessage.c_str());
            AssertFormatMsg(m_FakePhraseRecognizer != nullptr, "Failed creating FakePhraseRecognizer");
            DispatchSpeechError(kUnknownError);
            ChangeSystemStatus(kSpeechSystemFailed);
            return;
        }

        m_FakePhraseRecognizer->Start(exception);
        if (*exception != SCRIPTING_NULL)
            return;

        WriteSpeechEtwEvent("Initialization complete");
    }

    void PhraseRecognitionSystem::Shutdown()
    {
        WriteSpeechEtwEvent("Shutdown starting");

        if (GetIXRRemoteSpeech())
            GetIXRRemoteSpeech()->Shutdown();

        if (m_RecognitionResultEventDelegate.GetHandler() != nullptr)
            GlobalEventQueue::GetInstance().RemoveHandler(&m_RecognitionResultEventDelegate);

        if (m_RecognitionCompletedEventDelegate.GetHandler() != nullptr)
            GlobalEventQueue::GetInstance().RemoveHandler(&m_RecognitionCompletedEventDelegate);

        if (m_CompilationCompletedEventDelegate.GetHandler() != nullptr)
            GlobalEventQueue::GetInstance().RemoveHandler(&m_CompilationCompletedEventDelegate);

        if (m_AsyncActionEventDelegate.GetHandler() != nullptr)
            GlobalEventQueue::GetInstance().RemoveHandler(&m_AsyncActionEventDelegate);

        if (m_ContinuousRecognitionSession != nullptr)
        {
            if (m_ResultGeneratedToken.value != 0)
            {
                m_ContinuousRecognitionSession->remove_ResultGenerated(m_ResultGeneratedToken);
                m_ResultGeneratedToken.value = 0;
            }

            if (m_CompletedToken.value != 0)
            {
                m_ContinuousRecognitionSession->remove_Completed(m_CompletedToken);
                m_CompletedToken.value = 0;
            }

            if (!m_PendingRestart)
            {
                m_Tasks.ShutdownWithCleanup(&PhraseRecognitionSystem::StopAsync, false);
            }
            else
            {
                m_Tasks.Shutdown();
            }
        }

        m_EventHandler = nullptr;
        m_ContinuousRecognitionSession = nullptr;
        m_Recognizer = nullptr;
        ChangeSystemStatus(kSpeechSystemStopped);

        if (m_FakePhraseRecognizer != nullptr)
        {
            m_FakePhraseRecognizer->Destroy();
            m_FakePhraseRecognizer = nullptr;
        }

        WriteSpeechEtwEvent("Shutdown completed");
    }

    void PhraseRecognitionSystem::AddPhraseRecognizer(PhraseRecognizer* phraseRecognizer)
    {
        m_PhraseRecognizers.push_back(phraseRecognizer);

        if (m_Status != kSpeechSystemRunning)
            return;


        if (IsRemoteSpeechActive())
            return;

        m_HasUncompiledConstraints = true;

        if (!m_Tasks.HasPendingTasks())
        {
            if (m_EnabledPhraseRecognizerCount > 0 && !m_PendingRestart)
            {
                m_Tasks.AddTask(&PhraseRecognitionSystem::PauseAsync);
            }
            else
            {
                m_Tasks.AddTask(&PhraseRecognitionSystem::CompileAsync);
            }
        }
    }

    void PhraseRecognitionSystem::RemovePhraseRecognizer(PhraseRecognizer* phraseRecognizer)
    {
        for (size_t i = 0; i < m_PhraseRecognizers.size(); i++)
        {
            if (m_PhraseRecognizers[i] == phraseRecognizer)
            {
                m_PhraseRecognizers[i] = m_PhraseRecognizers[m_PhraseRecognizers.size() - 1];
                m_PhraseRecognizers.pop_back();
                break;
            }
        }

        if (m_FakePhraseRecognizer == phraseRecognizer)
            m_FakePhraseRecognizer = nullptr;

        if (m_Status != kSpeechSystemRunning)
            return;

        if (IsRemoteSpeechActive())
            return;

        m_HasUncompiledConstraints = true;

        if (!m_Tasks.HasPendingTasks())
        {
            if (m_EnabledPhraseRecognizerCount > 0 && !m_PendingRestart)
            {
                m_Tasks.AddTask(&PhraseRecognitionSystem::PauseAsync);
            }
            else
            {
                m_Tasks.AddTask(&PhraseRecognitionSystem::CompileAsync);
            }
        }
    }

    void PhraseRecognitionSystem::AddDictationRecognizer(DictationRecognizer* dictationRecognizer)
    {
        m_DictationRecognizers.push_back(dictationRecognizer);
    }

    void PhraseRecognitionSystem::RemoveDictationRecognizer(DictationRecognizer* dictationRecognizer)
    {
        for (size_t i = 0; i < m_DictationRecognizers.size(); i++)
        {
            if (m_DictationRecognizers[i] == dictationRecognizer)
            {
                m_DictationRecognizers[i] = m_DictationRecognizers[m_DictationRecognizers.size() - 1];
                m_DictationRecognizers.pop_back();
                break;
            }
        }
    }

    void PhraseRecognitionSystem::IncrementEnabledRecognizerCount(ScriptingExceptionPtr* exception)
    {
        if (m_Status == kSpeechSystemStopped)
        {
            Restart(exception);
            if (*exception != SCRIPTING_NULL)
                return;
        }

        m_EnabledPhraseRecognizerCount++;

        if (m_Status != kSpeechSystemRunning)
            return;

        if (IsRemoteSpeechActive())
        {
            GetIXRRemoteSpeech()->UpdateState();
            return;
        }

        if (!m_Tasks.HasPendingTasks() && m_EnabledPhraseRecognizerCount == 1)
        {
            if (m_PendingRestart)
            {
                m_Tasks.AddTask(&PhraseRecognitionSystem::StartAsync);
            }
            else
            {
                m_Tasks.AddTask(&PhraseRecognitionSystem::Resume);
            }
        }
    }

    void PhraseRecognitionSystem::DecrementEnabledRecognizerCount()
    {
        m_EnabledPhraseRecognizerCount--;

        if (m_Status != kSpeechSystemRunning)
            return;

        if (IsRemoteSpeechActive())
        {
            GetIXRRemoteSpeech()->UpdateState();
            return;
        }

        if (!m_Tasks.HasPendingTasks() && m_EnabledPhraseRecognizerCount == 0 && !m_PendingRestart)
            m_Tasks.AddTask(&PhraseRecognitionSystem::StopAsync);
    }

    void PhraseRecognitionSystem::StartAsync()
    {
        if (m_Status != kSpeechSystemRunning)
        {
            m_Tasks.ProcessNextTask();
            return;
        }

        WriteSpeechEtwEvent("StartAsync starting");

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        HRESULT hr = m_ContinuousRecognitionSession->StartAsync(&asyncAction);
        VerifyAsyncHR(hr, "Failed to start IContinuousRecognitionSession (hr = 0x%X)");

        hr = asyncAction->put_Completed(m_EventHandler);
        VerifyAsyncHR(hr, "Failed to register for IContinuousRecognitionSession::StartAsync completion (hr = 0x%X)");
    }

    void PhraseRecognitionSystem::CompileAsync()
    {
        if (m_Status != kSpeechSystemRunning)
        {
            m_Tasks.ProcessNextTask();
            return;
        }

        WriteSpeechEtwEvent("CompileAsync starting");

        win::ComPtr<UnityWinRTBase::IVector<UnityWinRTBase::ISpeechRecognitionConstraint*> > constraints;
        HRESULT hr = m_Recognizer->get_Constraints(&constraints);
        VerifyAsyncHR(hr, "Failed to get Constaints from ISpeechRecognizer2 (hr = 0x%X)");

        constraints->Clear();

        for (auto it = m_PhraseRecognizers.begin(); it != m_PhraseRecognizers.end(); it++)
        {
            hr = constraints->Append((*it)->GetConstraint());
            VerifyAsyncHR(hr, "Failed to add constraint to ISpeechRecognizer::Constaints (hr = 0x%X)");
        }

        m_HasUncompiledConstraints = false;

        win::ComPtr<UnityWinRTBase::IAsyncOperation<UnityWinRTBase::SpeechRecognitionCompilationResult*> > asyncOperation;
        hr = m_Recognizer->CompileConstraintsAsync(&asyncOperation);
        VerifyAsyncHR(hr, "Failed to start speech constraint compilation (hr = 0x%X)");

        hr = asyncOperation->put_Completed(m_EventHandler);
        VerifyAsyncHR(hr, "Failed to register for notification of speech constraint compilation completion (hr = 0x%X)");
    }

    void PhraseRecognitionSystem::StopAsync()
    {
        Assert(m_ContinuousRecognitionSession != nullptr);

        WriteSpeechEtwEvent("StopAsync starting");

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        HRESULT hr = m_ContinuousRecognitionSession->StopAsync(&asyncAction);
        VerifyAsyncHR(hr, "Failed to start stopping speech recognizer (hr = 0x%X)");

        hr = asyncAction->put_Completed(m_EventHandler);
        VerifyAsyncHR(hr, "Failed to register for speech recognizer stopping completion (hr = 0x%X)");
    }

    void PhraseRecognitionSystem::PauseAsync()
    {
        if (m_Status != kSpeechSystemRunning || m_PendingRestart)
        {
            m_Tasks.ProcessNextTask();
            return;
        }

        WriteSpeechEtwEvent("PauseAsync starting");

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        HRESULT hr = m_ContinuousRecognitionSession->PauseAsync(&asyncAction);
        VerifyAsyncHR(hr, "Failed to pause IContinuousRecognitionSession (hr = 0x%X)");

        hr = asyncAction->put_Completed(m_EventHandler);
        VerifyAsyncHR(hr, "Failed to register for IContinuousRecognitionSession::PauseAsync completion (hr = 0x%X)");
    }

    void PhraseRecognitionSystem::Resume()
    {
        if (m_Status == kSpeechSystemRunning)
        {
            WriteSpeechEtwEvent("Resume");
            HRESULT hr = m_ContinuousRecognitionSession->Resume();
            VerifyHR(hr, "Failed to resume IContinuousRecognitionSession (hr = 0x%X)");
        }

        m_Tasks.ProcessNextTask();
    }

    void PhraseRecognitionSystem::OnAsyncActionCompleted(HRESULT hr, int version)
    {
        WriteSpeechEtwEvent("AsyncAction completed on worker thread");

        if (m_Version == version)
            m_Tasks.MarkCurrentTaskAsQueuedToMainThread();

        SpeechRecognitionAsyncActionBuffer buffer = { version, hr };
        GlobalEventQueue::GetInstance().SendEvent(buffer);
    }

    void PhraseRecognitionSystem::OnCompilationCompleted(UnityWinRTBase::SpeechRecognitionResultStatus status, int version)
    {
        WriteSpeechEtwEvent("Compilation completed on worker thread");

        if (m_Version == version)
            m_Tasks.MarkCurrentTaskAsQueuedToMainThread();

        SpeechCompilationCompletedBuffer buffer = { version, status };
        GlobalEventQueue::GetInstance().SendEvent(buffer);
    }

    void PhraseRecognitionSystem::OnRecognitionCompleted(UnityWinRTBase::SpeechRecognitionResultStatus status, int version)
    {
        WriteSpeechEtwEvent("Recognition completed on worker thread");

        SpeechRecognitionCompletedBuffer buffer = { version, status };
        GlobalEventQueue::GetInstance().SendEvent(buffer);
    }

    void PhraseRecognitionSystem::OnRecognitionResult(UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechRecognitionResult* result, int version)
    {
        WriteSpeechEtwEvent("Recognition result generated on worker thread");

        if (GetPlayerPause() == kPlayerPaused)
            return;

        result->AddRef();
        SpeechRecognitionResultBuffer buffer = { version, result };
        GlobalEventQueue::GetInstance().SendEvent(buffer);
    }

    void PhraseRecognitionSystem::HandleEvent(const SpeechRecognitionAsyncActionBuffer& buffer)
    {
        if (buffer.version != m_Version)
        {
            WriteSpeechEtwEvent("AsyncAction completed on main thread (ignoring, event originated in last session)");
            return;
        }

        if (FAILED(buffer.hr))
        {
            if (m_Status != kSpeechSystemFailed)
            {
                const char* lastAction;
                if (m_Tasks.GetCurrentOperation() == &PhraseRecognitionSystem::StartAsync)
                {
                    lastAction = "start";
                }
                else if (m_Tasks.GetCurrentOperation() == &PhraseRecognitionSystem::PauseAsync)
                {
                    lastAction = "pause";
                }
                else if (m_Tasks.GetCurrentOperation() == &PhraseRecognitionSystem::StopAsync)
                {
                    lastAction = "stop";
                }
                else
                {
                    lastAction = "unknown action";
                }

                core::string errorMsg = FormatString("Failed to complete %s recognition system (hr = 0x%X)", lastAction, buffer.hr);
                AssertFormatMsg(false, errorMsg.c_str());
                WriteSpeechEtwEvent(errorMsg.c_str());
                DispatchSpeechError(kUnknownError);
                ChangeSystemStatus(kSpeechSystemFailed);
            }
        }
        else
        {
            if (m_Tasks.GetCurrentOperation() == &PhraseRecognitionSystem::StartAsync)
            {
                WriteSpeechEtwEvent("StartAsync completed on main thread");

                m_PendingRestart = false;

                if (m_HasUncompiledConstraints)
                {
                    if (m_EnabledPhraseRecognizerCount == 0)
                    {
                        m_Tasks.AddTask(&PhraseRecognitionSystem::StopAsync);
                    }
                    else
                    {
                        m_Tasks.AddTask(&PhraseRecognitionSystem::PauseAsync);
                    }
                }
            }
            else if (m_Tasks.GetCurrentOperation() == &PhraseRecognitionSystem::PauseAsync)
            {
                WriteSpeechEtwEvent("PauseAsync completed on main thread");

                if (m_HasUncompiledConstraints)
                {
                    m_Tasks.AddTask(&PhraseRecognitionSystem::CompileAsync);
                }
                else if (m_EnabledPhraseRecognizerCount > 0)
                {
                    if (m_PendingRestart)
                    {
                        m_Tasks.AddTask(&PhraseRecognitionSystem::StartAsync);
                    }
                    else
                    {
                        m_Tasks.AddTask(&PhraseRecognitionSystem::Resume);
                    }
                }
                else
                {
                    m_Tasks.AddTask(&PhraseRecognitionSystem::StopAsync);
                }
            }
            else if (m_Tasks.GetCurrentOperation() == &PhraseRecognitionSystem::StopAsync)
            {
                WriteSpeechEtwEvent("StopAsync completed on main thread");
                m_PendingRestart = true;

                if (m_HasUncompiledConstraints)
                {
                    m_Tasks.AddTask(&PhraseRecognitionSystem::CompileAsync);
                }
                else if (m_EnabledPhraseRecognizerCount > 0)
                {
                    m_Tasks.AddTask(&PhraseRecognitionSystem::StartAsync);
                }
            }
            else
            {
                AssertMsg(false, "Unknown AsyncAction completed on main thread");
                WriteSpeechEtwEvent("Unknown AsyncAction completed on main thread");
            }
        }

        m_Tasks.ProcessNextTask();
    }

    void PhraseRecognitionSystem::HandleEvent(const SpeechCompilationCompletedBuffer& buffer)
    {
        if (buffer.version != m_Version)
        {
            WriteSpeechEtwEvent("CompileAsync completed on main thread (ignoring, event originated in last session)");
            return;
        }

        if (m_Status != kSpeechSystemFailed)
        {
            if (buffer.resultStatus != UnityWinRTBase::SpeechRecognitionResultStatus_Success)
            {
                ReportSystemError(buffer.resultStatus);
            }
            else
            {
                WriteSpeechEtwEvent("CompileAsync completed on main thread");

                if (m_HasUncompiledConstraints)
                {
                    m_Tasks.AddTask(&PhraseRecognitionSystem::CompileAsync);
                }
                else if (m_EnabledPhraseRecognizerCount > 0)
                {
                    if (m_PendingRestart)
                    {
                        m_Tasks.AddTask(&PhraseRecognitionSystem::StartAsync);
                    }
                    else
                    {
                        m_Tasks.AddTask(&PhraseRecognitionSystem::Resume);
                    }
                }
                else if (!m_PendingRestart)
                {
                    m_Tasks.AddTask(&PhraseRecognitionSystem::StopAsync);
                }
            }
        }

        m_Tasks.ProcessNextTask();
    }

    void PhraseRecognitionSystem::HandleEvent(const SpeechRecognitionCompletedBuffer& buffer)
    {
        if (buffer.version != m_Version)
        {
            WriteSpeechEtwEvent("Recognition completed on main thread (ignoring, event originated in last session)");
            return;
        }

        if (m_Status != kSpeechSystemFailed)
        {
#if ENABLE_EVENT_TRACING_FOR_WINDOWS
            WriteSpeechEtwEvent(FormatString("Recognition completed on main thread: (SpeechRecognitionResultStatus)%d", buffer.resultStatus).c_str());
#endif

            switch (buffer.resultStatus)
            {
                case UnityWinRTBase::SpeechRecognitionResultStatus_PauseLimitExceeded:
                case UnityWinRTBase::SpeechRecognitionResultStatus_Success: // This actually means PauseLimitExceeded on Windows 10.0.10240.0, as there's a bug in speech implementation in OS
                case UnityWinRTBase::SpeechRecognitionResultStatus_UserCanceled:
                    if (m_Tasks.HasPendingTasks() || m_EnabledPhraseRecognizerCount == 0)
                    {
                        m_PendingRestart = true;
                    }
                    else
                    {
                        m_Tasks.AddTask(&PhraseRecognitionSystem::StartAsync);
                    }
                    break;

                default:
                    m_PendingRestart = true;
                    ReportSystemError(buffer.resultStatus);
                    break;
            }
        }
    }

    static inline bool InvokeOnPhraseRecognized(dynamic_array<PhraseRecognizer*>& phraseRecognizers, UnityWinRTBase::ISpeechRecognitionConstraint* constraint,
        UnityWinRTBase::ISpeechRecognitionResult* recognitionResult)
    {
        PhraseRecognizer* phraseRecognizer = nullptr;

        for (size_t i = 0; i < phraseRecognizers.size(); i++)
        {
            if (phraseRecognizers[i]->GetConstraint() == constraint)
            {
                phraseRecognizer = phraseRecognizers[i];
                break;
            }
        }

        if (phraseRecognizer != nullptr)
            return phraseRecognizer->OnPhraseRecognized(recognitionResult);

        // This is valid in cases where event fired after we disposed our recognizer,
        // but speech system has not yet paused since it's an async operation
        return false;
    }

    void PhraseRecognitionSystem::HandleEvent(const SpeechRecognitionResultBuffer& buffer)
    {
        if (buffer.version != m_Version)
        {
            WriteSpeechEtwEvent("Recognition result generated on main thread (ignoring, event originated in last session)");
            return;
        }

        WriteSpeechEtwEvent("Recognition result generated on main thread");

        win::ComPtr<UnityWinRTBase::ISpeechRecognitionConstraint> constraint;

        HRESULT hr = buffer.result->get_Constraint(&constraint);
        VerifyHR(hr, "Failed to get ISpeechRecognitionConstraint from ISpeechRecognitionResult (hr = 0x%X)");

        // constraint will be null if recognition is rejected
        if (constraint != nullptr && !InvokeOnPhraseRecognized(m_PhraseRecognizers, constraint, buffer.result))
        {
            win::ComPtr<UnityWinRTBase::IVectorView<UnityWinRTBase::SpeechRecognitionResult*> > alternates;
            hr = buffer.result->GetAlternates(kMaxAlternates, &alternates);
            VerifyHR(hr, "Failed to get alternates from ISpeechRecognitionResult (hr = 0x%X)");

            uint32_t size;
            hr = alternates->get_Size(&size);
            VerifyHR(hr, "Failed to get size of SpeechRecognitionResult alternates (hr = 0x%X)");

            for (uint32_t i = 0; i < size; i++)
            {
                win::ComPtr<UnityWinRTBase::ISpeechRecognitionResult> alternateResult;
                hr = alternates->GetAt(i, &alternateResult);
                VerifyHR(hr, "Failed to get i-th element of SpeechRecognitionResult alternates (hr = 0x%X)");

                hr = alternateResult->get_Constraint(&constraint);
                VerifyHR(hr, "Failed to get ISpeechRecognitionConstraint from ISpeechRecognitionResult (hr = 0x%X)");

                if (constraint == nullptr || InvokeOnPhraseRecognized(m_PhraseRecognizers, constraint, alternateResult))
                    break;
            }
        }
    }

    void PhraseRecognitionSystem::RegisterGlobalCallbacks()
    {
#if UNITY_EDITOR
        GlobalCallbacks::Get().beforeDomainUnload.Register(&PhraseRecognitionSystem::OnBeforeDomainUnload);
        GlobalCallbacks::Get().domainUnloadComplete.Register(&PhraseRecognitionSystem::OnDomainUnloadComplete);
#endif
    }

    void PhraseRecognitionSystem::UnregisterGlobalCallbacks()
    {
#if UNITY_EDITOR
        GlobalCallbacks::Get().beforeDomainUnload.Unregister(&PhraseRecognitionSystem::OnBeforeDomainUnload);
        GlobalCallbacks::Get().domainUnloadComplete.Unregister(&PhraseRecognitionSystem::OnDomainUnloadComplete);
#endif
    }

#if UNITY_EDITOR

    void PhraseRecognitionSystem::OnBeforeDomainUnload()
    {
        // We check for failed here, because if a bad grammar file was loaded, speech will stop working,
        // and users expect it to start working again if they fix the file, and re-enter playmode
        if (GetInstance().GetStatus() == kSpeechSystemRunning || GetInstance().GetStatus() == kSpeechSystemFailed)
            GetInstance().Shutdown();

        auto& phraseRecognizers = GetInstance().m_PhraseRecognizers;

        for (size_t i = 0; i < phraseRecognizers.size(); i++)
            phraseRecognizers[i]->OnBeforeDomainUnload();

        auto& dictationRecognizers = GetInstance().m_DictationRecognizers;

        for (size_t i = 0; i < dictationRecognizers.size(); i++)
            dictationRecognizers[i]->OnBeforeDomainUnload();
    }

    void PhraseRecognitionSystem::OnDomainUnloadComplete()
    {
        // We need to make a copy because Destroy actually alters original array
        {
            dynamic_array<PhraseRecognizer*> phraseRecognizers(GetInstance().m_PhraseRecognizers, kMemTempAlloc);

            for (size_t i = 0; i < phraseRecognizers.size(); i++)
            {
                if (!phraseRecognizers[i]->IsQueuedForDestruction())
                    phraseRecognizers[i]->Destroy();
            }
        }

        {
            dynamic_array<DictationRecognizer*> dictationRecognizers(GetInstance().m_DictationRecognizers, kMemTempAlloc);

            for (size_t i = 0; i < dictationRecognizers.size(); i++)
            {
                if (!dictationRecognizers[i]->IsQueuedForDestruction())
                    dictationRecognizers[i]->Destroy();
            }
        }
    }

#endif // UNITY_EDITOR

#undef VerifyHR
#undef VerifyAsyncHR
} // namespace Unity

#endif // PLATFORM_WIN
