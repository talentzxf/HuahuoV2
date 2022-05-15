#include "UnityPrefix.h"

#include "IMFSampleImageFileWriter.h"
#include "External/Windows10/src/WinRTFunctions.h"
#include "PlatformDependent/Win/ComPtr.h"
#include "PlatformDependent/Win/WinUnicode.h"

#include <mfidl.h>
#include <mfapi.h>
#include <wincodec.h>
#include <wincodecsdk.h>

#include "External/Windows10/src/mfapi_ext.h"
#include "External/Windows10/src/wincodec_ext.h"

#define VerifyHR(hr) \
do \
{ \
    if (FAILED(hr)) \
    { \
        return hr; \
    } \
} \
while (false)


namespace Unity
{
    inline long SwapLong(const long lVal)
    {
        return  ((((lVal) >> 24) & 0x000000FFL) |
            (((lVal) >> 8) & 0x0000FF00L)   |
            (((lVal) << 8) & 0x00FF0000L)   |
            (((lVal) << 24) & 0xFF000000L));
    }

    HRESULT IMFSampleImageFileWriter::ExifIsLittleEndian(IWICStream* pExifBlockStream, bool* pIsLittleEndian)
    {
        HRESULT hr = S_OK;
        byte buffer[2];
        ULONG bytesRead;

        hr = pExifBlockStream->Read(buffer, sizeof(buffer), &bytesRead);
        VerifyHR(hr);

        if (('I' == buffer[0]) && ('I' == buffer[1]))
        {
            *pIsLittleEndian = true;
        }
        else if (('M' == buffer[0]) && ('M' == buffer[1]))
        {
            *pIsLittleEndian = false;
        }
        else
        {
            return E_FAIL;
        }

        return hr;
    }

    HRESULT IMFSampleImageFileWriter::SeekExifToIFD0Start(IWICStream* pExifBlockStream, bool isLittleEndian)
    {
        HRESULT hr = S_OK;
        LARGE_INTEGER seekPos;
        ULONG ifdZeroOffset;
        ULONG bytesRead;

        // http://www.exif.org/Exif2-2.PDF - first, skip the 2-byte-order  marker & the fixed 0x002A value. Next, read offset of the IFD.
        seekPos.QuadPart = 4;
        hr = pExifBlockStream->Seek(seekPos, STREAM_SEEK_SET, nullptr);
        VerifyHR(hr);
        hr = pExifBlockStream->Read(&ifdZeroOffset, sizeof(ifdZeroOffset), &bytesRead);
        VerifyHR(hr);

        if (!isLittleEndian)
        {
            ifdZeroOffset = SwapLong(ifdZeroOffset);
        }

        seekPos.LowPart = ifdZeroOffset;
        seekPos.HighPart = 0;
        hr = pExifBlockStream->Seek(seekPos, STREAM_SEEK_SET, nullptr);
        VerifyHR(hr);

        return S_OK;
    }

    HRESULT IMFSampleImageFileWriter::GetMetaData(IMFSample *sample, BYTE** spExifMetadata, UINT32* blobSize)
    {
        win::ComPtr<IMFAttributes> spCaptureMetadata;
        HRESULT hr = sample->GetUnknown(MFSampleExtension_CaptureMetadata, __uuidof(IMFAttributes), &spCaptureMetadata);
        VerifyHR(hr);

        hr = spCaptureMetadata->GetBlobSize(MF_CAPTURE_METADATA_EXIF, blobSize);
        VerifyHR(hr);

        if (blobSize > 0)
        {
            UINT32 returnedBlobSize = 0;
            hr = spCaptureMetadata->GetAllocatedBlob(MF_CAPTURE_METADATA_EXIF, spExifMetadata, &returnedBlobSize);
            VerifyHR(hr);
        }

        return S_OK;
    }

    HRESULT IMFSampleImageFileWriter::SetupExifReader(IWICStream* pExifBlockStream, IWICComponentFactory *pComponentFactory, IWICMetadataReader** ppMetadataReader)
    {
        HRESULT hr = S_OK;
        bool isLittleEndian;
        DWORD persistOption;

        hr = ExifIsLittleEndian(pExifBlockStream, &isLittleEndian);
        VerifyHR(hr);
        hr = SeekExifToIFD0Start(pExifBlockStream, isLittleEndian);
        VerifyHR(hr);
        persistOption = WICPersistOptionDefault | WICMetadataCreationDefault;

        if (!isLittleEndian)
        {
            persistOption |= WICPersistOptionBigEndian;
        }

        hr = pComponentFactory->CreateMetadataReader(GUID_MetadataFormatIfd, nullptr, persistOption, pExifBlockStream, ppMetadataReader);
        VerifyHR(hr);

        return hr;
    }

