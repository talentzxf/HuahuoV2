#pragma once

#if PLATFORM_WIN && !PLATFORM_XBOXONE
#include "External/Windows10/src/ComHelpers.h"
#include "External/Windows10/src/Windows10Interfaces.h"
#include "External/Windows10/src/WinRTCollections.h"
#include "PlatformDependent/Win/ComPtr.h"
#endif

namespace Unity
{
#if PLATFORM_WIN && !PLATFORM_XBOXONE
    class MixedRealityCaptureVideoEffect :
        public UnityWinRTBase::InspectableClass<UnityWinRTBase::Windows::Media::Effects::IVideoEffectDefinition>
    {
        win::ComPtr<PropertySet<kMemWebCamId> > m_PropertySet;

    public:
        MixedRealityCaptureVideoEffect(UnityWinRTBase::Windows::Media::Capture::MediaStreamType streamType, float hologramOpacity);

        virtual HRESULT STDMETHODCALLTYPE get_ActivatableClassId(UnityWinRTBase::HSTRING* value) override;
        virtual HRESULT STDMETHODCALLTYPE get_Properties(UnityWinRTBase::Windows::Foundation::Collections::IPropertySet** value) override;
    };
#endif
}
