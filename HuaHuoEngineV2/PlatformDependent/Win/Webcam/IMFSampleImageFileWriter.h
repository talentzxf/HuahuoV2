#pragma once

#if PLATFORM_WIN && !PLATFORM_XBOXONE
#include "External/Windows10/src/ComHelpers.h"
#include "External/Windows10/src/Windows10Interfaces.h"
#include "External/Windows10/src/WinRTCollections.h"
#include "PlatformDependent/Win/ComPtr.h"
#endif

struct IMFSample;
struct IWICMetadataBlockWriter;
struct IWICComponentFactory;
struct IWICImagingFactory;
struct IWICMetadataReader;
struct IWICStream;

namespace Unity
{
#if PLATFORM_WIN && !PLATFORM_XBOXONE
    class IMFSampleImageFileWriter
    {
    public:
        static HRESULT ExportNV12AsJpeg(const core::wstring &filename, int imageWidth, int imageHeight, IMFSample *sample);
        static HRESULT ExportBGRA32AsJpeg(const core::wstring &filename, int imageWidth, int imageHeight, IMFSample *sample);
        static HRESULT ExportBGRA32AsPng(const core::wstring &filename, int imageWidth, int imageHeight, IMFSample *sample);


    private:
        static HRESULT GetMetaData(IMFSample *sample, BYTE** spExifMetadata, UINT32* blobSize);
        static HRESULT EncodeExifMetaDataIntoJpeg(IWICImagingFactory* pFactory, IWICMetadataBlockWriter* pBlockWriter, BYTE* spExifMetadata, UINT32 blobSize);
        static HRESULT SetupExifReader(IWICStream* pExifBlockStream, IWICComponentFactory *pComponentFactory, IWICMetadataReader** ppMetadataReader);
        static HRESULT ExifIsLittleEndian(IWICStream* pExifBlockStream, bool* pIsLittleEndian);
        static HRESULT SeekExifToIFD0Start(IWICStream* pExifBlockStream, bool isLittleEndian);
    };
#endif
}
