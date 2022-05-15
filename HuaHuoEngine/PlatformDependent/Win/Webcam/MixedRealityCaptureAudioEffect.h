#pragma once

#if PLATFORM_WIN && !PLATFORM_XBOXONE
#include "External/Windows10/src/ComHelpers.h"
#include "External/Windows10/src/Windows10Interfaces.h"
#include "External/Windows10/src/WinRTCollections.h"
#include "PlatformDependent/Win/ComPtr.h"
#endif

#include "Runtime/Scripting/BindingsDefs.h"
#include "Runtime/Utilities/ReflectableEnum.h"

namespace Unity
{
    REFLECTABLE_ENUM(AudioMixerMode,
        Mic,
        Loopback,
        MicAndLoopback,
        None
    );

#if PLATFORM_WIN && !PLATFORM_XBOXONE
    class MixedRealityCaptureAudioEffect :
        public UnityWinRTBase::InspectableClass<UnityWinRTBase::Windows::Media::Effects::IAudioEffectDefinition>
    {
    public:

    private:
        win::ComPtr<PropertySet<kMemWebCamId> > m_PropertySet;
        AudioMixerMode m_AudioMixerMode;

    public:
        MixedRealityCaptureAudioEffect(AudioMixerMode audioMixerMode);

        virtual HRESULT STDMETHODCALLTYPE get_ActivatableClassId(UnityWinRTBase::HSTRING* value) override;
        virtual HRESULT STDMETHODCALLTYPE get_Properties(UnityWinRTBase::Windows::Foundation::Collections::IPropertySet** value) override;
    };
#endif
}

BIND_MANAGED_TYPE_NAME(::Unity::AudioMixerMode::ActualEnumType, UnityEngine_Windows_WebCam_VideoCapture_AudioState);