    HRESULT IMFSampleImageFileWriter::EncodeExifMetaDataIntoJpeg(IWICImagingFactory* pFactory, IWICMetadataBlockWriter* pBlockWriter, BYTE* pExifMetadata, UINT32 blobSize)
    {
        HRESULT hr = S_OK;
        win::ComPtr<IWICComponentFactory> spComponentFactory;
        win::ComPtr<IWICStream> spExifBlockStream;
        win::ComPtr<IWICMetadataReader> spMetadataReader;
        win::ComPtr<IWICMetadataWriter> spApp1Writer;
        win::ComPtr<IWICMetadataQueryWriter> spMetadata;
        //PROPVARIANT spVal;

        hr = pFactory->QueryInterface(const_cast<IWICComponentFactory**>(spComponentFactory.GetAddressOf()));
        VerifyHR(hr);
        hr = pFactory->CreateStream(&spExifBlockStream);
        VerifyHR(hr);
        hr = spExifBlockStream->InitializeFromMemory(pExifMetadata, blobSize);
        VerifyHR(hr);

        hr = SetupExifReader(spExifBlockStream.Get(), spComponentFactory.Get(), &spMetadataReader);
        VerifyHR(hr);

        // 1. APP1: top level metadata structure in JPEG
        hr = spComponentFactory->CreateMetadataWriter(GUID_MetadataFormatApp1, nullptr, WICMetadataCreationDefault, &spApp1Writer);
        VerifyHR(hr);

        // 2. IFD0: sits within APP1
        IWICMetadataWriter *pIfdWriter = nullptr; // pIfdWriter's lifetime is controlled by ifdVar / spVal (CSmartPropVariant).
        PROPVARIANT ifdVar;
        PROPVARIANT varId;

        hr = spComponentFactory->CreateMetadataWriterFromReader(spMetadataReader.Get(), nullptr, &pIfdWriter);
        VerifyHR(hr);

        ifdVar.vt = VT_UNKNOWN;
        ifdVar.punkVal = pIfdWriter;
        //spVal.Attach(ifdVar);
        varId.vt = VT_UI2;
        varId.uiVal = 0; // The 0th IFD is the first (index 0) metadata item in APP1.

        // 3. Next, write the IFD into the APP1 structure.
        hr = spApp1Writer->SetValue(nullptr, &varId, &ifdVar);
        VerifyHR(hr);

        // 4. Finally, write the APP1 into the JPEG image.
        hr = pBlockWriter->AddWriter(spApp1Writer.Get());
        VerifyHR(hr);

        return hr;
    }

