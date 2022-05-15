#include "UnityPrefix.h"

#include "MixedRealityCaptureAudioEffect.h"

namespace UnityWinRTBase
{
    using namespace UnityWinRTBase::Windows::Foundation;
    using namespace UnityWinRTBase::Windows::Media::Capture;
}

namespace Unity
{
    MixedRealityCaptureAudioEffect::MixedRealityCaptureAudioEffect(AudioMixerMode audioMixerMode)
        : m_AudioMixerMode(audioMixerMode)
    {
        SetLabel(kMemWebCam);

        m_PropertySet.Attach(UNITY_NEW(PropertySet<kMemWebCamId>, kMemWebCam));

        win::ComPtr<UnityWinRTBase::IPropertyValueStatics> propertyValueStatics;
        HRESULT hr = UnityWinRTBase::RoGetActivationFactory(UnityWinRTBase::HStringReference(L"Windows.Foundation.PropertyValue"),
            __uuidof(UnityWinRTBase::IPropertyValueStatics), &propertyValueStatics);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to get activation factory for Windows.Foundation.PropertyValue! [0x%x]", hr);

        win::ComPtr<IInspectable> boxedMixerMode;

        hr = propertyValueStatics->CreateUInt32(static_cast<uint32_t>(m_AudioMixerMode), &boxedMixerMode);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to CreateUInt32 on IPropertyValueStatics! [0x%x]", hr);

        m_PropertySet->Insert(UnityWinRTBase::HStringReference(L"MixerMode"), boxedMixerMode);
    }

    HRESULT STDMETHODCALLTYPE MixedRealityCaptureAudioEffect::get_ActivatableClassId(UnityWinRTBase::HSTRING* value)
    {
        *value = UnityWinRTBase::HString(L"Windows.Media.MixedRealityCapture.MixedRealityCaptureAudioEffect").Detach();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE MixedRealityCaptureAudioEffect::get_Properties(UnityWinRTBase::Windows::Foundation::Collections::IPropertySet** value)
    {
        *value = m_PropertySet;
        m_PropertySet->AddRef();
        return S_OK;
    }
}
