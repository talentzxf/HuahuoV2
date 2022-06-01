#include "UnityPrefix.h"
#include "External/Windows10/src/SynchronousOperation.h"
#include "External/Windows10/src/WinRTCollections.h"
#include "PhraseRecognizer.h"
#include "PlatformDependent/Win/WinUnicode.h"
#include "Runtime/Scripting/CommonScriptingClasses.h"
#include "Runtime/Scripting/ScriptingManager.h"
#include "Runtime/Utilities/dynamic_array.h"

#if PLATFORM_WINRT
#include "PlatformDependent/MetroPlayer/MetroCapabilities.h"
#endif

#if PLATFORM_WIN

namespace UnityWinRTBase
{
    using namespace UnityWinRTBase::Windows::Foundation;
    using namespace UnityWinRTBase::Windows::Foundation::Collections;
    using namespace UnityWinRTBase::Windows::Media::SpeechRecognition;
    using namespace UnityWinRTBase::Windows::Storage;
}

namespace Unity
{
    struct PhraseRecognizerDestructionBuffer
    {
        PhraseRecognizer* phraseRecognizer;
        ScriptingBackendNativeGCHandle gchandle;

        void Destroy()
        {
            ScriptingGCHandle::FromScriptingBackendNativeGCHandle(gchandle).ReleaseAndClear();
        }
    };
}

using namespace Unity;

REGISTER_EVENT_ID_WITH_CLEANUP(0xD4D0D9DB91C74ED2ULL, 0x9438FFCB15991CB4ULL, PhraseRecognizerDestructionBuffer)

#define VerifyHR(hr, message) \
            if (FAILED(hr)) \
            { /* We can't really throw exception here as we depend on RAII and Mono doesn't unroll the stack */ \
              /* So instead we just allocate a temp string as an out parameter so the caller can throw exception instead */ \
                outErrorMessage = FormatString(message, hr); \
                return nullptr; \
            }

static inline void VerifyDuplicateKeywords(const wchar_t* keyword)
{
#if UNITY_DEVELOPER_BUILD
    // Verify that we have no duplicate keywords already used, and print error if there is
    // We do this only in development build, to not affect performance in final build
    // We cannot throw exception from here, because it would alter behaviour between development player and non-development player
    const dynamic_array<PhraseRecognizer*>& recognizers = PhraseRecognitionSystem::GetInstance().GetPhraseRecognizers();

    for (size_t i = 0; i < recognizers.size(); i++)
    {
#if UNITY_EDITOR
        if (recognizers[i]->IsQueuedForDestruction())
            continue;
#endif

        win::ComPtr<UnityWinRTBase::ISpeechRecognitionListConstraint> listConstraint;
        HRESULT hr = recognizers[i]->GetConstraint().As(&listConstraint);
        if (FAILED(hr))
            continue;

        win::ComPtr<UnityWinRTBase::IVector<UnityWinRTBase::HSTRING> > commands;
        hr = listConstraint->get_Commands(&commands);
        if (FAILED(hr))
            continue;

        uint32_t commandCount;
        hr = commands->get_Size(&commandCount);
        if (FAILED(hr))
            continue;

        for (uint32_t i = 0; i < commandCount; i++)
        {
            UnityWinRTBase::HString command;
            hr = commands->GetAt(i, &command);
            if (FAILED(hr))
                break;

            if (_wcsicmp(keyword, command.GetBuffer()) == 0)
            {
                core::string keywordUtf8;
                ConvertWideToUTF8String(keyword, keywordUtf8);
                ErrorStringMsg("Error: there already is a keyword recognizer with \"%s\" as one of its keywords", keywordUtf8.c_str());
                return;
            }
        }
    }
#endif
}

