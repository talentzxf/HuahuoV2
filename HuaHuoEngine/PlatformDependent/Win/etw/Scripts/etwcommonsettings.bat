@rem Customize 'CustomProviders' to your own ETW providers.

@rem @set CustomProviders=+Multi-MAIN+Multi-FrameRate+Multi-Input+Multi-Worker




@rem Set _NT_SYMBOL_PATH if it is not already set

@if not "%_NT_SYMBOL_PATH%" == "" goto SymbolPathSet
set _NT_SYMBOL_PATH=SRV*c:\symbols*\\perforce\symbols*http://msdl.microsoft.com/download/symbols;

:SymbolPathSet


@rem Set some helpful environment variables
@set INVALIDFLAGS=-2147023892
@set ACCESSISDENIED=-2147024891

@rem Set the flag that tells the 64-bit kernel to keep stackwalking metadata locked in memory.
@rem Forcing users to do this step manually adds too much confusion. This won't take effect
@rem until a reboot
@REG ADD "HKLM\System\CurrentControlSet\Control\Session Manager\Memory Management" -v DisablePagingExecutive -d 0x1 -t REG_DWORD -f >nul




@rem Set up reasonable defaults for Microsoft user-mode ETW providers, based
@rem on the OS version. Don't detect Windows 7 specifically because I want
@rem this batch file to work on Windows 8 and above.
@rem http://en.wikipedia.org/wiki/Ver_(command)

@ver | find "5.1."
@if %errorlevel% == 0 goto WindowsXP

@ver | find "6.0."
@if %errorlevel% == 0 goto WindowsVista

:Windows7
@rem Microsoft-Windows-Win32k adds Window focus events. This is available only
@rem on Windows 7 and above.
@rem f736823e-eef1-49c0-83fb-d036f507b210 <- Unity provider GUID
@set UserProviders=Microsoft-Windows-Win32k+f736823e-eef1-49c0-83fb-d036f507b210
@goto UserProvidersAreSet

:WindowsVista
@echo Vista detected. Xperf works better on Windows 7.
@rem This provider isn't very useful but I need something non-blank here to
@rem avoid changing the syntax too much.
@set UserProviders=Microsoft-Windows-LUA
@goto UserProvidersAreSet

:WindowsXP
@echo User providers are not supported on Windows XP. Xperf works better on Windows 7.
@goto Done

:UserProvidersAreSet

@rem The :0x2F mask for the DX provider enables a subset of events. See xperf\gpuview\log.cmd for examples.
@rem DX providers can be useful. Uncomment the following line to enable them.
@rem @set UserProviders=%UserProviders%+DX:0x2F

@rem DWM providers are occasionally helpful. Uncomment the following line to enable them.
@rem @set UserProviders=%UserProviders%+Microsoft-Windows-Dwm-Dwm

:Done
