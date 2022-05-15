#pragma once

#include "PlatformDependent/Win/WinUnicode.h"

// NOTE:
// Unity functions normally pass UTF8 encoded strings as parameters and return values, but this one has
// to use wide character strings because it passes the "..." parameters to a Windows API function which
// needs that type.
// We cannot use the ANSI version (FormatMessageA) because it doesn't work if any of the parameters
// contain characters which cannot be represented in the current system locale's codepage.

// Formats a message string using the specified message and variable list of arguments.
// It is important to note, though, that these functions are different from the ones
// declared in Word.h, because the string formatter that is used in GetFormattedString
// is FormatMessage WinAPI function, and it expects slightly different formatting syntax
// than, say, printf and its kin.
core::wstring GetFormattedString(LPCWSTR format, ...);

// Extracts an error message from HRESULT.
core::wstring GetHResultErrorMessageW(HRESULT hrError);
core::string GetHResultErrorMessage(HRESULT hrError);