static inline void VerifyDuplicateGrammarFiles(ICallString grammarFilePath)
{
#if UNITY_DEVELOPER_BUILD
    const wchar_t* path = reinterpret_cast<const wchar_t*>(grammarFilePath.GetRawCharBuffer());

    // Verify that we have no duplicate grammar files, and print error if there is
    // We do this only in development build, to not affect performance in final build
    // We cannot throw exception from here, because it would alter behaviour between development player and non-development player
    const dynamic_array<PhraseRecognizer*>& recognizers = PhraseRecognitionSystem::GetInstance().GetPhraseRecognizers();

    for (size_t i = 0; i < recognizers.size(); i++)
    {
        win::ComPtr<UnityWinRTBase::ISpeechRecognitionGrammarFileConstraint> grammarFileConstraint;
        HRESULT hr = recognizers[i]->GetConstraint().As(&grammarFileConstraint);
        if (FAILED(hr))
            continue;

        win::ComPtr<UnityWinRTBase::IStorageFile> storageFile;
        hr = grammarFileConstraint->get_GrammarFile(&storageFile);
        if (FAILED(hr))
            continue;

        win::ComPtr<UnityWinRTBase::IStorageItem> storageItem;
        hr = storageFile.As(&storageItem);
        if (FAILED(hr))
            continue;

        UnityWinRTBase::HString storageItemPath;
        hr = storageItem->get_Path(&storageItemPath);
        if (FAILED(hr))
            continue;

        if (_wcsicmp(path, storageItemPath.GetBuffer()) == 0)
        {
            ErrorStringMsg("Error: there already is a grammar recognizer which uses \"%s\" grammar file", grammarFilePath.ToUTF8().c_str());
            break;
        }
    }
#endif
}

static inline bool VerifyCapabilities()
{
#if PLATFORM_WINRT
    return metro::Capabilities::IsSupported(metro::Capabilities::kMicrophone, "in order to enable speech recognition functionality", true);
#else
    return true;
#endif
}

PhraseRecognizer* PhraseRecognizer::Create(ScriptingObjectPtr self, const wchar_t* const* keywords, size_t keywordCount, ConfidenceLevel minimumConfidence, core::string& outErrorMessage)
{
    win::ComPtr<Vector<UnityWinRTBase::HSTRING> > keywordsVector;
    keywordsVector.Attach(UNITY_NEW(Vector<UnityWinRTBase::HSTRING>, kMemSpeech)(kMemSpeech));
    keywordsVector->Reserve(keywordCount);

    for (size_t i = 0; i < keywordCount; i++)
    {
        VerifyDuplicateKeywords(keywords[i]);
        keywordsVector->Append(UnityWinRTBase::HStringReference(keywords[i]));
    }

    return Create(self, keywordsVector, minimumConfidence, outErrorMessage);
}

PhraseRecognizer* PhraseRecognizer::Create(ScriptingObjectPtr self, ScriptingArrayPtr keywords, ConfidenceLevel minimumConfidence, core::string& outErrorMessage)
{
    size_t keywordCount = GetScriptingArraySize(keywords);
    win::ComPtr<Vector<UnityWinRTBase::HSTRING> > keywordsVector;
    keywordsVector.Attach(UNITY_NEW(Vector<UnityWinRTBase::HSTRING>, kMemSpeech)(kMemSpeech));
    keywordsVector->Reserve(keywordCount);

    for (size_t i = 0; i < keywordCount; i++)
    {
        ScriptingStringPtr keyword = Scripting::GetScriptingArrayElementNoRef<ScriptingStringPtr>(keywords, i);
        const wchar_t* keywordChars = reinterpret_cast<const wchar_t*>(scripting_string_chars(keyword));

        VerifyDuplicateKeywords(keywordChars);
        keywordsVector->Append(UnityWinRTBase::HStringReference(keywordChars));
    }

    return Create(self, keywordsVector, minimumConfidence, outErrorMessage);
}

