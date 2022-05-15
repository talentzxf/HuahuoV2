#pragma once

#include "External/Windows10/src/ComHelpers.h"
#include "Runtime/Graphics/Texture.h"

namespace Unity
{
    class PhotoCapture;

    struct PhotoCaptureOperationParameters
    {
        ScriptingGCHandle callbackGCHandle;
        core::wstring * filepath;
        bool saveToDisk;
        int fileOutputFormat;

        PhotoCaptureOperationParameters() : callbackGCHandle(),
            filepath(NULL),
            saveToDisk(false),
            fileOutputFormat(0)
        {
        }

        ~PhotoCaptureOperationParameters()
        {
            callbackGCHandle.ReleaseAndClear();

            if (filepath != NULL)
            {
                UNITY_DELETE(filepath, kMemWebCam);
                filepath = NULL;
            }
        }
    };

    class PhotoCaptureOperation :
        private UnityWinRTBase::ComClass<UnityWinRTBase::Windows::Foundation::IAsyncOperationCompletedHandler<UnityWinRTBase::Windows::Media::Capture::CapturedPhoto*> >
    {
    public:
        static void Execute(PhotoCapture* photoCaptureObject, UnityWinRTBase::Windows::Media::Capture::ILowLagPhotoCapture* lowLagPhotoCapture, PhotoCaptureOperationParameters *photoCaptureOperationParams);

    private:
        win::ComPtr<PhotoCapture> m_PhotoCaptureObject;
        PhotoCaptureOperationParameters * m_PhotoCaptureParams;

        PhotoCaptureOperation(PhotoCapture* photoCaptureObject, PhotoCaptureOperationParameters *photoCaptureOperationParams);
        void OnPhotoCaptured(UnityWinRTBase::Windows::Foundation::IAsyncOperation<UnityWinRTBase::Windows::Media::Capture::CapturedPhoto*>* asyncOperation);

        virtual HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::Windows::Foundation::IAsyncOperation<UnityWinRTBase::Windows::Media::Capture::CapturedPhoto*>* asyncInfo, UnityWinRTBase::Windows::Foundation::AsyncStatus status) override;

        friend class win::ComPtr<PhotoCaptureOperation>;
    };
}