    HRESULT IMFSampleImageFileWriter::ExportNV12AsJpeg(const core::wstring &filename, int imageWidth, int imageHeight, IMFSample *sample)
    {
        UINT32 exifBlobSize = 0;
        BYTE* spExifMetadata = NULL;
        win::ComPtr<IWICImagingFactory> spFactory;
        win::ComPtr<IMFMediaBuffer> spMediaBuffer;
        win::ComPtr<IMF2DBuffer> sp2DBuffer;

        GetMetaData(sample, &spExifMetadata, &exifBlobSize);

        HRESULT hr = sample->ConvertToContiguousBuffer(&spMediaBuffer);
        VerifyHR(hr);

        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), &spFactory);
        VerifyHR(hr);

        hr = spMediaBuffer.As(&sp2DBuffer);
        VerifyHR(hr);

        WICBitmapPlane planes[2];

        BYTE* pScanline0;
        LONG pitch;
        hr = sp2DBuffer->Lock2D(&pScanline0, &pitch);
        VerifyHR(hr);

        planes[0].Format = GUID_WICPixelFormat8bppY;
        planes[0].cbStride = pitch;
        planes[0].cbBufferSize = pitch * imageHeight;
        planes[0].pbBuffer = pScanline0;

        planes[1].Format = GUID_WICPixelFormat16bppCbCr;
        planes[1].cbStride = pitch;
        planes[1].cbBufferSize = pitch * imageHeight / 2;
        planes[1].pbBuffer = planes[0].pbBuffer + planes[0].cbBufferSize;

        win::ComPtr<IWICStream> spStream;
        hr = spFactory->CreateStream(&spStream);
        VerifyHR(hr);

        hr = spStream->InitializeFromFilename(filename.c_str(), GENERIC_WRITE);
        VerifyHR(hr);

        win::ComPtr<IWICBitmapEncoder> spEncoder;
        hr = spFactory->CreateEncoder(GUID_ContainerFormatJpeg, nullptr, &spEncoder);
        VerifyHR(hr);

        hr = spEncoder->Initialize(spStream.Get(), WICBitmapEncoderNoCache);
        VerifyHR(hr);

        win::ComPtr<IWICBitmapFrameEncode> spBitmapFrame;
        win::ComPtr<IPropertyBag2> spPropertyBag;
        hr = spEncoder->CreateNewFrame(&spBitmapFrame, &spPropertyBag);
        VerifyHR(hr);

        PROPBAG2 jpegName = {};
        VARIANT jpegValue;

        jpegName.dwType = PROPBAG2_TYPE_DATA;
        jpegName.vt = VT_R4;
        jpegName.pstrName = L"ImageQuality";

        VariantInit(&jpegValue);
        jpegValue.vt = VT_R4;
        jpegValue.fltVal = 0.95f;

        hr = spPropertyBag->Write(1, &jpegName, &jpegValue);
        VerifyHR(hr);

        hr = spBitmapFrame->Initialize(spPropertyBag.Get());
        VerifyHR(hr);

        hr = spBitmapFrame->SetSize(imageWidth, imageHeight);
        VerifyHR(hr);

        WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat24bppBGR;
        hr = spBitmapFrame->SetPixelFormat(&pixelFormat);
        VerifyHR(hr);

        if (exifBlobSize > 0)
        {
            win::ComPtr<IWICMetadataBlockWriter> spBlockWriter;
            spBitmapFrame.As(&spBlockWriter);
            hr = EncodeExifMetaDataIntoJpeg(spFactory.Get(), spBlockWriter.Get(), spExifMetadata, exifBlobSize);
            VerifyHR(hr);
        }

        win::ComPtr<IWICPlanarBitmapFrameEncode> spPlanarFrameEncode;
        hr = spBitmapFrame.As(&spPlanarFrameEncode);
        VerifyHR(hr);

        hr = spPlanarFrameEncode->WritePixels(imageHeight, planes, 2);
        VerifyHR(hr);

        hr = spBitmapFrame->Commit();
        VerifyHR(hr);

        hr = spEncoder->Commit();
        VerifyHR(hr);

        hr = sp2DBuffer->Unlock2D();
        VerifyHR(hr);

        return S_OK;
    }

    HRESULT IMFSampleImageFileWriter::ExportBGRA32AsJpeg(const core::wstring &filename, int imageWidth, int imageHeight, IMFSample *sample)
    {
        UINT32 exifBlobSize = 0;
        BYTE* spExifMetadata = NULL;
        win::ComPtr<IWICImagingFactory> spFactory;
        win::ComPtr<IMFMediaBuffer> spMediaBuffer;
        win::ComPtr<IMF2DBuffer> sp2DBuffer;

        GetMetaData(sample, &spExifMetadata, &exifBlobSize);

        HRESULT hr = sample->ConvertToContiguousBuffer(&spMediaBuffer);
        VerifyHR(hr);

        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), &spFactory);
        VerifyHR(hr);

        hr = spMediaBuffer.As(&sp2DBuffer);
        VerifyHR(hr);

        BYTE* pScanline0;
        LONG pitch;
        hr = sp2DBuffer->Lock2D(&pScanline0, &pitch);
        VerifyHR(hr);

        win::ComPtr<IWICStream> spStream;
        hr = spFactory->CreateStream(&spStream);
        VerifyHR(hr);

        hr = spStream->InitializeFromFilename(filename.c_str(), GENERIC_WRITE);
        VerifyHR(hr);

        win::ComPtr<IWICBitmapEncoder> spEncoder;
        hr = spFactory->CreateEncoder(GUID_ContainerFormatJpeg, nullptr, &spEncoder);
        VerifyHR(hr);

        hr = spEncoder->Initialize(spStream.Get(), WICBitmapEncoderNoCache);
        VerifyHR(hr);

        win::ComPtr<IWICBitmapFrameEncode> spBitmapFrame;
        win::ComPtr<IPropertyBag2> spPropertyBag;
        hr = spEncoder->CreateNewFrame(&spBitmapFrame, &spPropertyBag);
        VerifyHR(hr);

        PROPBAG2 jpegName = {};
        VARIANT jpegValue;

        jpegName.dwType = PROPBAG2_TYPE_DATA;
        jpegName.vt = VT_R4;
        jpegName.pstrName = L"ImageQuality";

        VariantInit(&jpegValue);
        jpegValue.vt = VT_R4;
        jpegValue.fltVal = 0.95f;

        hr = spPropertyBag->Write(1, &jpegName, &jpegValue);
        VerifyHR(hr);

        hr = spBitmapFrame->Initialize(spPropertyBag.Get());
        VerifyHR(hr);

        hr = spBitmapFrame->SetSize(imageWidth, imageHeight);
        VerifyHR(hr);

        WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat32bppRGBA;
        hr = spBitmapFrame->SetPixelFormat(&pixelFormat);
        VerifyHR(hr);

        if (exifBlobSize > 0)
        {
            win::ComPtr<IWICMetadataBlockWriter> spBlockWriter;
            spBitmapFrame.As(&spBlockWriter);
            hr = EncodeExifMetaDataIntoJpeg(spFactory.Get(), spBlockWriter.Get(), spExifMetadata, exifBlobSize);
            VerifyHR(hr);
        }

        win::ComPtr<IWICBitmap> bmpImageData;
        hr = spFactory->CreateBitmapFromMemory(imageWidth, imageHeight, GUID_WICPixelFormat32bppBGRA, pitch, pitch * imageHeight, pScanline0, &bmpImageData);
        VerifyHR(hr);

        hr = spBitmapFrame->WriteSource(bmpImageData, nullptr);
        VerifyHR(hr);

        hr = spBitmapFrame->Commit();
        VerifyHR(hr);

        hr = spEncoder->Commit();
        VerifyHR(hr);

        hr = sp2DBuffer->Unlock2D();
        VerifyHR(hr);

        return S_OK;
    }

    HRESULT IMFSampleImageFileWriter::ExportBGRA32AsPng(const core::wstring &filename, int imageWidth, int imageHeight, IMFSample *sample)
    {
        win::ComPtr<IWICImagingFactory> spFactory;
        win::ComPtr<IMFMediaBuffer> spMediaBuffer;
        win::ComPtr<IMF2DBuffer> sp2DBuffer;

        HRESULT hr = sample->ConvertToContiguousBuffer(&spMediaBuffer);
        VerifyHR(hr);

        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), &spFactory);
        VerifyHR(hr);

        hr = spMediaBuffer.As(&sp2DBuffer);
        VerifyHR(hr);

        BYTE* pScanline0;
        LONG pitch;
        hr = sp2DBuffer->Lock2D(&pScanline0, &pitch);
        VerifyHR(hr);

        win::ComPtr<IWICStream> spStream;
        hr = spFactory->CreateStream(&spStream);
        VerifyHR(hr);

        hr = spStream->InitializeFromFilename(filename.c_str(), GENERIC_WRITE);
        VerifyHR(hr);

        win::ComPtr<IWICBitmapEncoder> spEncoder;
        hr = spFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &spEncoder);
        VerifyHR(hr);

        hr = spEncoder->Initialize(spStream.Get(), WICBitmapEncoderNoCache);
        VerifyHR(hr);

        win::ComPtr<IWICBitmapFrameEncode> spBitmapFrame;
        win::ComPtr<IPropertyBag2> spPropertyBag;
        hr = spEncoder->CreateNewFrame(&spBitmapFrame, &spPropertyBag);
        VerifyHR(hr);

        PROPBAG2 jpegName = {};
        VARIANT jpegValue;

        jpegName.dwType = PROPBAG2_TYPE_DATA;
        jpegName.vt = VT_R4;
        jpegName.pstrName = L"FilterOption";

        VariantInit(&jpegValue);
        jpegValue.vt = VT_UI1;
        jpegValue.bVal = WICPngFilterNone;

        hr = spPropertyBag->Write(1, &jpegName, &jpegValue);
        VerifyHR(hr);

        hr = spBitmapFrame->Initialize(spPropertyBag.Get());
        VerifyHR(hr);

        hr = spBitmapFrame->SetSize(imageWidth, imageHeight);
        VerifyHR(hr);

        WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat32bppRGBA;
        hr = spBitmapFrame->SetPixelFormat(&pixelFormat);
        VerifyHR(hr);

        win::ComPtr<IWICBitmap> bmpImageData;
        hr = spFactory->CreateBitmapFromMemory(imageWidth, imageHeight, GUID_WICPixelFormat32bppBGRA, pitch, pitch * imageHeight, pScanline0, &bmpImageData);
        VerifyHR(hr);

        hr = spBitmapFrame->WriteSource(bmpImageData, nullptr);
        VerifyHR(hr);

        hr = spBitmapFrame->Commit();
        VerifyHR(hr);

        hr = spEncoder->Commit();
        VerifyHR(hr);

        hr = sp2DBuffer->Unlock2D();
        VerifyHR(hr);

        return S_OK;
    }
}

#undef VerifyHR