PhraseRecognizer* PhraseRecognizer::Create(ScriptingObjectPtr self, UnityWinRTBase::Windows::Foundation::Collections::IIterable<UnityWinRTBase::HSTRING>* keywords, ConfidenceLevel minimumConfidence, core::string& outErrorMessage)
{
    outErrorMessage.clear();

    if (!PhraseRecognitionSystem::IsSupported())
    {
        outErrorMessage = "Speech recognition is not supported on this machine.";
        return nullptr;
    }

    if (!VerifyCapabilities())
        return nullptr;

    win::ComPtr<UnityWinRTBase::ISpeechRecognitionListConstraintFactory> listConstraintFactory;
    auto hr = UnityWinRTBase::RoGetActivationFactory(UnityWinRTBase::HStringReference(L"Windows.Media.SpeechRecognition.SpeechRecognitionListConstraint"),
        __uuidof(UnityWinRTBase::ISpeechRecognitionListConstraintFactory), &listConstraintFactory);

    // Fail silently here, as it most likely means we're not on Windows 10
    if (FAILED(hr))
        return nullptr;

    win::ComPtr<UnityWinRTBase::ISpeechRecognitionListConstraint> listConstraint;
    hr = listConstraintFactory->Create(keywords, &listConstraint);
    VerifyHR(hr, "Failed to create SpeechRecognitionListConstraint (hr = 0x%X)");

    win::ComPtr<UnityWinRTBase::ISpeechRecognitionConstraint> constraint;
    hr = listConstraint.As(&constraint);
    VerifyHR(hr, "Failed to cast ISpeechRecognitionListConstraint to ISpeechRecognitionConstraint (hr = 0x%X)");

    hr = constraint->put_IsEnabled(false);
    VerifyHR(hr, "Failed to disable ISpeechRecognitionConstraint for initial creation (hr = 0x%X)");

    return UNITY_NEW(PhraseRecognizer, kMemSpeech)(self, minimumConfidence, constraint);
}

PhraseRecognizer* PhraseRecognizer::Create(ScriptingObjectPtr self, ICallString grammarFilePath, ConfidenceLevel minimumConfidence, core::string& outErrorMessage)
{
    outErrorMessage.clear();

    if (!VerifyCapabilities())
        return nullptr;

    win::ComPtr<UnityWinRTBase::ISpeechRecognitionGrammarFileConstraintFactory> grammarFileConstraintFactory;
    auto hr = UnityWinRTBase::RoGetActivationFactory(UnityWinRTBase::HStringReference(L"Windows.Media.SpeechRecognition.SpeechRecognitionGrammarFileConstraint"),
        __uuidof(UnityWinRTBase::ISpeechRecognitionGrammarFileConstraintFactory), &grammarFileConstraintFactory);

    // Fail silently here, as it most likely means we're not on Windows 10
    if (FAILED(hr))
        return nullptr;

    win::ComPtr<UnityWinRTBase::IStorageFileStatics> storageFileStatics;
    hr = UnityWinRTBase::RoGetActivationFactory(UnityWinRTBase::HStringReference(L"Windows.Storage.StorageFile"),
        __uuidof(UnityWinRTBase::IStorageFileStatics), &storageFileStatics);
    VerifyHR(hr, "Failed get activation factory of Windows.Storage.StorageFile (hr = 0x%X)");

    const wchar_t* path = reinterpret_cast<const wchar_t*>(grammarFilePath.GetRawCharBuffer());
    size_t length = wcslen(path) + 1;
    dynamic_array<wchar_t> normalizedPath(length, kMemTempAlloc);

    for (size_t i = 0; i < length; i++)
        normalizedPath[i] = path[i] == '/' ? '\\' : path[i];

    win::ComPtr<UnityWinRTBase::IAsyncOperation<UnityWinRTBase::StorageFile*> > gettingStorageFileOperation;
    hr = storageFileStatics->GetFileFromPathAsync(UnityWinRTBase::HStringReference(normalizedPath.data(), length - 1), &gettingStorageFileOperation);
    VerifyHR(hr, "Failed to start getting storage file from path (hr = 0x%X)");

    win::ComPtr<UnityWinRTBase::IStorageFile> grammarStorageFile;
    hr = UnityWinRTBase::SynchronousOperation<UnityWinRTBase::StorageFile*>::Wait(kMemSpeech, gettingStorageFileOperation, &grammarStorageFile);
    VerifyHR(hr, "Failed to get storage file from path (hr = 0x%X)");

    win::ComPtr<UnityWinRTBase::ISpeechRecognitionGrammarFileConstraint> grammarFileConstraint;
    hr = grammarFileConstraintFactory->Create(grammarStorageFile, &grammarFileConstraint);
    VerifyHR(hr, "Failed to create SpeechRecognitionGrammarFileConstraint (hr = 0x%X)");

    win::ComPtr<UnityWinRTBase::ISpeechRecognitionConstraint> constraint;
    hr = grammarFileConstraint.As(&constraint);
    VerifyHR(hr, "Failed to cast ISpeechRecognitionListConstraint to ISpeechRecognitionConstraint (hr = 0x%X)");

    hr = constraint->put_IsEnabled(false);
    VerifyHR(hr, "Failed to disable ISpeechRecognitionConstraint for initial creation (hr = 0x%X)");

    VerifyDuplicateGrammarFiles(grammarFilePath);

    return UNITY_NEW(PhraseRecognizer, kMemSpeech)(self, minimumConfidence, constraint);
}

