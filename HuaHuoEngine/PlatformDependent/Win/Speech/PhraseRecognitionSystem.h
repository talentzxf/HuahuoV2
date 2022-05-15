#pragma once

#if PLATFORM_WIN

#include "PhraseRecognitionSystemEventHandler.h"
#include "PlatformDependent/Win/ComPtr.h"
#include "Runtime/EventQueue/GlobalEventQueue.h"
#include "Runtime/Threads/TaskChainProcessor.h"
#include "Runtime/Utilities/dynamic_array.h"

#endif

class ScriptingExceptionPtr;

namespace Unity
{
    // Mirrored in Speech.cs
    enum ConfidenceLevel
    {
        kConfidenceLevel_High = 0,
        kConfidenceLevel_Medium = 1,
        kConfidenceLevel_Low = 2,
        kConfidenceLevel_Rejected = 3,
    };

    // Mirrored in Speech.cs
    enum SpeechSystemStatus
    {
        kSpeechSystemStopped = 0,
        kSpeechSystemRunning = 1,
        kSpeechSystemFailed = 2,
    };

    // Mirrored in Speech.cs
    enum SpeechError
    {
        kNoError = 0,
        kTopicLanguageNotSupported = 1,
        kGrammarLanguageMismatch = 2,
        kGrammarCompilationFailure = 3,
        kAudioQualityFailure = 4,
        kPauseLimitExceeded = 5,
        kTimeoutExceeded = 6,
        kNetworkFailure = 7,
        kMicrophoneUnavailable = 8,
        kUnknownError = 9,
    };

#if PLATFORM_WIN

    class PhraseRecognizer;
    class DictationRecognizer;
    struct SpeechRecognitionAsyncActionBuffer;
    struct SpeechCompilationCompletedBuffer;
    struct SpeechRecognitionCompletedBuffer;
    struct SpeechRecognitionResultBuffer;

    class PhraseRecognitionSystem
    {
    public:
        static bool IsSupported();
        static PhraseRecognitionSystem& GetInstance();

        SpeechSystemStatus GetStatus() const { return m_Status; }
        void Restart(ScriptingExceptionPtr* exception);
        void Shutdown();

        void AddPhraseRecognizer(PhraseRecognizer* phraseRecognizer);
        void RemovePhraseRecognizer(PhraseRecognizer* phraseRecognizer);

        void AddDictationRecognizer(DictationRecognizer* dictationRecognizer);
        void RemoveDictationRecognizer(DictationRecognizer* dictationRecognizer);

        void IncrementEnabledRecognizerCount(ScriptingExceptionPtr* exception);
        void DecrementEnabledRecognizerCount();

        inline const dynamic_array<PhraseRecognizer*>& GetPhraseRecognizers() { return m_PhraseRecognizers; }
        inline const dynamic_array<DictationRecognizer*>& GetDictationRecognizers() { return m_DictationRecognizers; }

    private:
        SpeechSystemStatus m_Status;
        bool m_PendingRestart;

        // This recognizer has a ridiculous keyword constraint, and we don't expect it to fire (no harm done if it does)
        // We need it in order to keep speech system active in case all other phrase recognizers are disabled
        PhraseRecognizer* m_FakePhraseRecognizer;
        dynamic_array<PhraseRecognizer*> m_PhraseRecognizers;
        dynamic_array<DictationRecognizer*> m_DictationRecognizers;
        bool m_HasUncompiledConstraints;
        int m_EnabledPhraseRecognizerCount;
        int m_Version;

        win::ComPtr<UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechRecognizer> m_Recognizer;
        win::ComPtr<UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechContinuousRecognitionSession> m_ContinuousRecognitionSession;
        win::ComPtr<PhraseRecognitionSystemEventHandler> m_EventHandler;

        UnityWinRTBase::EventRegistrationToken m_CompletedToken;
        UnityWinRTBase::EventRegistrationToken m_ResultGeneratedToken;

        UnityEventQueue::ClassBasedEventHandler<SpeechRecognitionAsyncActionBuffer, PhraseRecognitionSystem> m_AsyncActionEventDelegate;
        UnityEventQueue::ClassBasedEventHandler<SpeechCompilationCompletedBuffer, PhraseRecognitionSystem> m_CompilationCompletedEventDelegate;
        UnityEventQueue::ClassBasedEventHandler<SpeechRecognitionCompletedBuffer, PhraseRecognitionSystem> m_RecognitionCompletedEventDelegate;
        UnityEventQueue::ClassBasedEventHandler<SpeechRecognitionResultBuffer, PhraseRecognitionSystem> m_RecognitionResultEventDelegate;

        TaskChainProcessor<PhraseRecognitionSystem> m_Tasks;

        bool m_ShouldRestartOnResuming;

        PhraseRecognitionSystem();
        ~PhraseRecognitionSystem();
        void ReportSystemError(UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus);
        void DispatchSpeechError(SpeechError error);
        void ChangeSystemStatus(SpeechSystemStatus newStatus);

        void StartAsync();
        void CompileAsync();
        void StopAsync();
        void PauseAsync();
        void Resume();

        void OnAsyncActionCompleted(HRESULT hr, int version);
        void OnCompilationCompleted(UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus args, int version);
        void OnRecognitionCompleted(UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionResultStatus args, int version);
        void OnRecognitionResult(UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechRecognitionResult* result, int version);

        void HandleEvent(const SpeechRecognitionAsyncActionBuffer& buffer);
        void HandleEvent(const SpeechCompilationCompletedBuffer& buffer);
        void HandleEvent(const SpeechRecognitionCompletedBuffer& buffer);
        void HandleEvent(const SpeechRecognitionResultBuffer& buffer);

        void RegisterGlobalCallbacks();
        void UnregisterGlobalCallbacks();

        friend class PhraseRecognitionSystemEventHandler;
        friend class UnityEventQueue::ClassBasedEventHandler<SpeechRecognitionAsyncActionBuffer, PhraseRecognitionSystem>;
        friend class UnityEventQueue::ClassBasedEventHandler<SpeechCompilationCompletedBuffer, PhraseRecognitionSystem>;
        friend class UnityEventQueue::ClassBasedEventHandler<SpeechRecognitionCompletedBuffer, PhraseRecognitionSystem>;
        friend class UnityEventQueue::ClassBasedEventHandler<SpeechRecognitionResultBuffer, PhraseRecognitionSystem>;

#if UNITY_EDITOR
        static void OnBeforeDomainUnload();
        static void OnDomainUnloadComplete();
#endif
    };

#endif // PLATFORM_WIN
}
