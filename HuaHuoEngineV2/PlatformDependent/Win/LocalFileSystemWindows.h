#pragma once

#include "LocalFileSystemWindowsShared.h"

class LocalFileSystemWindows : public LocalFileSystemWindowsShared
{
public:
    LocalFileSystemWindows();

    virtual bool          MoveToTrash(FileEntryData& data);
    virtual bool          Target(const FileEntryData& data, FileSystemEntry& out_symLink) const;

    virtual core::string  GetUserAppDataFolder() const;
    virtual core::string  GetApplicationManagedPath() const;
    virtual core::string  GetSharedApplicationDataFolder() const;

    // WARNING: Do not add logic to this class unless it is conceptually incompatible with UWP, or require very different implemention
    // Instead, put new code to LocalFileSystemWindowsShared
protected:
    virtual HMODULE GetHModuleForApplicationPath() const;

private:
    mutable core::string  m_CachedUserAppDataFolder;
};