PhraseRecognizer::PhraseRecognizer(ScriptingObjectPtr managedCounterpart, ConfidenceLevel minimumConfidence, UnityWinRTBase::ISpeechRecognitionConstraint* constraint) :
    m_MinimumConfidence(minimumConfidence),
    m_Constraint(constraint),
    m_IsRunning(false),
    m_ManagedCounterpart(managedCounterpart, GCHANDLE_WEAK)
#if UNITY_EDITOR
    , m_IsQueuedForDestruction(false)
#endif
{
    PhraseRecognitionSystem::GetInstance().AddPhraseRecognizer(this);

    static UnityEventQueue::StaticFunctionEventHandler<PhraseRecognizerDestructionBuffer> s_DestroyEventDelegate(&PhraseRecognizer::HandleEvent);
    static bool s_IsDestroyEventDelegateRegistered = false;

    if (!s_IsDestroyEventDelegateRegistered)
    {
        s_IsDestroyEventDelegateRegistered = true;
        GlobalEventQueue::GetInstance().AddHandler(&s_DestroyEventDelegate);
    }
}

void PhraseRecognizer::Start(ScriptingExceptionPtr* exception)
{
    if (m_IsRunning)
    {
        WarningString("Warning: PhraseRecognizer.Start() was called when PhraseRecognizer was already running.");
        return;
    }

    HRESULT hr = m_Constraint->put_IsEnabled(true);
    if (FAILED(hr))
    {
        ErrorStringMsg("Failed to enable PhraseRecognizer constraint (hr = 0x%X)", hr);
        return;
    }

    m_IsRunning = true;
    PhraseRecognitionSystem::GetInstance().IncrementEnabledRecognizerCount(exception);
    if (*exception != SCRIPTING_NULL)
        return;
}

void PhraseRecognizer::Stop()
{
    if (!m_IsRunning)
    {
        WarningString("Warning: PhraseRecognizer.Stop() was called when PhraseRecognizer was not running.");
        return;
    }

    HRESULT hr = m_Constraint->put_IsEnabled(false);
    if (FAILED(hr))
    {
        ErrorStringMsg("Failed to disable PhraseRecognizer constraint (hr = 0x%X)", hr);
        return;
    }

    m_IsRunning = false;
    PhraseRecognitionSystem::GetInstance().DecrementEnabledRecognizerCount();
}

