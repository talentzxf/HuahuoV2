#include "UnityPrefix.h"
#include "Runtime/Input/Cursor.h"
#include "JSBridge.h"
#include "Runtime/Graphics/Image.h"
#include "Runtime/Graphics/Texture2D.h"

#if PLATFORM_SUPPORTS_HARDWARE_CURSORS

namespace Cursors
{
    PPtr<Texture2D> s_SoftwareCursor;
    Vector2f s_Hotspot;

    template<typename T>
    void WriteHeader(char** buffer, T data)
    {
        memcpy(*buffer, &data, sizeof(data));
        *buffer += sizeof(data);
    }

    void SetCursor(Texture2D* texture, Vector2f hotSpot, CursorMode cursorMode)
    {
        s_Hotspot = hotSpot;
        s_SoftwareCursor = NULL;

        if (texture == NULL)
        {
            JS_Cursor_SetShow(true);
            return;
        }

        if (cursorMode == kHardwareCursorOff)
        {
            JS_Cursor_SetShow(false);
            s_SoftwareCursor = texture;
            return;
        }

        int width = texture->GetDataWidth();
        int height = texture->GetDataHeight();
        // CSS has a maximum cursor size of 128x128. Warn the user if a larger cursor is being used, that it will
        // likely go ignored.
        const int maxWebGLCursorSize = 128;
        if (width > maxWebGLCursorSize || height > maxWebGLCursorSize)
        {
            WarningString("The cursor image used is larger than the CSS cursor size limit of 128x128.");
        }
        char *buffer;
        size_t headerSize = 22 + 2 + 40;
        size_t size = headerSize + sizeof(UInt32) * width * height;
        MALLOC_TEMP_AUTO(buffer, size);
        char *start = buffer;

        // http://en.wikipedia.org/wiki/ICO_(file_format)
        // ICONDIR structure
        WriteHeader<UInt16>(&buffer, 0); // Reserved. Must always be 0.
        WriteHeader<UInt16>(&buffer, 2); // Specifies image type: 1 for icon (.ICO) image, 2 for cursor (.CUR) image. Other values are invalid.
        WriteHeader<UInt16>(&buffer, 1); // Specifies number of images in the file.

        // ICONDIRENTRY
        WriteHeader<UInt8>(&buffer, width); // Specifies image width in pixels. Can be any number between 0 and 255. Value 0 means image width is 256 pixels.
        WriteHeader<UInt8>(&buffer, height); // Specifies image height in pixels. Can be any number between 0 and 255. Value 0 means image height is 256 pixels.
        WriteHeader<UInt8>(&buffer, 0); // Specifies number of colors in the color palette. Should be 0 if the image does not use a color palette.
        WriteHeader<UInt8>(&buffer, 0); // Reserved. Should be 0.
        WriteHeader<UInt16>(&buffer, clamp<int>(hotSpot.x, 0, width - 1)); // In CUR format: Specifies the horizontal coordinates of the hotspot in number of pixels from the left.
        WriteHeader<UInt16>(&buffer, clamp<int>(hotSpot.y, 0, height - 1)); // In CUR format: Specifies the vertical coordinates of the hotspot in number of pixels from the top.
        WriteHeader<UInt32>(&buffer, size - 24); // Specifies the size of the image's data in bytes
        WriteHeader<UInt32>(&buffer, 24); // Specifies the offset of BMP or PNG data from the beginning of the ICO/CUR file

        WriteHeader<UInt16>(&buffer, 0); // Two padding bytes, so that the image data is 32-bit aligned.

        // http://en.wikipedia.org/wiki/BMP_file_format
        // BITMAPINFOHEADER
        WriteHeader<UInt32>(&buffer, 40); // the size of this header (40 bytes)
        WriteHeader<UInt32>(&buffer, width); // the bitmap width in pixels (signed integer)
        WriteHeader<UInt32>(&buffer, height * 2); // the bitmap height in pixels (signed integer)
        WriteHeader<UInt16>(&buffer, 1); // the number of color planes must be 1
        WriteHeader<UInt16>(&buffer, 32); // the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32.
        WriteHeader<UInt32>(&buffer, 0); // the compression method being used. See the next table for a list of possible values
        WriteHeader<UInt32>(&buffer, 0); // the image size. This is the size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps.
        WriteHeader<UInt32>(&buffer, 0); // the horizontal resolution of the image. (pixel per meter, signed integer)
        WriteHeader<UInt32>(&buffer, 0); // the vertical resolution of the image. (pixel per meter, signed integer)
        WriteHeader<UInt32>(&buffer, 0); // the number of colors in the color palette, or 0 to default to 2n
        WriteHeader<UInt32>(&buffer, 0); // the number of important colors used, or 0 when every color is important; generally ignored

        // writing image data
        ImageReference image(width, height, width * 4, kFormatA8R8G8B8_UNorm, buffer);
        texture->ExtractImage(&image);
        ConvertPixels(reinterpret_cast<const UInt8*>(buffer), kFormatA8R8G8B8_UNorm,
            reinterpret_cast<UInt8*>(buffer), kFormatB8G8R8A8_UNorm, width * height);

        JS_Cursor_SetImage(start, size);
    }

    void InitializeCursors(Texture2D* defaultCursorTexture, Vector2f defaultCursorHotSpot)
    {
        SetCursor(defaultCursorTexture, defaultCursorHotSpot, kAutoHardwareCursor);
    }

    void CleanupCursors()
    {
    }

    Texture2D* GetSoftwareCursor()
    {
        return s_SoftwareCursor;
    }

    Vector2f GetCursorHotspot()
    {
        return s_Hotspot;
    }
} //namespace
#endif
