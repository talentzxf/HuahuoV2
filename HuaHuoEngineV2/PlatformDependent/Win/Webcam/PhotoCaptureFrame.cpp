#include "UnityPrefix.h"

#include "PhotoCaptureFrame.h"
#include "Runtime/Graphics/TextureFormat.h"
#include "Runtime/GfxDevice/GfxDevice.h"

namespace Unity
{
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

#define VerifyHRWithReturnValue(hr, message, returnValue) \
    do \
    { \
        if (FAILED(hr)) \
        { \
            AssertFormatMsg(false, message, hr); \
            return returnValue; \
        } \
    } \
    while (false)

    PhotoCaptureFrame::PhotoCaptureFrame(const Matrix4x4f& cameraToWorld, const Matrix4x4f& projection, IMFSample* sample, IMFMediaBuffer* mfBuffer, int width, int height, CapturePixelFormat format, bool hasLocationData) :
        m_CameraToWorld(cameraToWorld),
        m_Projection(projection),
        m_Width(width),
        m_Height(height),
        m_Format(format),
        m_MFSample(sample),
        m_MFBuffer(mfBuffer),
        m_Length(0),
        m_HasLocationData(hasLocationData)
    {
        HRESULT hr = mfBuffer->GetCurrentLength(reinterpret_cast<DWORD*>(&m_Length));
        VerifyHR(hr, "Failed to get length of IMFBuffer (hr = 0x%X)");
    }

    void PhotoCaptureFrame::CopyDataIntoBuffer(void* targetBuffer) const
    {
        uint8_t* source;
        DWORD maxLength, currentLength;

        HRESULT hr = m_MFBuffer->Lock(&source, &maxLength, &currentLength);
        VerifyHR(hr, "Failed to lock IMFBuffer in order to copy data out of it (hr = 0x%X)");

        memcpy(targetBuffer, source, m_Length);

        hr = m_MFBuffer->Unlock();
        AssertMsg(SUCCEEDED(hr), "Failed to unlock the photo capture frame image buffer.");
    }

    int PhotoCaptureFrame::CopyRawImageDataIntoBuffer(dynamic_array<UInt8>& byteArray)
    {
        uint8_t* source;
        DWORD maxLength, currentLength;

        HRESULT hr = m_MFBuffer->Lock(&source, &maxLength, &currentLength);
        VerifyHRWithReturnValue(hr, "Failed to lock IMFBuffer in order to copy data out of it (hr = 0x%X)", 0);

        memcpy(byteArray.data(), source, m_Length);

        hr = m_MFBuffer->Unlock();
        AssertMsg(SUCCEEDED(hr), "Can not copy raw image data into buffer due to a failure in unlocking the photo capture frame image buffer.");

        return currentLength;
    }

    void PhotoCaptureFrame::UploadImageDataToTexture(Texture2D* targetTexture)
    {
        Assert(m_Format == CapturePixelFormat::BGRA32);

        uint8_t* source;
        DWORD maxLength, currentLength;

        HRESULT hr = m_MFBuffer->Lock(&source, &maxLength, &currentLength);
        VerifyHR(hr, "Failed to lock IMFBuffer in order to copy data out of it (hr = 0x%X)");

        ImageReference targetImage;
        if (!targetTexture->GetWriteImageReference(&targetImage, 0, 0))
        {
            ErrorStringMsg("Error: specified texture is not writeable.");
            return;
        }

        ImageReference inputImage(m_Width, m_Height, GetRowSize(m_Width, kFormatB8G8R8A8_UNorm), kFormatB8G8R8A8_UNorm, source);
        targetImage.BlitImage(inputImage, kImageBlitBilinearScale);
        targetImage.FlipImageY();

        hr = m_MFBuffer->Unlock();
        AssertMsg(SUCCEEDED(hr), "Can not upload image data into a texture due to a failure in unlocking the photo capture frame image buffer.");

        targetTexture->Apply(true, false);
    }
} // end namespace Unity
