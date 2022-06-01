#pragma once

#if PLATFORM_WIN

#include "External/Windows10/src/ComHelpers.h"
#include "External/Windows10/src/Windows10Interfaces.h"
#include "PhraseRecognitionSystem.h"
#include "PlatformDependent/Win/ComPtr.h"
#include "Runtime/Threads/TaskChainProcessor.h"
#include "Runtime/BaseClasses/BaseObject.h"
#include "Runtime/EventQueue/GlobalEventQueue.h"
#include "Runtime/Threads/MainThreadValue.h"

#endif

namespace Unity
{
    enum DictationCompletionCause
    {
        kDCCauseComplete = 0,           // underlying system completed the phrase without issue
        kDCCauseAudioQualityFailure = 1, // audio issues caused a completion event
        kDCCauseCanceled = 2,           // user canceled the recognition task
        kDCCauseTimeout = 3,            // timeout forced the system to complete
        kDCCausePauseLimitExceeded = 4, // system paused too long
        kDCCauseNetworkFailure = 5,     // network issues caused the task to complete
        kDCCauseMicrophoneUnavailable = 6, // no mic == no recognition tasks
        kDCCauseUnknownError = 7        // other random error expected or otherwise
    };

    enum DictationTopicConstraint
    {
        kWebSearch,
        kForm,
        kDictation
    };

#if PLATFORM_WIN

    class DictationRecognizer;
    typedef ClassAsyncTask<Unity::DictationRecognizer> DictationTask;

    struct DictationResultBuffer;
    struct DictationHypothesisBuffer;
    struct DictationCompletedBuffer;
    struct DictationErrorBuffer;
    struct DictationCompileComplete;
    struct DictationAsyncComplete;
    struct DictationDestructionBuffer;
    struct DictationSessionEventHandler;

    class DictationRecognizer :
        public UnityWinRTBase::ComClass<>
    {
    public:
        DictationRecognizer(ScriptingObjectPtr managedCounterpart, ConfidenceLevel minimumConfidence, DictationTopicConstraint dictationTopicConstraint, core::string& outErrorMessage);

        void StartRecognition(ScriptingExceptionPtr* exception);
        void StopRecognition();

        void Destroy();
        void DestroyThreaded();

        float GetAutoSilenceTimeoutSeconds();
        void SetAutoSilenceTimeoutSeconds(float newVal);
        float GetInitialSilenceTimeoutSeconds();
        void SetInitialSilenceTimeoutSeconds(float newVal);

        inline SpeechSystemStatus GetStatus() const { return m_Status; }

        // event thunks from our event system that originate on API callback
        // threads.  these are needed to validate recognizer pointers and to latch
        // our handling on the main thread.  this works because a) our thunk will
        // only be called on the main thread, and b) it's only valid to kill
        // recognizers on the main thread so we don't require additional synchronization.
        void HandleEvent(const DictationResultBuffer& event);
        void HandleEvent(const DictationHypothesisBuffer& event);
        void HandleEvent(const DictationCompletedBuffer& event);
        void HandleEvent(const DictationErrorBuffer& event);
        void HandleEvent(const DictationCompileComplete& event);
        void HandleEvent(const DictationAsyncComplete& event);
        static void HandleEvent(const DictationDestructionBuffer& event);

#if UNITY_EDITOR
        void OnBeforeDomainUnload();
        inline bool IsQueuedForDestruction() const { return m_IsQueuedForDestruction; }
#endif

    private:
        win::ComPtr<UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechRecognizer> m_Recognizer;
        win::ComPtr<UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechContinuousRecognitionSession> m_ContinuousRecognitionSession;
        win::ComPtr<UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechRecognizerTimeouts> m_Timeouts;
        win::ComPtr<DictationSessionEventHandler> m_DictationSessionEventHandler;
        TaskChainProcessor<DictationRecognizer> m_Tasks;

        ConfidenceLevel m_MinimumConfidence;
        ScriptingGCHandle m_ManagedCounterpart;
        SpeechSystemStatus m_Status;

        // There is a subtle edge case:
        // 1. Dictation recognizer is stopped (StopAsync starts)
        // 2. DictationRecognizer::Destroy() gets called
        // 3. At this point, we won't ever process StopAsync completion on the main thread because we unsubscribe from events in DictationRecognizer::Destroy(), therefore our status never gets changed to stopped
        // 4. StopAsync gets called again because we don't know we're stopped
        //
        // To solve this, I introduced this m_IsStopping variable, which gets set to true in StopAsync and gets set to false in StopAsync main thread handler.
        MainThreadValue<bool> m_IsStopping;

        UnityWinRTBase::EventRegistrationToken m_OnHypothesisGeneratedToken;
        UnityWinRTBase::EventRegistrationToken m_OnResultGeneratedToken;
        UnityWinRTBase::EventRegistrationToken m_OnCompletedToken;

#if UNITY_EDITOR
        volatile bool m_IsQueuedForDestruction;
#endif

        // delegates for event thunks
        UnityEventQueue::ClassBasedEventHandler<Unity::DictationResultBuffer, Unity::DictationRecognizer> m_ResultEventDelegate;
        UnityEventQueue::ClassBasedEventHandler<Unity::DictationHypothesisBuffer, Unity::DictationRecognizer> m_HypothesisEventDelegate;
        UnityEventQueue::ClassBasedEventHandler<Unity::DictationCompletedBuffer, Unity::DictationRecognizer> m_CompletionEventDelegate;
        UnityEventQueue::ClassBasedEventHandler<Unity::DictationErrorBuffer, Unity::DictationRecognizer> m_ErrorEventDelegate;
        UnityEventQueue::ClassBasedEventHandler<Unity::DictationCompileComplete, Unity::DictationRecognizer> m_CompileCompleteDelegate;
        UnityEventQueue::ClassBasedEventHandler<Unity::DictationAsyncComplete, Unity::DictationRecognizer> m_AsyncCompleteDelegate;

        virtual ~DictationRecognizer();

        void ReportDictationError(const char * format, ...);
        bool VerifyAndReportHR(HRESULT hr, const char * format, ...);

        // operations
        void StartAsync();
        void StopAsync();
        void CompileAsync();

        friend struct DictationSessionEventHandler;
    };

#endif // PLATFORM_WIN
}