static inline ScriptingObjectPtr MarshalSemanticMeaning(UnityWinRTBase::ISpeechRecognitionSemanticInterpretation* semanticInterpretation)
{
    win::ComPtr<UnityWinRTBase::IMapView<UnityWinRTBase::HSTRING, UnityWinRTBase::IVectorView<UnityWinRTBase::HSTRING>*> > semanticInterpretationProperties;
    HRESULT hr = semanticInterpretation->get_Properties(&semanticInterpretationProperties);
    AssertFormatMsg(SUCCEEDED(hr), "Failed to get properties from ISpeechRecognitionSemanticInterpretation (hr = 0x%X)", hr);
    if (FAILED(hr))
        return SCRIPTING_NULL;

    uint32_t size;
    semanticInterpretationProperties->get_Size(&size);
    AssertFormatMsg(SUCCEEDED(hr), "Failed to get semantic interpretation properties size (hr = 0x%X)", hr);
    if (FAILED(hr) || size == 0)
        return SCRIPTING_NULL;

    win::ComPtr<UnityWinRTBase::IIterable<UnityWinRTBase::IKeyValuePair<UnityWinRTBase::HSTRING, UnityWinRTBase::IVectorView<UnityWinRTBase::HSTRING>*>*> > propertiesIterable;
    hr = semanticInterpretationProperties.As(&propertiesIterable);
    AssertFormatMsg(SUCCEEDED(hr), "Failed to cast semantic interpretation to IIterable (hr = 0x%X)", hr);
    if (FAILED(hr))
        return SCRIPTING_NULL;

    win::ComPtr<UnityWinRTBase::IIterator<UnityWinRTBase::IKeyValuePair<UnityWinRTBase::HSTRING, UnityWinRTBase::IVectorView<UnityWinRTBase::HSTRING>*>*> > propertiesIterator;
    hr = propertiesIterable->First(&propertiesIterator);
    AssertFormatMsg(SUCCEEDED(hr), "Failed to get IIterator from semantic interpretation IIterable (hr = 0x%X)", hr);
    if (FAILED(hr))
        return SCRIPTING_NULL;

    dynamic_array<const wchar_t*> keys(kMemTempAlloc);
    dynamic_array<const wchar_t*> values(kMemTempAlloc);
    dynamic_array<UnityWinRTBase::HSTRING> hstringKeys(kMemTempAlloc);
    dynamic_array<UnityWinRTBase::HSTRING> hstringValues(kMemTempAlloc);
    dynamic_array<uint32_t> valueSizes(kMemTempAlloc);

    keys.reserve(size);
    hstringKeys.reserve(size);
    valueSizes.reserve(size);

    boolean hasCurrent;
    hr = propertiesIterator->get_HasCurrent(&hasCurrent);
    AssertFormatMsg(SUCCEEDED(hr), "Failed to find out whether semantic interpretation iterator has any values (hr = 0x%X)", hr);
    AssertFormatMsg(hasCurrent, "Error: semantic interpretation has more than 0 values, but its iterator has no values (hr = 0x%X)", hr);
    if (FAILED(hr) || !hasCurrent)
        return SCRIPTING_NULL;

    do
    {
        win::ComPtr<UnityWinRTBase::IKeyValuePair<UnityWinRTBase::HSTRING, UnityWinRTBase::IVectorView<UnityWinRTBase::HSTRING>*> > keyValuePair;
        hr = propertiesIterator->get_Current(&keyValuePair);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to get current value out of semantic interpretation iterator (hr = 0x%X)", hr);
        if (FAILED(hr))
            break;

        UnityWinRTBase::HString key;
        win::ComPtr<UnityWinRTBase::IVectorView<UnityWinRTBase::HSTRING> > valueVector;

        hr = keyValuePair->get_Key(&key);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to get key of semantic interpretation KeyValuePair (hr = 0x%X)", hr);
        if (FAILED(hr))
            break;

        hr = keyValuePair->get_Value(&valueVector);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to get values of semantic interpretation KeyValuePair (hr = 0x%X)", hr);
        if (FAILED(hr))
            break;

        uint32_t valuesSize;
        hr = valueVector->get_Size(&valuesSize);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to get size of semantic interpretation values vector (hr = 0x%X)", hr);
        if (FAILED(hr))
            break;

        size_t oldHstringValuesSize = hstringValues.size();
        hstringValues.resize_uninitialized(oldHstringValuesSize + valuesSize);

        uint32_t actualValuesSize;
        hr = valueVector->GetMany(0, valuesSize, hstringValues.begin() + oldHstringValuesSize, &actualValuesSize);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to get values of semantic interpretation values vector (hr = 0x%X)", hr);
        Assert(actualValuesSize == valuesSize);
        if (FAILED(hr))
            break;

        keys.push_back(key.GetBuffer());
        hstringKeys.push_back(key.Detach());
        valueSizes.push_back(actualValuesSize);

        hr = propertiesIterator->MoveNext(&hasCurrent);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to find out whether semantic interpretation iterator has any values (hr = 0x%X)", hr);
        if (FAILED(hr))
            break;
    }
    while (hasCurrent);

    Assert(size == keys.size());
    ScriptingObjectPtr managedSemanticMeaning = SCRIPTING_NULL;

    if (keys.size() != 0)
    {
        values.resize_uninitialized(hstringValues.size());
        for (size_t i = 0; i < hstringValues.size(); i++)
        {
            uint32_t strLength;
            values[i] = UnityWinRTBase::WindowsGetStringRawBuffer(hstringValues[i], &strLength);
        }

        ScriptingInvocation invocation(GetCoreScriptingClasses().marshalSemanticMeaning);
        invocation.AddIntPtr(keys.data());
        invocation.AddIntPtr(values.data());
        invocation.AddIntPtr(valueSizes.data());
        invocation.AddInt(static_cast<int>(keys.size()));

        managedSemanticMeaning = invocation.Invoke();
    }

    for (size_t i = 0; i < hstringKeys.size(); i++)
        UnityWinRTBase::WindowsDeleteString(hstringKeys[i]);

    for (size_t i = 0; i < hstringValues.size(); i++)
        UnityWinRTBase::WindowsDeleteString(hstringValues[i]);

    return managedSemanticMeaning;
}

