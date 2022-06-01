//
// Created by VincentZhang on 5/27/2022.
//

#include "ShaderUtilities.h"
#include "GfxDevice/GfxDevice.h"
#include "Input/TimeManager.h"
#include "Utilities/Utility.h"

namespace ShaderLab
{
    void UpdateGlobalShaderProperties(float overrideTime)
    {
        GfxDevice& device = GetGfxDevice();
        BuiltinShaderParamValues& params = device.GetBuiltinParamValues();

        // Time properties
        const TimeManager& timeMgr = GetTimeManager();
        float time = timeMgr.GetTimeSinceSceneLoad();
        if (overrideTime >= 0.0f)
            time = overrideTime;

        const float lastFrameTime = time - timeMgr.GetDeltaTime();
        const float kMinDT = 0.005f;
        const float kMaxDT = 0.2f;
        const float deltaTime = clamp(timeMgr.GetDeltaTime(), kMinDT, kMaxDT);
        const float smoothDeltaTime = clamp(timeMgr.GetSmoothDeltaTime(), kMinDT, kMaxDT);

        // The 0.05 in kShaderVecTime is a typo, but can't change it now. There are water shaders out there that
        // use exactly .x component :(

        params.SetVectorParam(kShaderVecTime, Vector4f(0.05f * time, time, 2.0f * time, 3.0f * time));
        params.SetVectorParam(kShaderLastVecTime, Vector4f(0.05f * lastFrameTime, lastFrameTime, 2.0f * lastFrameTime, 3.0f * lastFrameTime));
        params.SetVectorParam(kShaderVecSinTime, Vector4f(sinf(0.125f * time), sinf(0.25f * time), sinf(0.5f * time), sinf(time)));
        params.SetVectorParam(kShaderVecCosTime, Vector4f(cosf(0.125f * time), cosf(0.25f * time), cosf(0.5f * time), cosf(time)));
        params.SetVectorParam(kShaderVecPiTime, Vector4f(fmodf(time, kPI), fmodf(2.0f * time, kPI), fmodf(3.0f * time, kPI), fmodf(4.0f * time, kPI)));
        params.SetVectorParam(kShaderVecDeltaTime, Vector4f(deltaTime, 1.0f / deltaTime, smoothDeltaTime, 1.0f / smoothDeltaTime));
    }
}