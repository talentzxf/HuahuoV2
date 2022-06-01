#pragma once

#if PLATFORM_WIN

#include "External/Windows10/src/ComHelpers.h"
#include "External/Windows10/src/Windows10Interfaces.h"

namespace Unity
{
    class PhraseRecognitionSystem;

    class PhraseRecognitionSystemEventHandler :
        public UnityWinRTBase::ComClass<
            UnityWinRTBase::Windows::Foundation::ITypedEventHandler<UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechContinuousRecognitionSession*, UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechContinuousRecognitionCompletedEventArgs*>,
            UnityWinRTBase::Windows::Foundation::ITypedEventHandler<UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechContinuousRecognitionSession*, UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechContinuousRecognitionResultGeneratedEventArgs*>,
            UnityWinRTBase::Windows::Foundation::IAsyncOperationCompletedHandler<UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionCompilationResult*>,
            UnityWinRTBase::Windows::Foundation::IAsyncActionCompletedHandler>
    {
    private:
        PhraseRecognitionSystem* m_PhraseRecognitionSystem;
        int m_Version;

    public:
        inline PhraseRecognitionSystemEventHandler(PhraseRecognitionSystem* phraseRecognitionSystem, int version) :
            m_PhraseRecognitionSystem(phraseRecognitionSystem), m_Version(version)
        {
            SetLabel(kMemSpeech);
        }

        // ResultGenerated handler
        virtual HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechContinuousRecognitionSession* sender, UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechContinuousRecognitionResultGeneratedEventArgs* args) override;

        // Completed handler
        virtual HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechContinuousRecognitionSession* sender, UnityWinRTBase::Windows::Media::SpeechRecognition::ISpeechContinuousRecognitionCompletedEventArgs* args) override;

        // CompilationCompleted handler
        virtual HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::Windows::Foundation::IAsyncOperation<UnityWinRTBase::Windows::Media::SpeechRecognition::SpeechRecognitionCompilationResult*>* asyncInfo, UnityWinRTBase::Windows::Foundation::AsyncStatus status) override;

        // StartAsync handler
        // PauseAsync handler
        virtual HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::Windows::Foundation::IAsyncAction* asyncInfo, UnityWinRTBase::Windows::Foundation::AsyncStatus asyncStatus) override;
    };
}

#endif