bool PhraseRecognizer::OnPhraseRecognized(UnityWinRTBase::ISpeechRecognitionResult* recognitionResult)
{
    UnityWinRTBase::HString text;
    UnityWinRTBase::SpeechRecognitionConfidence confidence;
    UnityWinRTBase::DateTime phraseStartTime;
    UnityWinRTBase::TimeSpan phraseDuration;
    win::ComPtr<UnityWinRTBase::ISpeechRecognitionSemanticInterpretation> semanticInterpretation;

    auto hr = recognitionResult->get_Confidence(&confidence);
    if (FAILED(hr))
    {
        ErrorStringMsg("Failed to get confidence from ISpeechRecognitionResult (hr = 0x%X)", hr);
        return false;
    }

    if (static_cast<int>(confidence) > m_MinimumConfidence)
        return false;

    hr = recognitionResult->get_Text(&text);
    if (FAILED(hr))
    {
        ErrorStringMsg("Failed to get text from ISpeechRecognitionResult (hr = 0x%X)", hr);
        return false;
    }

    hr = recognitionResult->get_SemanticInterpretation(&semanticInterpretation);
    if (FAILED(hr))
    {
        ErrorStringMsg("Failed to get semantic interpretation from ISpeechRecognitionResult (hr = 0x%X)", hr);
        return false;
    }

    win::ComPtr<UnityWinRTBase::ISpeechRecognitionResult2> recognitionResult2;
    hr = recognitionResult->QueryInterface(__uuidof(UnityWinRTBase::ISpeechRecognitionResult2), &recognitionResult2);
    if (FAILED(hr))
    {
        ErrorStringMsg("Failed to cast ISpeechRecognitionResult to ISpeechRecognitionResult2 (hr = 0x%X)", hr);
        return false;
    }

    hr = recognitionResult2->get_PhraseStartTime(&phraseStartTime);
    if (FAILED(hr))
    {
        ErrorStringMsg("Failed to get phrase start time from ISpeechRecognitionResult2 (hr = 0x%X)", hr);
        return false;
    }

    hr = recognitionResult2->get_PhraseDuration(&phraseDuration);
    if (FAILED(hr))
    {
        ErrorStringMsg("Failed to get phrase duration from ISpeechRecognitionResult2 (hr = 0x%X)", hr);
        return false;
    }

    return InvokePhraseRecognizedEvent(scripting_string_new(text.GetBuffer()), confidence, MarshalSemanticMeaning(semanticInterpretation),
        phraseStartTime.UniversalTime, phraseDuration.Duration);
}

