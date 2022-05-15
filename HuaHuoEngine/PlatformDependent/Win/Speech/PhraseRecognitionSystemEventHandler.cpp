#include "UnityPrefix.h"
#include "PhraseRecognitionSystem.h"
#include "PhraseRecognitionSystemEventHandler.h"

#if PLATFORM_WIN

using namespace Unity;
using namespace UnityWinRTBase::Windows::Foundation;
using namespace UnityWinRTBase::Windows::Media::SpeechRecognition;

HRESULT PhraseRecognitionSystemEventHandler::Invoke(ISpeechContinuousRecognitionSession* sender, ISpeechContinuousRecognitionResultGeneratedEventArgs* args)
{
    win::ComPtr<ISpeechRecognitionResult> speechRecognitionResult;
    HRESULT hr = args->get_Result(&speechRecognitionResult);
    Assert(SUCCEEDED(hr));

    if (SUCCEEDED(hr))
    {
        m_PhraseRecognitionSystem->OnRecognitionResult(speechRecognitionResult, m_Version);
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE PhraseRecognitionSystemEventHandler::Invoke(ISpeechContinuousRecognitionSession* sender, ISpeechContinuousRecognitionCompletedEventArgs* args)
{
    SpeechRecognitionResultStatus status;
    HRESULT hr = args->get_Status(&status);
    Assert(SUCCEEDED(hr));

    if (FAILED(hr))
        status = SpeechRecognitionResultStatus_Unknown;

    m_PhraseRecognitionSystem->OnRecognitionCompleted(status, m_Version);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PhraseRecognitionSystemEventHandler::Invoke(IAsyncOperation<SpeechRecognitionCompilationResult*>* asyncInfo, UnityWinRTBase::Windows::Foundation::AsyncStatus status)
{
    SpeechRecognitionResultStatus compilationStatus = SpeechRecognitionResultStatus_Unknown;
    Assert(status == Completed);

    if (status == Completed)
    {
        win::ComPtr<ISpeechRecognitionCompilationResult> compilationResult;
        HRESULT hr = asyncInfo->GetResults(&compilationResult);
        Assert(SUCCEEDED(hr));

        if (SUCCEEDED(hr))
        {
            hr = compilationResult->get_Status(&compilationStatus);
            Assert(SUCCEEDED(hr));
        }
    }

    m_PhraseRecognitionSystem->OnCompilationCompleted(compilationStatus, m_Version);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE PhraseRecognitionSystemEventHandler::Invoke(IAsyncAction* asyncInfo, UnityWinRTBase::Windows::Foundation::AsyncStatus asyncStatus)
{
    HRESULT hr = asyncInfo->GetResults();
    m_PhraseRecognitionSystem->OnAsyncActionCompleted(hr, m_Version);

    return S_OK;
}

#endif
