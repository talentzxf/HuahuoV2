#include "UnityPrefix.h"
#include "Runtime/Misc/Player.h"

void GetCanvasClientSize(double *outWidth, double *outHeight)
{
    // When canvas element is not displayed, its size can not be determined (width and height are zero).
    // To allow the build run in background, the last valid size of the canvas is returned.

    static double lastValidWidth = 1;
    static double lastValidHeight = 1;

    JS_SystemInfo_GetCanvasClientSize("#canvas", outWidth, outHeight);

    if (*outWidth > 0 && *outHeight > 0)
    {
        lastValidWidth = *outWidth;
        lastValidHeight = *outHeight;
    }
    else if (GetPlayerSettingsRunInBackground())
    {
        *outWidth = lastValidWidth;
        *outHeight = lastValidHeight;
    }
    // If canvas has zero size and runInBackground property is not enabled,
    // then this zero size will be returned, which will prevent the player loop from running.
}