bool PhraseRecognizer::OnPhraseRecognized(const wchar_t* text, int confidence)
{
    if (confidence > m_MinimumConfidence)
        return false;

    return InvokePhraseRecognizedEvent(scripting_string_new(text), confidence, (ScriptingObjectPtr)SCRIPTING_NULL, 0, 0);
}

bool PhraseRecognizer::InvokePhraseRecognizedEvent(ScriptingStringPtr text, int confidence, ScriptingObjectPtr semanticMeanings, int64_t phraseStartTime, int64_t phraseDuration)
{
    if (!m_ManagedCounterpart.HasTarget())
        return false;

    ScriptingObjectPtr managedObject = m_ManagedCounterpart.Resolve();
    DebugAssert(managedObject != SCRIPTING_NULL);

    ScriptingInvocation invocation(managedObject, GetCoreScriptingClasses().invokePhraseRecognizedEvent);
    invocation.AddString(text);
    invocation.AddInt(confidence);
    invocation.AddObject(semanticMeanings);
    invocation.AddInt64(phraseStartTime);
    invocation.AddInt64(phraseDuration);

    invocation.Invoke();

    return true;
}

void PhraseRecognizer::Destroy()
{
    if (m_IsRunning)
    {
        m_IsRunning = false;
        PhraseRecognitionSystem::GetInstance().DecrementEnabledRecognizerCount();
    }

    PhraseRecognitionSystem::GetInstance().RemovePhraseRecognizer(this);

    // EVENTS CAN NO LONGER FIRE FROM THIS POINT ONWARD

    m_ManagedCounterpart.ReleaseAndClear();

    PhraseRecognizer* me = this;
    UNITY_DELETE(me, kMemSpeech);
}

// DestroyThreaded can be called from any thread, potentially finalizer thread
// We need to take a gchandle to the managed object to make sure it survives
// until we can unsubscribe from Recognized event (as we can only unsubscribe from the main thread)
void PhraseRecognizer::DestroyThreaded()
{
    ScriptingGCHandle strongGCHandle;

    if (m_ManagedCounterpart.HasTarget())
    {
        ScriptingObjectPtr managedObject = m_ManagedCounterpart.Resolve();
        strongGCHandle.AcquireStrong(managedObject);
    }

#if UNITY_EDITOR
    m_IsQueuedForDestruction = true;
#endif
    PhraseRecognizerDestructionBuffer buffer = { this, ScriptingGCHandle::ToScriptingBackendNativeGCHandle(strongGCHandle) };
    GlobalEventQueue::GetInstance().SendEvent(buffer);
}

void PhraseRecognizer::HandleEvent(const PhraseRecognizerDestructionBuffer& buffer)
{
    buffer.phraseRecognizer->Destroy();
}

#if UNITY_EDITOR
void PhraseRecognizer::OnBeforeDomainUnload()
{
    // We cannot fire events on managed objects anymore after domain unloading starts
    // But we still cannot destroy this object yet because finalizers might be invoked
    // after domain unload starts.
    m_ManagedCounterpart.ReleaseAndClear();
}

#endif


#endif // PLATFORM_WIN
