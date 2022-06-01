#include "UnityPrefix.h"

#include "PhotoCaptureOperation.h"
#include "PhotoCapture.h"
#include <mfidl.h>

namespace UnityWinRTBase
{
    using namespace UnityWinRTBase::Windows::Foundation;
    using namespace UnityWinRTBase::Windows::Media::Capture;
    using namespace UnityWinRTBase::Windows::Media::MediaProperties;
}

#define VerifyHR(hr, message) \
    do \
    { \
        if (FAILED(hr)) \
        { \
            AssertFormatMsg(false, message, hr); \
            return; \
        } \
    } \
    while (false)

#define VerifyHRAndNotifyScript(photoCaptureInstance, photoCaptureParams, hr) \
    do \
    { \
        if (FAILED(hr)) \
        {\
            if(photoCaptureParams->saveToDisk)\
            {\
                photoCaptureInstance->NotifyScriptOfErrorOnMainThread(PhotoCapture::OperationType::TakePhotoAsyncToDisk, hr, photoCaptureParams->callbackGCHandle);\
            }\
            else\
            {\
                photoCaptureInstance->NotifyScriptOfErrorOnMainThread(PhotoCapture::OperationType::TakePhotoAsyncToMemory, hr, photoCaptureParams->callbackGCHandle);\
            }\
            photoCaptureInstance->m_HasErrors = true; \
        }\
    } \
    while (false)


namespace Unity
{
    const GUID kMfWrappedSampleService = { 0x31f52bf2, 0xd03e, 0x4048, { 0x80, 0xd0, 0x9c, 0x10, 0x46, 0xd8, 0x7c, 0x61 } };

    void PhotoCaptureOperation::Execute(PhotoCapture* photoCaptureObject, UnityWinRTBase::ILowLagPhotoCapture* lowLagPhotoCapture, PhotoCaptureOperationParameters *photoCaptureOperationParams)
    {
        win::ComPtr<PhotoCaptureOperation> operation;
        operation.Attach(UNITY_NEW(PhotoCaptureOperation, kMemWebCam)(photoCaptureObject, photoCaptureOperationParams));

        win::ComPtr<UnityWinRTBase::IAsyncOperation<UnityWinRTBase::CapturedPhoto*> > captureOperation;
        HRESULT hr = lowLagPhotoCapture->CaptureAsync(&captureOperation);
        VerifyHRAndNotifyScript(photoCaptureObject, photoCaptureOperationParams, hr);

        hr = captureOperation->put_Completed(operation);
        VerifyHRAndNotifyScript(photoCaptureObject, photoCaptureOperationParams, hr);
    }

    PhotoCaptureOperation::PhotoCaptureOperation(PhotoCapture* photoCaptureObject, PhotoCaptureOperationParameters *photoCaptureOperationParams) :
        m_PhotoCaptureObject(photoCaptureObject),
        m_PhotoCaptureParams(photoCaptureOperationParams)
    {
    }

    void PhotoCaptureOperation::OnPhotoCaptured(UnityWinRTBase::IAsyncOperation<UnityWinRTBase::CapturedPhoto*>* asyncOperation)
    {
        win::ComPtr<UnityWinRTBase::ICapturedPhoto> capturedPhoto;
        HRESULT hr = asyncOperation->GetResults(&capturedPhoto);
        if (m_PhotoCaptureObject != NULL)
        {
            VerifyHRAndNotifyScript(m_PhotoCaptureObject, m_PhotoCaptureParams, hr);
        }
        else
        {
            VerifyHR(hr, "Error: failed to retrieve data from captured photo (hr = 0x%X)");
        }

        win::ComPtr<IMFGetService> mfGetService;
        hr = capturedPhoto->QueryInterface(__uuidof(IMFGetService), &mfGetService);
        if (m_PhotoCaptureObject != NULL)
        {
            VerifyHRAndNotifyScript(m_PhotoCaptureObject, m_PhotoCaptureParams, hr);
        }
        else
        {
            VerifyHR(hr, "Error: failed to cast ICapturedPhoto to IMFGetService");
        }

        win::ComPtr<IMFSample> mfSample;
        hr = mfGetService->GetService(kMfWrappedSampleService, __uuidof(IMFSample), &mfSample);
        if (m_PhotoCaptureObject != NULL)
        {
            VerifyHRAndNotifyScript(m_PhotoCaptureObject, m_PhotoCaptureParams, hr);
        }
        else
        {
            VerifyHR(hr, "Error: failed to get IMFSample from IMFGetService");
        }

        m_PhotoCaptureObject->OnPhotoCapturedToMemory(mfSample, m_PhotoCaptureParams);
    }

    HRESULT STDMETHODCALLTYPE PhotoCaptureOperation::Invoke(UnityWinRTBase::IAsyncOperation<UnityWinRTBase::CapturedPhoto*>* asyncOperation, UnityWinRTBase::AsyncStatus status)
    {
        if (status == UnityWinRTBase::Completed)
        {
            OnPhotoCaptured(asyncOperation);
            return S_OK;
        }

        ErrorStringMsg("Failed capturing photo (hr = 0x%X)", UnityWinRTBase::GetAsyncOperationErrorCode(asyncOperation));
        return S_OK;
    }

#undef VerifyHRAndNotifyScript
}
