#include "PlatformDependent/Win/Speech/DictationRecognizer.h"
#include "PlatformDependent/Win/Speech/PhraseRecognizer.h"
#include "PlatformDependent/Win/Speech/PhraseRecognitionSystem.h"
#include "Runtime/Scripting/BindingsDefs.h"
#include "Runtime/Scripting/ScriptingExportUtility.h"

using namespace Unity;

class PhraseRecognitionSystemBindings
{
public:
    inline static bool GetIsSupported()
    {
#if PLATFORM_WIN
        return PhraseRecognitionSystem::IsSupported();
#else
        return false;
#endif
    }

    inline static SpeechSystemStatus GetStatus()
    {
#if PLATFORM_WIN
        return PhraseRecognitionSystem::GetInstance().GetStatus();
#else
        return kSpeechSystemStopped;
#endif
    }

    inline static void Restart(ScriptingExceptionPtr* exception)
    {
#if PLATFORM_WIN
        PhraseRecognitionSystem::GetInstance().Restart(exception);
#endif
    }

    inline static void Shutdown()
    {
#if PLATFORM_WIN
        PhraseRecognitionSystem::GetInstance().Shutdown();
#endif
    }
};

class PhraseRecognizerBindings
{
public:
    inline static void* CreateFromKeywords(ScriptingObjectPtr self, ScriptingArrayPtr keywords, ConfidenceLevel minimumConfidence, ScriptingExceptionPtr* exception)
    {
#if PLATFORM_WIN
        core::string errorStr;
        void* result = PhraseRecognizer::Create(self, keywords, minimumConfidence, errorStr);

        if (!errorStr.empty())
        {
            *exception = Scripting::CreateUnityException(errorStr.c_str());
            return NULL;
        }

        return result;
#else
        return NULL;
#endif
    }

    inline static void* CreateFromGrammarFile(ScriptingObjectPtr self, ICallString grammarFilePath, ConfidenceLevel minimumConfidence, ScriptingExceptionPtr* exception)
    {
#if PLATFORM_WIN
        core::string errorStr;
        void* result = PhraseRecognizer::Create(self, grammarFilePath, minimumConfidence, errorStr);

        if (!errorStr.empty())
        {
            *exception = Scripting::CreateUnityException(errorStr.c_str());
            return NULL;
        }

        return result;
#else
        return NULL;
#endif
    }

    inline static void Start_Internal(void* recognizer, ScriptingExceptionPtr* exception)
    {
#if PLATFORM_WIN
        static_cast<PhraseRecognizer*>(recognizer)->Start(exception);
#endif
    }

    inline static void Stop_Internal(void* recognizer)
    {
#if PLATFORM_WIN
        static_cast<PhraseRecognizer*>(recognizer)->Stop();
#endif
    }

    inline static bool IsRunning_Internal(void* recognizer)
    {
#if PLATFORM_WIN
        return static_cast<PhraseRecognizer*>(recognizer)->IsRunning();
#else
        return false;
#endif
    }

    inline static void Destroy(void* recognizer)
    {
#if PLATFORM_WIN
        static_cast<PhraseRecognizer*>(recognizer)->Destroy();
#endif
    }

    inline static void DestroyThreaded(void* recognizer)
    {
#if PLATFORM_WIN
        static_cast<PhraseRecognizer*>(recognizer)->DestroyThreaded();
#endif
    }
};

class DictationRecognizerBindings
{
public:
    inline static void* Create(ScriptingObjectPtr self, ConfidenceLevel minimumConfidence, DictationTopicConstraint topicConstraint, ScriptingExceptionPtr* exception)
    {
#if PLATFORM_WIN
        if (!PhraseRecognitionSystem::IsSupported())
        {
            *exception = Scripting::CreateUnityException("Speech recognition is not supported on this machine.");
            return NULL;
        }

        core::string errorMessage;
        DictationRecognizer* dictationRecognizer = UNITY_NEW(DictationRecognizer, kMemSpeech)(self, minimumConfidence, topicConstraint, errorMessage);

        if (errorMessage.empty())
            return dictationRecognizer;

        dictationRecognizer->Release();
        *exception = Scripting::CreateUnityException(errorMessage.c_str());
        return NULL;
#else
        return NULL;
#endif
    }

    inline static void Start(void* self, ScriptingExceptionPtr* exception)
    {
#if PLATFORM_WIN
        static_cast<DictationRecognizer*>(self)->StartRecognition(exception);
#endif
    }

    inline static void Stop(void* self)
    {
#if PLATFORM_WIN
        static_cast<DictationRecognizer*>(self)->StopRecognition();
#endif
    }

    inline static void Destroy(void* self)
    {
#if PLATFORM_WIN
        static_cast<DictationRecognizer*>(self)->Destroy();
#endif
    }

    inline static void DestroyThreaded(void* self)
    {
#if PLATFORM_WIN
        static_cast<DictationRecognizer*>(self)->DestroyThreaded();
#endif
    }

    inline static SpeechSystemStatus GetStatus(void* self)
    {
#if PLATFORM_WIN
        return static_cast<DictationRecognizer*>(self)->GetStatus();
#else
        return kSpeechSystemStopped;
#endif
    }

    inline static float GetAutoSilenceTimeoutSeconds(void* self)
    {
#if PLATFORM_WIN
        return static_cast<DictationRecognizer*>(self)->GetAutoSilenceTimeoutSeconds();
#else
        return 0.0f;
#endif
    }

    inline static void SetAutoSilenceTimeoutSeconds(void* self, float value)
    {
#if PLATFORM_WIN
        static_cast<DictationRecognizer*>(self)->SetAutoSilenceTimeoutSeconds(value);
#endif
    }

    inline static float GetInitialSilenceTimeoutSeconds(void* self)
    {
#if PLATFORM_WIN
        return static_cast<DictationRecognizer*>(self)->GetInitialSilenceTimeoutSeconds();
#else
        return 0.0f;
#endif
    }

    inline static void SetInitialSilenceTimeoutSeconds(void* self, float value)
    {
#if PLATFORM_WIN
        static_cast<DictationRecognizer*>(self)->SetInitialSilenceTimeoutSeconds(value);
#endif
    }
};

BIND_MANAGED_TYPE_NAME(PhraseRecognitionSystemBindings, UnityEngine_Windows_Speech_PhraseRecognitionSystem);
BIND_MANAGED_TYPE_NAME(PhraseRecognizerBindings, UnityEngine_Windows_Speech_PhraseRecognizer);
BIND_MANAGED_TYPE_NAME(DictationRecognizerBindings, UnityEngine_Windows_Speech_DictationRecognizer);
BIND_MANAGED_TYPE_NAME(SpeechSystemStatus, UnityEngine_Windows_Speech_SpeechSystemStatus)
BIND_MANAGED_TYPE_NAME(ConfidenceLevel, UnityEngine_Windows_Speech_ConfidenceLevel)
BIND_MANAGED_TYPE_NAME(DictationTopicConstraint, UnityEngine_Windows_Speech_DictationTopicConstraint)
