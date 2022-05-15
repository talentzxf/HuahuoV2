#pragma once

class CrashUI
{
public:
    virtual bool Show(
        const wchar_t* applicationPath,
        const wchar_t* applicationName,
        const wchar_t* versionInfo) = 0;

    virtual bool Hide() = 0;
    virtual bool IsShown() const = 0;

    // Thread safe.
    virtual void NotifyProgress(int progress) = 0;

    // Blocks until dialog is complete.
    virtual bool Wait() = 0;

    static CrashUI& Get();

protected:
    CrashUI() {}
    virtual ~CrashUI() {}
};
