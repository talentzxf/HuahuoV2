#pragma once

#if PLATFORM_WIN

#include "External/Windows10/src/Windows10Interfaces.h"
#include "PhraseRecognitionSystem.h"
#include "PlatformDependent/Win/ComPtr.h"
#include "Runtime/Scripting/ScriptingExportUtility.h"

namespace Unity
{
    struct PhraseRecognizerDestructionBuffer;

    class PhraseRecognizer
    {
    public:
        static PhraseRecognizer* Create(ScriptingObjectPtr self, const wchar_t* const* keywords, size_t keywordCount, ConfidenceLevel minimumConfidence, core::string& outErrorMessage);
        static PhraseRecognizer* Create(ScriptingObjectPtr self, ScriptingArrayPtr keywords, ConfidenceLevel minimumConfidence, core::string& outErrorMessage);
        static PhraseRecognizer* Create(ScriptingObjectPtr self, UnityWinRTBase::Windows::Foundation::Collections::IIterable<UnityWinRTBase::HSTRING>* keywords, ConfidenceLevel minimumConfidence, core::string& outErrorMessage);
        static PhraseRecognizer* Create(ScriptingObjectPtr self, ICallString grammarFilePath, ConfidenceLevel minimumConfidence, core::string& outErrorMessage);

        void Start(ScriptingExceptionPtr* exception);
        void Stop();

        inline bool IsRunning() const { return m_IsRunning; }

        bool OnPhraseRecognized(UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechRecognitionResult* recognitionResult);

        // This explicit version allows us to emulate phrase recognition (e.g., for remoting)
        bool OnPhraseRecognized(const wchar_t* text, int confidence);

        void Destroy();
        void DestroyThreaded();

        inline win::ComPtr<UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechRecognitionConstraint> GetConstraint() { return m_Constraint; }

#if UNITY_EDITOR
        void OnBeforeDomainUnload();
        // Editor needs to know whether finalizer managed to queue up object for destruction when domain reloads
        inline bool IsQueuedForDestruction() const { return m_IsQueuedForDestruction; }
#endif

    protected:
        bool InvokePhraseRecognizedEvent(ScriptingStringPtr text, int confidence, ScriptingObjectPtr semanticInterpretation, int64_t phraseStartTime, int64_t phraseDuration);

    private:
        win::ComPtr<UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechRecognitionConstraint> m_Constraint;
        ScriptingGCHandle m_ManagedCounterpart;
        ConfidenceLevel m_MinimumConfidence;
        bool m_IsRunning;
#if UNITY_EDITOR
        volatile bool m_IsQueuedForDestruction;
#endif

        PhraseRecognizer(ScriptingObjectPtr managedCounterpart, ConfidenceLevel minimumConfidence, UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechRecognitionConstraint* constraint);

        static void HandleEvent(const PhraseRecognizerDestructionBuffer& buffer);
    };
}

#endif // PLATFORM_WIN
