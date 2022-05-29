#pragma once

#include "GfxDevice/GfxDeviceTypes.h"
#include "Graphics/ColorGamut.h"
//#include "Runtime/Scripting/BindingsDefs.h"

class ScriptingExceptionPtr;

struct DisplayChromacities
{
    float RedPrimaryX, RedPrimaryY;
    float GreenPrimaryX, GreenPrimaryY;
    float BluePrimaryX, BluePrimaryY;
    float WhitePointX, WhitePointY;
};

class HDROutputSettings
{
public:
    static const float DefaultPaperWhiteInNits;

    HDROutputSettings();
    virtual ~HDROutputSettings() {}

    void ResetDisplayChromacity(ColorGamut displayColorGamut);

    void SetActive(bool active) { m_Active = active; m_HDRModeChangeRequested = false; }
    void SetAvailable(bool available) { m_Available = available; }
    void SetAutomaticHDRTonemapping(bool enable) { m_AutomaticToneMapping = enable; }
    void SetDisplayColorGamut(ColorGamut displayColorGamut) { m_DisplayColorGamut = displayColorGamut; }
    void SetGraphicsFormat(GraphicsFormat displayFormat) { m_GraphicsFormat = displayFormat; }
    void SetPaperWhiteInNits(float paperWhite) { m_PaperWhiteInNits = paperWhite; }
    void SetMaxFullFrameToneMapLuminance(int maxFullFrame) { m_MaxFullFrameToneMapLuminance = maxFullFrame; }
    void SetMaxToneMapLuminance(int max) { m_MaxToneMapLuminance = max; }
    void SetMinToneMapLuminance(int min) { m_MinToneMapLuminance = min; }
    void SetHDRModeChangeRequested(bool active) { m_HDRModeChangeRequested = active; }
    void SetDesiredHDRActive(bool active) { m_DesiredHDRActive = active; }

    bool GetActive() const { return m_Active; }
    bool GetAvailable() const;
    bool GetAutomaticHDRTonemapping() const { return m_AutomaticToneMapping; }
    ColorGamut GetDisplayColorGamut() const { return m_DisplayColorGamut; }
    RenderTextureFormat GetFormat() const { return GetRenderTextureFormat(m_GraphicsFormat); }
    GraphicsFormat GetGraphicsFormat() const { return m_GraphicsFormat; }
    float GetPaperWhiteInNits() const { return m_PaperWhiteInNits; }
    int GetMaxFullFrameToneMapLuminance() const { return m_MaxFullFrameToneMapLuminance; }
    int GetMaxToneMapLuminance() const { return m_MaxToneMapLuminance; }
    int GetMinToneMapLuminance() const { return m_MinToneMapLuminance; }
    bool GetHDRModeChangeRequested() const { return m_HDRModeChangeRequested; }
    bool GetDesiredHDRActive() { return m_DesiredHDRActive; }

    void SetRedPrimaries(float x, float y)
    {
        m_DisplayChromacities.RedPrimaryX = x;
        m_DisplayChromacities.RedPrimaryY = y;
    }

    void SetGreenPrimaries(float x, float y)
    {
        m_DisplayChromacities.GreenPrimaryX = x;
        m_DisplayChromacities.GreenPrimaryY = y;
    }

    void SetBluePrimaries(float x, float y)
    {
        m_DisplayChromacities.BluePrimaryX = x;
        m_DisplayChromacities.BluePrimaryY = y;
    }

    void SetWhitePoint(float x, float y)
    {
        m_DisplayChromacities.WhitePointX = x;
        m_DisplayChromacities.WhitePointY = y;
    }

    DisplayChromacities GetDisplayChromacities() { return m_DisplayChromacities; }

    //Often we get nonsense back from the platform api's when asking the display for its luminance range.
    //This function attempts to sanitize this as best as possible by writing -1 to the luminances and reporting warnings where we could get sensible values back
    //Call this after populating the HDROutputSettings with values returned from the display.
    void ValidateLuminances();

    void RequestHDRModeChange(bool enabled);

private:
    bool m_Active;
    bool m_Available;
    bool m_AutomaticToneMapping;
    ColorGamut m_DisplayColorGamut;
    GraphicsFormat m_GraphicsFormat;
    float m_PaperWhiteInNits;
    int m_MaxFullFrameToneMapLuminance;
    int m_MaxToneMapLuminance;
    int m_MinToneMapLuminance;
    bool m_HDRModeChangeRequested;

    DisplayChromacities m_DisplayChromacities;

    // Does the user currently want HDR enabled if we are on an HDR display.
    // We need to know this here and on desktop (unlike consoles) we have the potential that the window might move from HDR capable to non HDR capable displays
    // When returning to an HDR display in such an scenario we need to keep track of whether the user wants HDR active.
    bool m_DesiredHDRActive;
};

namespace HDROutputSettingsBindings
{
    bool GetActive(int displayIndex, ScriptingExceptionPtr* exception);
    bool GetAvailable(int displayIndex, ScriptingExceptionPtr* exception);
    bool GetAutomaticHDRTonemapping(int displayIndex, ScriptingExceptionPtr* exception);
    void SetAutomaticHDRTonemapping(int displayIndex, bool enabled, ScriptingExceptionPtr* exception);
    ColorGamut GetDisplayColorGamut(int displayIndex, ScriptingExceptionPtr* exception);
    GraphicsFormat GetGraphicsFormat(int displayIndex, ScriptingExceptionPtr* exception);
    float GetPaperWhiteNits(int displayIndex, ScriptingExceptionPtr* exception);
    void SetPaperWhiteNits(int displayIndex, float paperWhite, ScriptingExceptionPtr* exception);
    int GetMaxFullFrameToneMapLuminance(int displayIndex, ScriptingExceptionPtr* exception);
    int GetMaxToneMapLuminance(int displayIndex, ScriptingExceptionPtr* exception);
    int GetMinToneMapLuminance(int displayIndex, ScriptingExceptionPtr* exception);
    bool GetHDRModeChangeRequested(int displayIndex, ScriptingExceptionPtr* exception);
    void RequestHDRModeChange(int displayIndex, bool enabled, ScriptingExceptionPtr* exception);

    HDROutputSettings* GetHDROutputSettingsThrowIfNull(int displayIndex, ScriptingExceptionPtr* exception);
    HDROutputSettings* GetHDROutputSettingsThrowIfNotAvailable(int displayIndex, ScriptingExceptionPtr* exception);
}
