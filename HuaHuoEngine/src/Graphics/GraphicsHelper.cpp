//
// Created by VincentZhang on 5/26/2022.
//

#include "GraphicsHelper.h"
#include "Shaders/GraphicsCaps.h"

#define LogRepeatingWarningString LogString

namespace GraphicsHelper
{
    void ValidateMemoryless(GfxRenderTargetSetup& setup)
    {
        for (int i = 0; i < setup.colorCount; i++)
        {
            if (HasFlag(setup.color[i]->flags, kSurfaceCreateMemoryless) && setup.colorLoadAction[i] == kGfxRTLoadActionLoad)
            {
                LogRepeatingWarningString("Ignoring color surface load action as it is memoryless");
                setup.colorLoadAction[i] = kGfxRTLoadActionDontCare;
            }
            if (HasFlag(setup.color[i]->flags, kSurfaceCreateMemoryless) && setup.colorStoreAction[i] == kGfxRTStoreActionStore)
            {
                // In the case of memoryless auto-resolve surfaces the memoryless flag refers to the MSAA data, the store action refers to the resolved data.
                // Overriding the store action to DontCare would invalidate the resolved data, the memoryless MSAA part is implicitly DontCare.
                if (!GetGraphicsCaps().hasMultiSampleAutoResolve || setup.color[i]->samples <= 1)
                {
                    LogRepeatingWarningString("Ignoring color surface store action as it is memoryless");
                    setup.colorStoreAction[i] = kGfxRTStoreActionDontCare;
                }
            }
        }
        if (setup.depth != NULL && HasFlag(setup.depth->flags, kSurfaceCreateMemoryless) && setup.depthLoadAction == kGfxRTLoadActionLoad)
        {
            LogRepeatingWarningString("Ignoring depth surface load action as it is memoryless");
            setup.depthLoadAction = kGfxRTLoadActionDontCare;
        }
        if (setup.depth != NULL && HasFlag(setup.depth->flags, kSurfaceCreateMemoryless) && setup.depthStoreAction == kGfxRTStoreActionStore)
        {
            LogRepeatingWarningString("Ignoring depth surface store action as it is memoryless");
            setup.depthStoreAction = kGfxRTStoreActionDontCare;
        }
    }

}
