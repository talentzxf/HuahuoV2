#include "UnityPrefix.h"

#include "MixedRealityCaptureVideoEffect.h"
#include "windows.foundation.h"

namespace UnityWinRTBase
{
    using namespace UnityWinRTBase::Windows::Foundation;
    using namespace UnityWinRTBase::Windows::Media::Capture;
}

namespace Unity
{
    MixedRealityCaptureVideoEffect::MixedRealityCaptureVideoEffect(UnityWinRTBase::Windows::Media::Capture::MediaStreamType streamType, float hologramOpacity)
    {
        SetLabel(kMemWebCam);

        m_PropertySet.Attach(UNITY_NEW(PropertySet<kMemWebCamId>, kMemWebCam));

        win::ComPtr<UnityWinRTBase::IPropertyValueStatics> propertyValueStatics;
        HRESULT hr = UnityWinRTBase::RoGetActivationFactory(UnityWinRTBase::HStringReference(L"Windows.Foundation.PropertyValue"),
            __uuidof(UnityWinRTBase::IPropertyValueStatics), &propertyValueStatics);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to get activation factory for Windows.Foundation.PropertyValue! [0x%x]", hr);

        win::ComPtr<IInspectable> boxedStreamType, hologramComposition, videoStabilization, videoStabilizationBufferLength, hologramOpacityProperty, recordingIndicatorEnabledProperty;

        hr = propertyValueStatics->CreateUInt32(static_cast<uint32_t>(streamType), &boxedStreamType);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to CreateUnit32 for boxed stream type on property value statics! [0x%x]", hr);

        hr = propertyValueStatics->CreateBoolean(true, &hologramComposition);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to CreateBoolean for hologram composition on property value statics! [0x%x]", hr);

        // TODO Microsoft would eventually like this exposed.  For now default to false for better perf
        hr = propertyValueStatics->CreateBoolean(false, &videoStabilization);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to CreateBoolean for video stabilization on property value statics! [0x%x]", hr);

        hr = propertyValueStatics->CreateInt32(30, &videoStabilizationBufferLength);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to createInt32 for video stabilization buffer length on property value statics! [0x%x]", hr);

        hr = propertyValueStatics->CreateSingle(hologramOpacity, &hologramOpacityProperty);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to CreateSingle for holograph opacity on property value statics! [0x%x]", hr);

        hr = propertyValueStatics->CreateBoolean(true, &recordingIndicatorEnabledProperty);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to CreateBoolean for recording indicator enabled on property value statics! [0x%x]", hr);

        m_PropertySet->Insert(UnityWinRTBase::HStringReference(L"StreamType"), boxedStreamType);
        m_PropertySet->Insert(UnityWinRTBase::HStringReference(L"HologramCompositionEnabled"), hologramComposition);
        m_PropertySet->Insert(UnityWinRTBase::HStringReference(L"VideoStabilizationEnabled"), videoStabilization);
        m_PropertySet->Insert(UnityWinRTBase::HStringReference(L"VideoStabilizationBufferLength"), videoStabilizationBufferLength);
        m_PropertySet->Insert(UnityWinRTBase::HStringReference(L"GlobalOpacityCoefficient"), hologramOpacityProperty);
        m_PropertySet->Insert(UnityWinRTBase::HStringReference(L"RecordingIndicatorEnabled"), recordingIndicatorEnabledProperty);
    }

    HRESULT STDMETHODCALLTYPE MixedRealityCaptureVideoEffect::get_ActivatableClassId(UnityWinRTBase::HSTRING* value)
    {
        *value = UnityWinRTBase::HString(L"Windows.Media.MixedRealityCapture.MixedRealityCaptureVideoEffect").Detach();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE MixedRealityCaptureVideoEffect::get_Properties(UnityWinRTBase::Windows::Foundation::Collections::IPropertySet** value)
    {
        *value = m_PropertySet;
        m_PropertySet->AddRef();
        return S_OK;
    }
}
