/**********************************************************************
 *
 * StackWalker.cpp
 * http://stackwalker.codeplex.com/
 *
 *
 * History:
 *  2005-07-27   v1    - First public release on http://www.codeproject.com/
 *                       http://www.codeproject.com/threads/StackWalker.asp
 *  2005-07-28   v2    - Changed the params of the constructor and ShowCallstack
 *                       (to simplify the usage)
 *  2005-08-01   v3    - Changed to use 'CONTEXT_FULL' instead of CONTEXT_ALL
 *                       (should also be enough)
 *                     - Changed to compile correctly with the PSDK of VC7.0
 *                       (GetFileVersionInfoSizeA and GetFileVersionInfoA is wrongly defined:
 *                        it uses LPSTR instead of LPCSTR as first paremeter)
 *                     - Added declarations to support VC5/6 without using 'dbghelp.h'
 *                     - Added a 'pUserData' member to the ShowCallstack function and the
 *                       PReadProcessMemoryRoutine declaration (to pass some user-defined data,
 *                       which can be used in the readMemoryFunction-callback)
 *  2005-08-02   v4    - OnSymInit now also outputs the OS-Version by default
 *                     - Added example for doing an exception-callstack-walking in main.cpp
 *                       (thanks to owillebo: http://www.codeproject.com/script/profile/whos_who.asp?id=536268)
 *  2005-08-05   v5    - Removed most Lint (http://www.gimpel.com/) errors... thanks to Okko Willeboordse!
 *  2008-08-04   v6    - Fixed Bug: Missing LEAK-end-tag
 *                       http://www.codeproject.com/KB/applications/leakfinder.aspx?msg=2502890#xx2502890xx
 *                       Fixed Bug: Compiled with "WIN32_LEAN_AND_MEAN"
 *                       http://www.codeproject.com/KB/applications/leakfinder.aspx?msg=1824718#xx1824718xx
 *                       Fixed Bug: Compiling with "/Wall"
 *                       http://www.codeproject.com/KB/threads/StackWalker.aspx?msg=2638243#xx2638243xx
 *                       Fixed Bug: Now checking SymUseSymSrv
 *                       http://www.codeproject.com/KB/threads/StackWalker.aspx?msg=1388979#xx1388979xx
 *                       Fixed Bug: Support for recursive function calls
 *                       http://www.codeproject.com/KB/threads/StackWalker.aspx?msg=1434538#xx1434538xx
 *                       Fixed Bug: Missing FreeLibrary call in "GetModuleListTH32"
 *                       http://www.codeproject.com/KB/threads/StackWalker.aspx?msg=1326923#xx1326923xx
 *                       Fixed Bug: SymDia is number 7, not 9!
 *  2008-09-11   v7      For some (undocumented) reason, dbhelp.h is needing a packing of 8!
 *                       Thanks to Teajay which reported the bug...
 *                       http://www.codeproject.com/KB/applications/leakfinder.aspx?msg=2718933#xx2718933xx
 *  2008-11-27   v8      Debugging Tools for Windows are now stored in a different directory
 *                       Thanks to Luiz Salamon which reported this "bug"...
 *                       http://www.codeproject.com/KB/threads/StackWalker.aspx?msg=2822736#xx2822736xx
 *  2009-04-10   v9      License slihtly corrected (<ORGANIZATION> replaced)
 *  2009-11-01   v10     Moved to http://stackwalker.codeplex.com/
 *  2009-11-02   v11     Now try to use IMAGEHLP_MODULE64_V3 if available
 *  2010-04-15   v12     Added support for VS2010 RTM
 *  2010-05-25   v13     Now using secure MyStrcCpy. Thanks to luke.simon:
 *                       http://www.codeproject.com/KB/applications/leakfinder.aspx?msg=3477467#xx3477467xx
 *  2013-01-07   v14     Runtime Check Error VS2010 Debug Builds fixed:
 *                       http://stackwalker.codeplex.com/workitem/10511
 *  2017-06-27   v15     (pete.lewis@unity3d.com) Update stack walk to work Out-of-Process
 *
 *
 * LICENSE (http://www.opensource.org/licenses/bsd-license.php)
 *
 *   Copyright (c) 2005-2013, Jochen Kalmbach
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without modification,
 *   are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *   Neither the name of Jochen Kalmbach nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 *   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **********************************************************************/

// Note: When Unity is installed, the pdb next to Unity.exe is stripped (https://msdn.microsoft.com/en-us/library/y87kw2fd.aspx)
//       When Unity is built locally, the pdb next to Unity.exe has full information.
//	The stripped PDB file will not contain:
//		Type information
//		Line number information
//		Per-object file CodeView symbols such as those for functions, locals, and static data
// Thus stackwalker won't ouput line numbers for C++ functions when using stripped pdb

#if PLATFORM_METRO
// Note: We're including DbgToolsIncludes.h first so it would be possible to use forbidden Desktop API
#include "PlatformDependent/WinRT/DebuggingTools/DbgToolsIncludes.h"

#else
#include "UnityPrefix.h"
#include "VersionHelpers.h"
#include <windows.h>
#pragma warning(disable:4826)

#endif

#if !PLATFORM_WINRT || (UNITY_DEVELOPER_BUILD && DBGTOOLS_WINRT_FORBIDDEN_API_AVAILABLE && (DBGTOOLS_UAP_X86 || DBGTOOLS_UAP_X64))
// Missing Stackwalker items for ARM platform:
//		Need to correctly setup frame pointers for Windows Store Apps/Windows Phone 8.1 ARM
//
// Missing functions for Windows Phone 8.1 ARM
//		__imp_GetEnvironmentVariableA
//		__imp_GetCurrentDirectoryA
//		__imp_LoadLibraryA
//		__imp_GetUserNameA
//		GetFileVersionInfoSizeA
//		GetFileVersionInfoA
//		VerQueryValueA
//
// Missing functions for Windows Phone 8.1 X86
//		__imp__GetCurrentDirectoryA
//		__imp__GetEnvironmentVariableA
//		__imp__GetUserNameA
//		__imp__LoadLibraryA
//
// Missing functions for Windows Store Apps ARM
//		GetFileVersionInfoSizeA
//		GetFileVersionInfoA
//		VerQueryValueA


#if !DBGTOOLS_UAP_ARM
#pragma comment(lib, "version.lib")  // for "VerQueryValue"
#endif

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h> // alloca
#include "PlatformDependent/Win/StackWalker.h"
#include "Runtime/Scripting/ManagedStacktrace.h"
#include "Runtime/Testing/Faking.h"


#pragma pack(push,8)
#include <dbghelp.h>
#pragma pack(pop)

// Some missing defines (for VC5/6):
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif


void StackWalker::MyStrCpy(char* szDest, size_t nMaxDestSize, const char* szSrc)
{
  if (nMaxDestSize <= 0) return;
  if (strlen(szSrc) < nMaxDestSize)
  {
    strcpy_s(szDest, nMaxDestSize, szSrc);
  }
  else
  {
    strncpy_s(szDest, nMaxDestSize, szSrc, nMaxDestSize);
    szDest[nMaxDestSize-1] = 0;
  }
}  // MyStrCpy

// Normally it should be enough to use 'CONTEXT_FULL' (better would be 'CONTEXT_ALL')
#define USED_CONTEXT_FLAGS CONTEXT_FULL


class StackWalkerInternal
{
public:
  StackWalkerInternal(StackWalker *parent, HANDLE hProcess)
  {
    m_parent = parent;
    m_hDbhHelp = NULL;
    pSC = NULL;
    m_hProcess = hProcess;
    m_szSymPath = NULL;
    pSFTA = NULL;
    pSGLFA = NULL;
    pSGMB = NULL;
    pSGMI = NULL;
    pSGO = NULL;
    pSGSFA = NULL;
    pSI = NULL;
    pSLM = NULL;
    pSSO = NULL;
    pSW = NULL;
    pUDSN = NULL;
    pSGSP = NULL;
  }
  ~StackWalkerInternal()
  {
    if (pSC != NULL)
      pSC(m_hProcess);  // SymCleanup
    if (m_hDbhHelp != NULL)
      FreeLibrary(m_hDbhHelp);
    m_hDbhHelp = NULL;
    m_parent = NULL;
    if(m_szSymPath != NULL)
      free(m_szSymPath);
    m_szSymPath = NULL;
  }
  BOOL Init(LPCSTR szSymPath)
  {
    if (m_parent == NULL)
      return FALSE;
    // Dynamically load the Entry-Points for dbghelp.dll:
    // First try to load the newsest one from
    TCHAR szTemp[4096];
    // But before wqe do this, we first check if the ".local" file exists
    if (GetModuleFileName(NULL, szTemp, 4096) > 0)
    {
      _tcscat_s(szTemp, _T(".local"));
      if (GetFileAttributes(szTemp) == INVALID_FILE_ATTRIBUTES)
      {
        // ".local" file does not exist, so we can try to load the dbghelp.dll from the "Debugging Tools for Windows"
        // Ok, first try the new path according to the archtitecture:
#ifdef _M_IX86
	    if ( (m_hDbhHelp == NULL) && (GetEnvironmentVariable(_T("ProgramFiles"), szTemp, 4096) > 0) )
	    {
	      _tcscat_s(szTemp, _T("\\Debugging Tools for Windows (x86)\\dbghelp.dll"));
	      // now check if the file exists:
	      if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
	      {
	        m_hDbhHelp = LoadLibrary(szTemp);
	      }
	    }
#elif _M_X64
	    if ( (m_hDbhHelp == NULL) && (GetEnvironmentVariable(_T("ProgramFiles"), szTemp, 4096) > 0) )
	    {
	      _tcscat_s(szTemp, _T("\\Debugging Tools for Windows (x64)\\dbghelp.dll"));
	      // now check if the file exists:
	      if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
	      {
	        m_hDbhHelp = LoadLibrary(szTemp);
	      }
	    }
#elif _M_IA64
	    if ( (m_hDbhHelp == NULL) && (GetEnvironmentVariable(_T("ProgramFiles"), szTemp, 4096) > 0) )
	    {
	      _tcscat_s(szTemp, _T("\\Debugging Tools for Windows (ia64)\\dbghelp.dll"));
	      // now check if the file exists:
	      if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
	      {
	        m_hDbhHelp = LoadLibrary(szTemp);
	      }
	    }
#endif
	    // If still not found, try the old directories...
	    if ( (m_hDbhHelp == NULL) && (GetEnvironmentVariable(_T("ProgramFiles"), szTemp, 4096) > 0) )
	    {
	      _tcscat_s(szTemp, _T("\\Debugging Tools for Windows\\dbghelp.dll"));
	      // now check if the file exists:
	      if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
	      {
	        m_hDbhHelp = LoadLibrary(szTemp);
	      }
	    }
#if defined _M_X64 || defined _M_IA64
		// Still not found? Then try to load the (old) 64-Bit version:
		if ( (m_hDbhHelp == NULL) && (GetEnvironmentVariable(_T("ProgramFiles"), szTemp, 4096) > 0) )
		{
		  _tcscat_s(szTemp, _T("\\Debugging Tools for Windows 64-Bit\\dbghelp.dll"));
		  if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
		  {
		    m_hDbhHelp = LoadLibrary(szTemp);
		  }
		}
#endif
	  }
	}
	if (m_hDbhHelp == NULL)  // if not already loaded, try to load a default-one
	  m_hDbhHelp = LoadLibrary( _T("dbghelp.dll") );
	if (m_hDbhHelp == NULL)
	  return FALSE;
	pSI = (tSI) GetProcAddress(m_hDbhHelp, "SymInitialize" );
	pSC = (tSC) GetProcAddress(m_hDbhHelp, "SymCleanup" );

	pSW = (tSW) GetProcAddress(m_hDbhHelp, "StackWalk64" );
	pSGO = (tSGO) GetProcAddress(m_hDbhHelp, "SymGetOptions" );
	pSSO = (tSSO) GetProcAddress(m_hDbhHelp, "SymSetOptions" );

	pSFTA = (tSFTA) GetProcAddress(m_hDbhHelp, "SymFunctionTableAccess64" );
	pSGLFA = (tSGLFA) GetProcAddress(m_hDbhHelp, "SymGetLineFromAddr64" );
	pSGMB = (tSGMB) GetProcAddress(m_hDbhHelp, "SymGetModuleBase64" );
	pSGMI = (tSGMI) GetProcAddress(m_hDbhHelp, "SymGetModuleInfo64" );
	pSGSFA = (tSGSFA) GetProcAddress(m_hDbhHelp, "SymGetSymFromAddr64" );
	pUDSN = (tUDSN) GetProcAddress(m_hDbhHelp, "UnDecorateSymbolName" );
	pSLM = (tSLM) GetProcAddress(m_hDbhHelp, "SymLoadModule64" );
	pSGSP =(tSGSP) GetProcAddress(m_hDbhHelp, "SymGetSearchPath" );

	if ( pSC == NULL || pSFTA == NULL || pSGMB == NULL || pSGMI == NULL ||
	  pSGO == NULL || pSGSFA == NULL || pSI == NULL || pSSO == NULL ||
	  pSW == NULL || pUDSN == NULL || pSLM == NULL )
	{
	  FreeLibrary(m_hDbhHelp);
	  m_hDbhHelp = NULL;
	  pSC = NULL;
	  return FALSE;
	}

	// SymInitialize
	if (szSymPath != NULL)
	  m_szSymPath = _strdup(szSymPath);
	if (this->pSI(m_hProcess, m_szSymPath, FALSE) == FALSE)
	  this->m_parent->OnDbgHelpErr("SymInitialize", GetLastError(), 0);

	DWORD symOptions = this->pSGO();  // SymGetOptions
	symOptions |= SYMOPT_LOAD_LINES;
	symOptions |= SYMOPT_FAIL_CRITICAL_ERRORS;
	symOptions |= SYMOPT_DEFERRED_LOADS;
	//symOptions |= SYMOPT_NO_PROMPTS;
	// SymSetOptions
	symOptions = this->pSSO(symOptions);

	char buf[StackWalker::STACKWALK_MAX_NAMELEN] = {0};
	if (this->pSGSP != NULL)
	{
	  if (this->pSGSP(m_hProcess, buf, StackWalker::STACKWALK_MAX_NAMELEN) == FALSE)
	    this->m_parent->OnDbgHelpErr("SymGetSearchPath", GetLastError(), 0);
	}
	char szUserName[1024] = {0};
#if PLATFORM_WINRT
	// GetUserNameA & GetUserNameW don't exist on UWP
	strcpy(szUserName, "Unknown");
#else
	DWORD dwSize = 1024;
	GetUserNameA(szUserName, &dwSize);
#endif
	this->m_parent->OnSymInit(buf, symOptions, szUserName);

	return TRUE;
  }

  StackWalker *m_parent;

  HMODULE m_hDbhHelp;
  HANDLE m_hProcess;
  LPSTR m_szSymPath;

#pragma pack(push,8)
struct IMAGEHLP_MODULE64_V3 {
	DWORD    SizeOfStruct;           // set to sizeof(IMAGEHLP_MODULE64)
	DWORD64  BaseOfImage;            // base load address of module
	DWORD    ImageSize;              // virtual size of the loaded module
	DWORD    TimeDateStamp;          // date/time stamp from pe header
	DWORD    CheckSum;               // checksum from the pe header
	DWORD    NumSyms;                // number of symbols in the symbol table
	SYM_TYPE SymType;                // type of symbols loaded
	CHAR     ModuleName[32];         // module name
	CHAR     ImageName[256];         // image name
	CHAR     LoadedImageName[256];   // symbol file name
	// new elements: 07-Jun-2002
	CHAR     LoadedPdbName[256];     // pdb file name
	DWORD    CVSig;                  // Signature of the CV record in the debug directories
	CHAR     CVData[MAX_PATH * 3];   // Contents of the CV record
	DWORD    PdbSig;                 // Signature of PDB
	GUID     PdbSig70;               // Signature of PDB (VC 7 and up)
	DWORD    PdbAge;                 // DBI age of pdb
	BOOL     PdbUnmatched;           // loaded an unmatched pdb
	BOOL     DbgUnmatched;           // loaded an unmatched dbg
	BOOL     LineNumbers;            // we have line number information
	BOOL     GlobalSymbols;          // we have internal symbol information
	BOOL     TypeInfo;               // we have type information
	// new elements: 17-Dec-2003
	BOOL     SourceIndexed;          // pdb supports source server
	BOOL     Publics;                // contains public symbols
};

struct IMAGEHLP_MODULE64_V2 {
	DWORD    SizeOfStruct;           // set to sizeof(IMAGEHLP_MODULE64)
	DWORD64  BaseOfImage;            // base load address of module
	DWORD    ImageSize;              // virtual size of the loaded module
	DWORD    TimeDateStamp;          // date/time stamp from pe header
	DWORD    CheckSum;               // checksum from the pe header
	DWORD    NumSyms;                // number of symbols in the symbol table
	SYM_TYPE SymType;                // type of symbols loaded
	CHAR     ModuleName[32];         // module name
	CHAR     ImageName[256];         // image name
	CHAR     LoadedImageName[256];   // symbol file name
};
#pragma pack(pop)


  // SymCleanup()
  typedef BOOL (__stdcall *tSC)( IN HANDLE hProcess );
  tSC pSC;

  // SymFunctionTableAccess64()
  typedef PVOID (__stdcall *tSFTA)( HANDLE hProcess, DWORD64 AddrBase );
  tSFTA pSFTA;

  // SymGetLineFromAddr64()
  typedef BOOL (__stdcall *tSGLFA)( IN HANDLE hProcess, IN DWORD64 dwAddr,
    OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Line );
  tSGLFA pSGLFA;

  // SymGetModuleBase64()
  typedef DWORD64 (__stdcall *tSGMB)( IN HANDLE hProcess, IN DWORD64 dwAddr );
  tSGMB pSGMB;

  // SymGetModuleInfo64()
  typedef BOOL (__stdcall *tSGMI)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT IMAGEHLP_MODULE64_V3 *ModuleInfo );
  tSGMI pSGMI;

  // SymGetOptions()
  typedef DWORD (__stdcall *tSGO)( VOID );
  tSGO pSGO;

  // SymGetSymFromAddr64()
  typedef BOOL (__stdcall *tSGSFA)( IN HANDLE hProcess, IN DWORD64 dwAddr,
    OUT PDWORD64 pdwDisplacement, OUT PIMAGEHLP_SYMBOL64 Symbol );
  tSGSFA pSGSFA;

  // SymInitialize()
  typedef BOOL (__stdcall *tSI)( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );
  tSI pSI;

  // SymLoadModule64()
  typedef DWORD64 (__stdcall *tSLM)( IN HANDLE hProcess, IN HANDLE hFile,
    IN PSTR ImageName, IN PSTR ModuleName, IN DWORD64 BaseOfDll, IN DWORD SizeOfDll );
  tSLM pSLM;

  // SymSetOptions()
  typedef DWORD (__stdcall *tSSO)( IN DWORD SymOptions );
  tSSO pSSO;

  // StackWalk64()
  typedef BOOL (__stdcall *tSW)(
    DWORD MachineType,
    HANDLE hProcess,
    HANDLE hThread,
    LPSTACKFRAME64 StackFrame,
    PVOID ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress );
  tSW pSW;

  // UnDecorateSymbolName()
  typedef DWORD (__stdcall WINAPI *tUDSN)( PCSTR DecoratedName, PSTR UnDecoratedName,
    DWORD UndecoratedLength, DWORD Flags );
  tUDSN pUDSN;

  typedef BOOL (__stdcall WINAPI *tSGSP)(HANDLE hProcess, PSTR SearchPath, DWORD SearchPathLength);
  tSGSP pSGSP;


private:
  // **************************************** ToolHelp32 ************************
  #define MAX_MODULE_NAME32 255
  #define TH32CS_SNAPMODULE   0x00000008
  #pragma pack( push, 8 )
  typedef struct tagMODULEENTRY32
  {
      DWORD   dwSize;
      DWORD   th32ModuleID;       // This module
      DWORD   th32ProcessID;      // owning process
      DWORD   GlblcntUsage;       // Global usage count on the module
      DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
      BYTE  * modBaseAddr;        // Base address of module in th32ProcessID's context
      DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
      HMODULE hModule;            // The hModule of this module in th32ProcessID's context
      char    szModule[MAX_MODULE_NAME32 + 1];
      char    szExePath[MAX_PATH];
  } MODULEENTRY32;
  typedef MODULEENTRY32 *  PMODULEENTRY32;
  typedef MODULEENTRY32 *  LPMODULEENTRY32;
  #pragma pack( pop )

  BOOL GetModuleListTH32(HANDLE hProcess, DWORD pid)
  {
    // CreateToolhelp32Snapshot()
    typedef HANDLE (__stdcall *tCT32S)(DWORD dwFlags, DWORD th32ProcessID);
    // Module32First()
    typedef BOOL (__stdcall *tM32F)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
    // Module32Next()
    typedef BOOL (__stdcall *tM32N)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

    // try both dlls...
    const TCHAR *dllname[] = { _T("kernel32.dll"), _T("tlhelp32.dll") };
    HINSTANCE hToolhelp = NULL;
    tCT32S pCT32S = NULL;
    tM32F pM32F = NULL;
    tM32N pM32N = NULL;

    HANDLE hSnap;
    MODULEENTRY32 me;
    me.dwSize = sizeof(me);
    BOOL keepGoing;
    size_t i;

    for (i = 0; i<(sizeof(dllname) / sizeof(dllname[0])); i++ )
    {
      hToolhelp = LoadLibrary( dllname[i] );
      if (hToolhelp == NULL)
        continue;
      pCT32S = (tCT32S) GetProcAddress(hToolhelp, "CreateToolhelp32Snapshot");
      pM32F = (tM32F) GetProcAddress(hToolhelp, "Module32First");
      pM32N = (tM32N) GetProcAddress(hToolhelp, "Module32Next");
      if ( (pCT32S != NULL) && (pM32F != NULL) && (pM32N != NULL) )
        break; // found the functions!
      FreeLibrary(hToolhelp);
      hToolhelp = NULL;
    }

    if (hToolhelp == NULL)
      return FALSE;

    hSnap = pCT32S( TH32CS_SNAPMODULE, pid );
    if (hSnap == (HANDLE) -1)
    {
      FreeLibrary(hToolhelp);
      return FALSE;
    }

    keepGoing = !!pM32F( hSnap, &me );
    int cnt = 0;
    while (keepGoing)
    {
      this->LoadModule(hProcess, me.szExePath, me.szModule, (DWORD64) me.modBaseAddr, me.modBaseSize);
      cnt++;
      keepGoing = !!pM32N( hSnap, &me );
    }
    CloseHandle(hSnap);
    FreeLibrary(hToolhelp);
    if (cnt <= 0)
      return FALSE;
    return TRUE;
  }  // GetModuleListTH32

  // **************************************** PSAPI ************************
  typedef struct _MODULEINFO {
      LPVOID lpBaseOfDll;
      DWORD SizeOfImage;
      LPVOID EntryPoint;
  } MODULEINFO, *LPMODULEINFO;

  BOOL GetModuleListPSAPI(HANDLE hProcess)
  {
    // EnumProcessModules()
    typedef BOOL (__stdcall *tEPM)(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded );
    // GetModuleFileNameEx()
    typedef DWORD (__stdcall *tGMFNE)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
    // GetModuleBaseName()
    typedef DWORD (__stdcall *tGMBN)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
    // GetModuleInformation()
    typedef BOOL (__stdcall *tGMI)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO pmi, DWORD nSize );

    HINSTANCE hPsapi;
    tEPM pEPM;
    tGMFNE pGMFNE;
    tGMBN pGMBN;
    tGMI pGMI;

    DWORD i;
    //ModuleEntry e;
    DWORD cbNeeded;
    MODULEINFO mi;
    HMODULE *hMods = 0;
    char *tt = NULL;
    char *tt2 = NULL;
    const SIZE_T TTBUFLEN = 8096;
    int cnt = 0;

    hPsapi = LoadLibrary( _T("psapi.dll") );
    if (hPsapi == NULL)
      return FALSE;

    pEPM = (tEPM) GetProcAddress( hPsapi, "EnumProcessModules" );
    pGMFNE = (tGMFNE) GetProcAddress( hPsapi, "GetModuleFileNameExA" );
    pGMBN = (tGMFNE) GetProcAddress( hPsapi, "GetModuleBaseNameA" );
    pGMI = (tGMI) GetProcAddress( hPsapi, "GetModuleInformation" );
    if ( (pEPM == NULL) || (pGMFNE == NULL) || (pGMBN == NULL) || (pGMI == NULL) )
    {
      // we couldn't find all functions
      FreeLibrary(hPsapi);
      return FALSE;
    }

    hMods = (HMODULE*) malloc(sizeof(HMODULE) * (TTBUFLEN / sizeof HMODULE));
    tt = (char*) malloc(sizeof(char) * TTBUFLEN);
    tt2 = (char*) malloc(sizeof(char) * TTBUFLEN);
    if ( (hMods == NULL) || (tt == NULL) || (tt2 == NULL) )
      goto cleanup;

    if ( ! pEPM( hProcess, hMods, TTBUFLEN, &cbNeeded ) )
    {
      //_ftprintf(fLogFile, _T("%lu: EPM failed, GetLastError = %lu\n"), g_dwShowCount, gle );
      goto cleanup;
    }

    if ( cbNeeded > TTBUFLEN )
    {
      //_ftprintf(fLogFile, _T("%lu: More than %lu module handles. Huh?\n"), g_dwShowCount, lenof( hMods ) );
      goto cleanup;
    }

    for ( i = 0; i < cbNeeded / sizeof hMods[0]; i++ )
    {
      // base address, size
      pGMI(hProcess, hMods[i], &mi, sizeof mi );
      // image file name
      tt[0] = 0;
      pGMFNE(hProcess, hMods[i], tt, TTBUFLEN );
      // module name
      tt2[0] = 0;
      pGMBN(hProcess, hMods[i], tt2, TTBUFLEN );

      DWORD dwRes = this->LoadModule(hProcess, tt, tt2, (DWORD64) mi.lpBaseOfDll, mi.SizeOfImage);
      if (dwRes != ERROR_SUCCESS)
        this->m_parent->OnDbgHelpErr("LoadModule", dwRes, 0);
      cnt++;
    }

  cleanup:
    if (hPsapi != NULL) FreeLibrary(hPsapi);
    if (tt2 != NULL) free(tt2);
    if (tt != NULL) free(tt);
    if (hMods != NULL) free(hMods);

    return cnt != 0;
  }  // GetModuleListPSAPI

  DWORD LoadModule(HANDLE hProcess, LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size)
  {
    CHAR *szImg = _strdup(img);
    CHAR *szMod = _strdup(mod);
    DWORD result = ERROR_SUCCESS;
    if ( (szImg == NULL) || (szMod == NULL) )
      result = ERROR_NOT_ENOUGH_MEMORY;
    else
    {
      if (pSLM(hProcess, 0, szImg, szMod, baseAddr, size) == 0)
        result = GetLastError();
    }
    ULONGLONG fileVersion = 0;
    if ( (m_parent != NULL) && (szImg != NULL) )
    {
      // try to retrive the file-version:
      if ( (this->m_parent->m_options & StackWalker::RetrieveFileVersion) != 0)
      {
        VS_FIXEDFILEINFO *fInfo = NULL;
        DWORD dwHandle;
        DWORD dwSize = GetFileVersionInfoSizeA(szImg, &dwHandle);
        if (dwSize > 0)
        {
          LPVOID vData = malloc(dwSize);
          if (vData != NULL)
          {
            if (GetFileVersionInfoA(szImg, dwHandle, dwSize, vData) != 0)
            {
              UINT len;
              TCHAR szSubBlock[] = _T("\\");
              if (VerQueryValue(vData, szSubBlock, (LPVOID*) &fInfo, &len) == 0)
                fInfo = NULL;
              else
              {
                fileVersion = ((ULONGLONG)fInfo->dwFileVersionLS) + ((ULONGLONG)fInfo->dwFileVersionMS << 32);
              }
            }
            free(vData);
          }
        }
      }

      // Retrive some additional-infos about the module
      IMAGEHLP_MODULE64_V3 Module;
      const char *szSymType = "-unknown-";
      if (this->GetModuleInfo(hProcess, baseAddr, &Module) != FALSE)
      {
        switch(Module.SymType)
        {
          case SymNone:
            szSymType = "-nosymbols-";
            break;
          case SymCoff:  // 1
            szSymType = "COFF";
            break;
          case SymCv:  // 2
            szSymType = "CV";
            break;
          case SymPdb:  // 3
            szSymType = "PDB";
            break;
          case SymExport:  // 4
            szSymType = "-exported-";
            break;
          case SymDeferred:  // 5
            szSymType = "-deferred-";
            break;
          case SymSym:  // 6
            szSymType = "SYM";
            break;
          case 7: // SymDia:
            szSymType = "DIA";
            break;
          case 8: //SymVirtual:
            szSymType = "Virtual";
            break;
        }
      }
      LPCSTR pdbName = Module.LoadedImageName;
      if (Module.LoadedPdbName[0] != 0)
        pdbName = Module.LoadedPdbName;
      this->m_parent->OnLoadModule(img, mod, baseAddr, size, result, szSymType, pdbName, fileVersion);
    }
    if (szImg != NULL) free(szImg);
    if (szMod != NULL) free(szMod);
    return result;
  }
public:
  BOOL LoadModules(HANDLE hProcess, DWORD dwProcessId)
  {
    // first try toolhelp32
    if (GetModuleListTH32(hProcess, dwProcessId))
      return true;
    // then try psapi
    return GetModuleListPSAPI(hProcess);
  }


  BOOL GetModuleInfo(HANDLE hProcess, DWORD64 baseAddr, IMAGEHLP_MODULE64_V3 *pModuleInfo)
  {
    memset(pModuleInfo, 0, sizeof(IMAGEHLP_MODULE64_V3));
    if(this->pSGMI == NULL)
    {
      SetLastError(ERROR_DLL_INIT_FAILED);
      return FALSE;
    }
    // First try to use the larger ModuleInfo-Structure
    pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V3);
    void *pData = malloc(4096); // reserve enough memory, so the bug in v6.3.5.1 does not lead to memory-overwrites...
    if (pData == NULL)
    {
      SetLastError(ERROR_NOT_ENOUGH_MEMORY);
      return FALSE;
    }
    memcpy(pData, pModuleInfo, sizeof(IMAGEHLP_MODULE64_V3));
    static bool s_useV3Version = true;
    if (s_useV3Version)
    {
      if (this->pSGMI(hProcess, baseAddr, (IMAGEHLP_MODULE64_V3*) pData) != FALSE)
      {
        // only copy as much memory as is reserved...
        memcpy(pModuleInfo, pData, sizeof(IMAGEHLP_MODULE64_V3));
        pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V3);
        free(pData);
        return TRUE;
      }
      // thomasn: By default, stack walker uses IMAGEHLP_MODULE64_V3 to get module information.
      // However, this may sometimes fail (for instance, on Mono frames), and it would before
      // revert to use IMAGEHLP_MODULE64_V2 in all subsequent cases (as it stored this state in
      // a static). We don't want this behaviour and instead we want to always try using
      // IMAGEHLP_MODULE64_V3 if possible.
      // s_useV3Version = false;
    }

    // could not retrive the bigger structure, try with the smaller one (as defined in VC7.1)...
    pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V2);
    memcpy(pData, pModuleInfo, sizeof(IMAGEHLP_MODULE64_V2));
    if (this->pSGMI(hProcess, baseAddr, (IMAGEHLP_MODULE64_V3*) pData) != FALSE)
    {
      // only copy as much memory as is reserved...
      memcpy(pModuleInfo, pData, sizeof(IMAGEHLP_MODULE64_V2));
      pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V2);
      free(pData);
      return TRUE;
    }
    free(pData);
    SetLastError(ERROR_DLL_INIT_FAILED);
    return FALSE;
  }
};

// #############################################################
StackWalker::StackWalker(DWORD dwProcessId, HANDLE hProcess)
{
  this->m_options = OptionsAll;
  this->m_modulesLoaded = FALSE;
  this->m_hProcess = hProcess;
  this->m_sw = new StackWalkerInternal(this, this->m_hProcess);
  this->m_dwProcessId = dwProcessId;
  this->m_szSymPath = NULL;
  this->m_MaxRecursionCount = 1000;
}
StackWalker::StackWalker(int options, LPCSTR szSymPath, DWORD dwProcessId, HANDLE hProcess)
{
  this->m_options = options;
  this->m_modulesLoaded = FALSE;
  this->m_hProcess = hProcess;
  // By using kMemMemoryProfiler, we ensure that MemoryProfiler will skip this allocation
  this->m_sw =  UNITY_NEW(StackWalkerInternal, kMemMemoryProfiler)(this, this->m_hProcess);
  this->m_dwProcessId = dwProcessId;
  if (szSymPath != NULL)
  {
    this->m_szSymPath = _strdup(szSymPath);
    this->m_options |= SymBuildPath;
  }
  else
    this->m_szSymPath = NULL;
}

StackWalker::~StackWalker()
{
  if (m_szSymPath != NULL)
    free(m_szSymPath);
  m_szSymPath = NULL;
  if (this->m_sw != NULL)
	  UNITY_DELETE(this->m_sw, kMemMemoryProfiler);
  this->m_sw = NULL;
}

/////FIXME: this takes ages on x64; investigate
BOOL StackWalker::LoadModules()
{
  if (this->m_sw == NULL)
  {
    SetLastError(ERROR_DLL_INIT_FAILED);
    return FALSE;
  }
  if (m_modulesLoaded != FALSE)
    return TRUE;

  // Build the sym-path:
  char *szSymPath = NULL;
  if ( (this->m_options & SymBuildPath) != 0)
  {
    const size_t nSymPathLen = 4096;
    szSymPath = (char*) malloc(nSymPathLen);
    if (szSymPath == NULL)
    {
      SetLastError(ERROR_NOT_ENOUGH_MEMORY);
      return FALSE;
    }
    szSymPath[0] = 0;
    // Now first add the (optional) provided sympath:
    if (this->m_szSymPath != NULL)
    {
      strcat_s(szSymPath, nSymPathLen, this->m_szSymPath);
      strcat_s(szSymPath, nSymPathLen, ";");
    }

    strcat_s(szSymPath, nSymPathLen, ".;");

    const size_t nTempLen = 1024;
    char szTemp[nTempLen];
    // Now add the current directory:
    if (GetCurrentDirectoryA(nTempLen, szTemp) > 0)
    {
      szTemp[nTempLen-1] = 0;
      strcat_s(szSymPath, nSymPathLen, szTemp);
      strcat_s(szSymPath, nSymPathLen, ";");
    }

    // Now add the path for the main-module:
    if (GetModuleFileNameA(NULL, szTemp, nTempLen) > 0)
    {
      szTemp[nTempLen-1] = 0;
      for (char *p = (szTemp+strlen(szTemp)-1); p >= szTemp; --p)
      {
        // locate the rightmost path separator
        if ( (*p == '\\') || (*p == '/') || (*p == ':') )
        {
          *p = 0;
          break;
        }
      }  // for (search for path separator...)
      if (strlen(szTemp) > 0)
      {
        strcat_s(szSymPath, nSymPathLen, szTemp);
        strcat_s(szSymPath, nSymPathLen, ";");
      }
    }
    if (GetEnvironmentVariableA("_NT_SYMBOL_PATH", szTemp, nTempLen) > 0)
    {
      szTemp[nTempLen-1] = 0;
      strcat_s(szSymPath, nSymPathLen, szTemp);
      strcat_s(szSymPath, nSymPathLen, ";");
    }
    if (GetEnvironmentVariableA("_NT_ALTERNATE_SYMBOL_PATH", szTemp, nTempLen) > 0)
    {
      szTemp[nTempLen-1] = 0;
      strcat_s(szSymPath, nSymPathLen, szTemp);
      strcat_s(szSymPath, nSymPathLen, ";");
    }
    if (GetEnvironmentVariableA("SYSTEMROOT", szTemp, nTempLen) > 0)
    {
      szTemp[nTempLen-1] = 0;
      strcat_s(szSymPath, nSymPathLen, szTemp);
      strcat_s(szSymPath, nSymPathLen, ";");
      // also add the "system32"-directory:
      strcat_s(szTemp, nTempLen, "\\system32");
      strcat_s(szSymPath, nSymPathLen, szTemp);
      strcat_s(szSymPath, nSymPathLen, ";");
    }

    if ( (this->m_options & SymUseSymSrv) != 0)
    {
      if (GetEnvironmentVariableA("SYSTEMDRIVE", szTemp, nTempLen) > 0)
      {
        szTemp[nTempLen-1] = 0;
        strcat_s(szSymPath, nSymPathLen, "SRV*");
        strcat_s(szSymPath, nSymPathLen, szTemp);
        strcat_s(szSymPath, nSymPathLen, "\\websymbols");
        strcat_s(szSymPath, nSymPathLen, "*http://msdl.microsoft.com/download/symbols;");
      }
      else
        strcat_s(szSymPath, nSymPathLen, "SRV*c:\\websymbols*http://msdl.microsoft.com/download/symbols;");
    }
  }  // if SymBuildPath

  // First Init the whole stuff...
  BOOL bRet = this->m_sw->Init(szSymPath);
  if (szSymPath != NULL) free(szSymPath); szSymPath = NULL;
  if (bRet == FALSE)
  {
    this->OnDbgHelpErr("Error while initializing dbghelp.dll", 0, 0);
    SetLastError(ERROR_DLL_INIT_FAILED);
    return FALSE;
  }

  bRet = this->m_sw->LoadModules(this->m_hProcess, this->m_dwProcessId);
  if (bRet != FALSE)
    m_modulesLoaded = TRUE;
  return bRet;
}

BOOL StackWalker::ShowCallstack(HANDLE hThread, const CONTEXT *context, int maxFrames)
{
	void** frames;
	ALLOC_TEMP_AUTO(frames, maxFrames);

	int totalFrames = 0;
	BOOL res = FALSE;
	res = GetCurrentCallstack(frames, maxFrames, totalFrames, hThread, context);

	// Even if we fail to resolve full stacktrace, print at least those stacktraces we did succeed to resolve
	if (res == FALSE)
	{
		if (totalFrames > 0)
			GetStringFromStacktrace(frames, totalFrames);
		this->OnOutput("<Missing stacktrace information>\n");
		return FALSE;
	}

	return GetStringFromStacktrace(frames, totalFrames);
}

BOOL StackWalker::GetCurrentCallstack(void** frameBuffer, int maxFrames, int& totalFrames, HANDLE hThread, const CONTEXT *context)
{
    CONTEXT c;
    if (context == NULL)
    {
        // If no context is provided, capture the context
        if (hThread == GetCurrentThread())
        {
            GET_CURRENT_CONTEXT(c, USED_CONTEXT_FLAGS);
        }
        else
        {
            SuspendThread(hThread);
            memset(&c, 0, sizeof(CONTEXT));
            c.ContextFlags = USED_CONTEXT_FLAGS;
            if (GetThreadContext(hThread, &c) == FALSE)
            {
                ResumeThread(hThread);
                return FALSE;
            }
        }
    }
    else
        c = *context;

    bool success;
    if (GetCurrentProcess() == m_hProcess)
        success = GetCurrentCallstackInProcess(frameBuffer, maxFrames, totalFrames, hThread, &c);
    else
        success = GetCurrentCallstackOutOfProcess(frameBuffer, maxFrames, totalFrames, hThread, &c);

    if (context == NULL && hThread != GetCurrentThread())
        ResumeThread(hThread);

    return success;
}

#if defined(_M_IX86)
BOOL StackWalker::GetCurrentCallstackInProcess(void** frameBuffer, int maxFrames, int& totalFrames, HANDLE hThread, const CONTEXT *context)
{
    CONTEXT c = *context;
	int frameNum;
	BOOL success = TRUE;
	totalFrames = 0;

	// sometimes IP is NULL when crashing, retrieve values from previous frame
	if (!c.Eip)
	{
		c.Eip = ((DWORD*)c.Ebp)[1];
		c.Ebp = ((DWORD*)c.Ebp)[0];
	}

	for (frameNum = 0; frameNum < maxFrames; ++frameNum)
	{
		// If the IP is zero, we've reached the end of the call stack.
		if (!c.Eip)
			break;

		frameBuffer[frameNum] = (void*)c.Eip;

		c.Eip = ((DWORD*)c.Ebp)[1];
		c.Ebp = ((DWORD*)c.Ebp)[0];
	}

	totalFrames = frameNum;

	return success;
}

static bool MoveToNextStackFrameOoP(HANDLE hProcess, CONTEXT& c)
{
    const DWORD* ebp = reinterpret_cast<const DWORD*>(c.Ebp);
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(ebp + 1), &c.Eip, sizeof(c.Eip), nullptr))
        return false;
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(ebp), &c.Ebp, sizeof(c.Ebp), nullptr))
        return false;
    return true;
}

BOOL StackWalker::GetCurrentCallstackOutOfProcess(void** frameBuffer, int maxFrames, int& totalFrames, HANDLE hThread, const CONTEXT *context)
{
    CONTEXT c = *context;
	int frameNum;
	BOOL success = TRUE;
	totalFrames = 0;

	// sometimes IP is NULL when crashing, retrieve values from previous frame
	if (!c.Eip)
	{
        if (!MoveToNextStackFrameOoP(this->m_hProcess, c))
            return false;
	}

	for (frameNum = 0; frameNum < maxFrames; ++frameNum)
	{
		// If the IP is zero, we've reached the end of the call stack.
		if (!c.Eip)
			break;

		frameBuffer[frameNum] = (void*)c.Eip;

        if (!MoveToNextStackFrameOoP(this->m_hProcess, c))
        {
            totalFrames = frameNum + 1;
            return false;
        }
	}

	totalFrames = frameNum;

	return success;
}
#endif

#if defined(_M_X64)
BOOL StackWalker::GetCurrentCallstackInProcess(void** frameBuffer, int maxFrames, int& totalFrames, HANDLE hThread, const CONTEXT *context)
{
    CONTEXT c = *context;
    int frameNum;
    BOOL success = TRUE;
    totalFrames = 0;

    DWORD64 currentFunctionImageBase = 0;
    // check if we are a leaf function and adjust accordingly
    if (!RtlLookupFunctionEntry(c.Rip, &currentFunctionImageBase, NULL))
    {
        // https://msdn.microsoft.com/library/8ydc79k6(v=vs.100).aspx
        // If no function table entry is found, then it is in a leaf function, and RSP will directly address the return pointer.
        // The return pointer at [RSP] is stored in the updated context, the simulated RSP is incremented by 8, and step 1 is repeated.
        c.Rip = *(PULONG64)c.Rsp;
        c.Rsp += 8;
    }
    UNWIND_HISTORY_TABLE unwindHistoryTable;
    memset(&unwindHistoryTable, 0, sizeof(UNWIND_HISTORY_TABLE));

    for (frameNum = 0; frameNum < maxFrames; ++frameNum)
    {
        // If the IP is zero, we've reached the end of the call stack.
        if (!c.Rip)
            break;

        PRUNTIME_FUNCTION currentFunction = RtlLookupFunctionEntry(c.Rip, &currentFunctionImageBase, &unwindHistoryTable);
        // Case 696112. Workaround a rare crash until we get a 100% repro. Passing NULL currentFunction to RtlVirtualUnwind will cause a crash.
        //				It's not clear when RtlLookupFunctionEntry fails, but when it does it seems to happen right after JIT'ed stack frame.
        //				For ex.,
        //				...
        //				0x0000000147FC9ACF (Mono JIT Code) [LoggingDifferentStacktraceWorksScript.cs:43] LoggingDifferentStacktraceWorksScript:Start ()
        //				0x0000000116FC83B2 (Mono JIT Code) (wrapper runtime-invoke) object:runtime_invoke_void__this__ (object,intptr,intptr,intptr)
        //				<RtlLookupFunctionEntry will return NULL sometimes here>, the correct behavior would be to return function for
        //				0x00007FFA83DF3E53 (mono) mono_set_defaults
        //				0x00007FFA83D482C5 (mono) mono_runtime_invoke
        //				...
        if (currentFunction == NULL)
        {
            this->OnOutput("RtlLookupFunctionEntry returned NULL function. Aborting stack walk.\n");
            success = FALSE;
            break;
        }

        frameBuffer[frameNum] = (void*)c.Rip;

        void* handlerData;
        DWORD64 establisherFrame;
        RtlVirtualUnwind(
            0,
            currentFunctionImageBase,
            c.Rip,
            currentFunction,
            &c,
            &handlerData,
            &establisherFrame,
            NULL
        );
    }

    totalFrames = frameNum;

    return success;
}

BOOL StackWalker::GetCurrentCallstackOutOfProcess(void** frameBuffer, int maxFrames, int& totalFrames, HANDLE hThread, const CONTEXT *context)
{
    CONTEXT c = *context;
    int frameNum;
    int curRecursionCount = 0;

    // init STACKFRAME for first call
    STACKFRAME64 s; // in/out stackframe
    memset(&s, 0, sizeof(s));
    DWORD imageType;
    imageType = IMAGE_FILE_MACHINE_AMD64;
    s.AddrPC.Offset = c.Rip;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrFrame.Offset = c.Rsp;
    s.AddrFrame.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.Rsp;
    s.AddrStack.Mode = AddrModeFlat;

    for (frameNum = 0; frameNum < maxFrames; ++frameNum)
    {
        // get next stack frame (StackWalk64(), SymFunctionTableAccess64(), SymGetModuleBase64())
        // if this returns ERROR_INVALID_ADDRESS (487) or ERROR_NOACCESS (998), you can
        // assume that either you are done, or that the stack is so hosed that the next
        // deeper frame could not be found.
        // CONTEXT need not to be suplied if imageTyp is IMAGE_FILE_MACHINE_I386!
        if (!this->m_sw->pSW(imageType, this->m_hProcess, hThread, &s, &c, myReadProcMem, this->m_sw->pSFTA, this->m_sw->pSGMB, NULL))
        {
            break;
        }

        if (s.AddrPC.Offset == s.AddrReturn.Offset)
        {
            if ((this->m_MaxRecursionCount > 0) && (curRecursionCount > m_MaxRecursionCount))
            {
                this->OnDbgHelpErr("Endless Callstack!", 0, s.AddrPC.Offset);
                break;
            }
            curRecursionCount++;
        }
        else
            curRecursionCount = 0;

        if (s.AddrPC.Offset != 0)
        {
            frameBuffer[frameNum] = reinterpret_cast<void*>(s.AddrPC.Offset);
        }
    }

    totalFrames = frameNum;

    return TRUE;
}
#endif

#if defined(_M_ARM64)
BOOL StackWalker::GetCurrentCallstackInProcess(void** frameBuffer, int maxFrames, int& totalFrames, HANDLE hThread, const CONTEXT *context)
{
    CONTEXT c = *context;
    int frameNum;
    BOOL success = TRUE;
    totalFrames = 0;

    DWORD64 currentFunctionImageBase = 0;
    // check if we are a leaf function and adjust accordingly
    if (!RtlLookupFunctionEntry(c.Pc, &currentFunctionImageBase, NULL))
    {
        // If no function table entry is found, then it is in a leaf function, and Lr register will contain the return address
        c.Pc = c.Lr;
    }
    UNWIND_HISTORY_TABLE unwindHistoryTable;
    memset(&unwindHistoryTable, 0, sizeof(UNWIND_HISTORY_TABLE));

    for (frameNum = 0; frameNum < maxFrames; ++frameNum)
    {
        // If the IP is zero, we've reached the end of the call stack.
        if (!c.Pc)
            break;

        PRUNTIME_FUNCTION currentFunction = RtlLookupFunctionEntry(c.Pc, &currentFunctionImageBase, &unwindHistoryTable);
        if (currentFunction == NULL)
        {
            this->OnOutput("RtlLookupFunctionEntry returned NULL function. Aborting stack walk.\n");
            success = FALSE;
            break;
        }

        frameBuffer[frameNum] = (void*)c.Pc;

        void* handlerData;
        DWORD64 establisherFrame;
        RtlVirtualUnwind(
            0,
            currentFunctionImageBase,
            c.Pc,
            currentFunction,
            &c,
            &handlerData,
            &establisherFrame,
            NULL
        );
    }

    totalFrames = frameNum;

    return success;
}

BOOL StackWalker::GetCurrentCallstackOutOfProcess(void** frameBuffer, int maxFrames, int& totalFrames, HANDLE hThread, const CONTEXT *context)
{
    CONTEXT c = *context;
    int frameNum;
    int curRecursionCount = 0;

    // init STACKFRAME for first call
    STACKFRAME64 s; // in/out stackframe
    memset(&s, 0, sizeof(s));
    DWORD imageType;
    imageType = IMAGE_FILE_MACHINE_ARM64;
    s.AddrPC.Offset = c.Pc;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrFrame.Offset = c.Sp;
    s.AddrFrame.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.Sp;
    s.AddrStack.Mode = AddrModeFlat;

    for (frameNum = 0; frameNum < maxFrames; ++frameNum)
    {
        // get next stack frame (StackWalk64(), SymFunctionTableAccess64(), SymGetModuleBase64())
        // if this returns ERROR_INVALID_ADDRESS (487) or ERROR_NOACCESS (998), you can
        // assume that either you are done, or that the stack is so hosed that the next
        // deeper frame could not be found.
        // CONTEXT need not to be suplied if imageTyp is IMAGE_FILE_MACHINE_I386!
        if (!this->m_sw->pSW(imageType, this->m_hProcess, hThread, &s, &c, myReadProcMem, this->m_sw->pSFTA, this->m_sw->pSGMB, NULL))
        {
            break;
        }

        if (s.AddrPC.Offset == s.AddrReturn.Offset)
        {
            if ((this->m_MaxRecursionCount > 0) && (curRecursionCount > m_MaxRecursionCount))
            {
                this->OnDbgHelpErr("Endless Callstack!", 0, s.AddrPC.Offset);
                break;
            }
            curRecursionCount++;
        }
        else
            curRecursionCount = 0;

        if (s.AddrPC.Offset != 0)
        {
            frameBuffer[frameNum] = reinterpret_cast<void*>(s.AddrPC.Offset);
        }
    }

    totalFrames = frameNum;

    return TRUE;
}
#endif

BOOL StackWalker::GetStringFromStacktrace(const void* const* stack, int frames)
{
	int maxFrames = frames;

	CallstackEntry csEntry;
	IMAGEHLP_SYMBOL64 *pSym = NULL;
	StackWalkerInternal::IMAGEHLP_MODULE64_V3 Module;
	IMAGEHLP_LINE64 Line;
	int frameNum;
	bool bLastEntryCalled = true;

	if (m_modulesLoaded == FALSE)
		this->LoadModules();  // ignore the result...

	if (this->m_sw->m_hDbhHelp == NULL)
	{
		SetLastError(ERROR_DLL_INIT_FAILED);
		return FALSE;
	}

	// Walking the stack is a bit tricky for us since we have Mono frames sitting right there
	// in the middle throwing dbghelp off its tracks (both in 32bits and 64bits).
	//
	// On 32bits, we solve this by simply relying on frame pointers never being omitted.  This
	// happens to be true as we explicitly never do it for the runtime and we always enable
	// Mono debugging (which in turn disables Mono frame pointer optimizations) for the executables
	// using this code here.
	//
	// On 64bits, we have a better path by simply relying on the unwind table data.  For some
	// reason, RtlCaptureStackBackTrace does not take dynamic table data (i.e. Mono's stuff)
	// into account so we manually unwind using RtlVirtualUnwind here.  This works quite nicely.

	pSym = (IMAGEHLP_SYMBOL64 *)malloc(sizeof(IMAGEHLP_SYMBOL64) + STACKWALK_MAX_NAMELEN);
	if (!pSym) goto cleanup;  // not enough memory...
	memset(pSym, 0, sizeof(IMAGEHLP_SYMBOL64) + STACKWALK_MAX_NAMELEN);
	pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	pSym->MaxNameLength = STACKWALK_MAX_NAMELEN;

	memset(&Line, 0, sizeof(Line));
	Line.SizeOfStruct = sizeof(Line);

	memset(&Module, 0, sizeof(Module));
	Module.SizeOfStruct = sizeof(Module);

	for (frameNum = 0; frameNum < maxFrames; ++frameNum)
	{
		DWORD64 AddrPCOffset = (DWORD64) stack[frameNum];

		//We reached the end.
		if (AddrPCOffset == 0)
			break;


		csEntry.offset = AddrPCOffset;
		csEntry.name[0] = 0;
		csEntry.undName[0] = 0;
		csEntry.undFullName[0] = 0;
		csEntry.offsetFromSmybol = 0;
		csEntry.offsetFromLine = 0;
		csEntry.lineFileName[0] = 0;
		csEntry.lineNumber = 0;
		csEntry.loadedImageName[0] = 0;
		csEntry.moduleName[0] = 0;
		csEntry.isManaged = FALSE;
		csEntry.moduleGUID[0] = 0;
		csEntry.pdbName[0] = 0;


		// Find out whether this is a managed method.
		bool isManagedMethod = false;
		isManagedMethod = TryGetManagedStackFrame(AddrPCOffset, csEntry);

		// If it's not a managed method, let dbghelp find all the data we need.
		if (!isManagedMethod)
		{
			// we seem to have a valid PC
			// show procedure info (SymGetSymFromAddr64())
			if (this->m_sw->pSGSFA(this->m_hProcess, AddrPCOffset, &(csEntry.offsetFromSmybol), pSym) != FALSE)
			{
				MyStrCpy(csEntry.name, STACKWALK_MAX_NAMELEN, pSym->Name);
				// UnDecorateSymbolName()
				this->m_sw->pUDSN(pSym->Name, csEntry.undName, STACKWALK_MAX_NAMELEN, UNDNAME_NAME_ONLY);
				this->m_sw->pUDSN(pSym->Name, csEntry.undFullName, STACKWALK_MAX_NAMELEN, UNDNAME_COMPLETE);
			}
			else
			{
				this->OnDbgHelpErr("SymGetSymFromAddr64", GetLastError(), AddrPCOffset);
			}

			// show line number info, NT5.0-method (SymGetLineFromAddr64())
			if (this->m_sw->pSGLFA != NULL)
			{ // yes, we have SymGetLineFromAddr64()
				if (this->m_sw->pSGLFA(this->m_hProcess, AddrPCOffset, &(csEntry.offsetFromLine), &Line) != FALSE)
				{
					csEntry.lineNumber = Line.LineNumber;
					MyStrCpy(csEntry.lineFileName, STACKWALK_MAX_NAMELEN, Line.FileName);
				}
				// Don't give an error here.  The SymGetLineFromAddr64 will fail if there simply
				// is no source information (like, for example, for kernel stuff) which shouldn't
				// make us spill errors.
				//else
				//{
				//  this->OnDbgHelpErr("SymGetLineFromAddr64", GetLastError(), AddrPCOffset);
				//}
			} // yes, we have SymGetLineFromAddr64()

			// show module info (SymGetModuleInfo64())
			if (this->m_sw->GetModuleInfo(this->m_hProcess, AddrPCOffset, &Module) != FALSE)
			{ // got module info OK
				switch (Module.SymType)
				{
				case SymNone:
					csEntry.symTypeString = "-nosymbols-";
					break;
				case SymCoff:
					csEntry.symTypeString = "COFF";
					break;
				case SymCv:
					csEntry.symTypeString = "CV";
					break;
				case SymPdb:
					csEntry.symTypeString = "PDB";
					break;
				case SymExport:
					csEntry.symTypeString = "-exported-";
					break;
				case SymDeferred:
					csEntry.symTypeString = "-deferred-";
					break;
				case SymSym:
					csEntry.symTypeString = "SYM";
					break;
#if API_VERSION_NUMBER >= 9
				case SymDia:
					csEntry.symTypeString = "DIA";
					break;
#endif
				case 8: //SymVirtual:
					csEntry.symTypeString = "Virtual";
					break;
				default:
					//_snprintf( ty, sizeof ty, "symtype=%ld", (long) Module.SymType );
					csEntry.symTypeString = NULL;
					break;
				}

				MyStrCpy(csEntry.moduleName, STACKWALK_MAX_NAMELEN, Module.ModuleName);
				csEntry.baseOfImage = Module.BaseOfImage;
				MyStrCpy(csEntry.loadedImageName, STACKWALK_MAX_NAMELEN, Module.LoadedImageName);
				MyStrCpy(csEntry.pdbName, STACKWALK_MAX_NAMELEN, Module.CVData);
				// Crash reporting expects the GUID without dashes (the format used by symbol servers)
				_snprintf_s(csEntry.moduleGUID, ARRAY_SIZE(csEntry.moduleGUID)-1, "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%x",
					(unsigned char)(Module.PdbSig70.Data1 >> 24),
					(unsigned char)(Module.PdbSig70.Data1 >> 16),
					(unsigned char)(Module.PdbSig70.Data1 >> 8),
					(unsigned char)Module.PdbSig70.Data1,
					(unsigned char)(Module.PdbSig70.Data2 >> 8),
					(unsigned char)Module.PdbSig70.Data2,
					(unsigned char)(Module.PdbSig70.Data3 >> 8),
					(unsigned char)Module.PdbSig70.Data3,
					Module.PdbSig70.Data4[0], Module.PdbSig70.Data4[1], Module.PdbSig70.Data4[2], Module.PdbSig70.Data4[3],
					Module.PdbSig70.Data4[4], Module.PdbSig70.Data4[5], Module.PdbSig70.Data4[6], Module.PdbSig70.Data4[7],
					Module.PdbAge);
			} // got module info OK
			else
			{
				this->OnDbgHelpErr("SymGetModuleInfo64", GetLastError(), AddrPCOffset);
			}
		} // we seem to have a valid PC

		CallstackEntryType et = nextEntry;
		if (frameNum == 0)
			et = firstEntry;
		bLastEntryCalled = false;
		this->OnCallstackEntry(et, csEntry);

	} // for ( frameNum )

cleanup:
	if (pSym) free(pSym);

	if (bLastEntryCalled == false)
		this->OnCallstackEntry(lastEntry, csEntry);

	return TRUE;
}


// The following is used to pass the "userData"-Pointer to the user-provided readMemoryFunction
// This has to be done due to a problem with the "hProcess"-parameter in x64...
// Because this class is in no case multi-threading-enabled (because of the limitations
// of dbghelp.dll) it is "safe" to use a static-variable
static StackWalker::PReadProcessMemoryRoutine s_readMemoryFunction = NULL;
static LPVOID s_readMemoryFunction_UserData = NULL;

BOOL __stdcall StackWalker::myReadProcMem(
    HANDLE      hProcess,
    DWORD64     qwBaseAddress,
    PVOID       lpBuffer,
    DWORD       nSize,
    LPDWORD     lpNumberOfBytesRead
    )
{
  if (s_readMemoryFunction == NULL)
  {
    SIZE_T st;
    BOOL bRet = ReadProcessMemory(hProcess, (LPVOID) qwBaseAddress, lpBuffer, nSize, &st);
    *lpNumberOfBytesRead = (DWORD) st;
    //printf("ReadMemory: hProcess: %p, baseAddr: %p, buffer: %p, size: %d, read: %d, result: %d\n", hProcess, (LPVOID) qwBaseAddress, lpBuffer, nSize, (DWORD) st, (DWORD) bRet);
    return bRet;
  }
  else
  {
    return s_readMemoryFunction(hProcess, qwBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead, s_readMemoryFunction_UserData);
  }
}

void StackWalker::OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion)
{
  CHAR buffer[STACKWALK_MAX_NAMELEN];
  if (fileVersion == 0)
    _snprintf_s(buffer, STACKWALK_MAX_NAMELEN-1, "%s:%s (%p), size: %d (result: %d), SymType: '%s', PDB: '%s'\n", img, mod, (LPVOID) baseAddr, size, result, symType, pdbName);
  else
  {
    DWORD v4 = (DWORD) (fileVersion & 0xFFFF);
    DWORD v3 = (DWORD) ((fileVersion>>16) & 0xFFFF);
    DWORD v2 = (DWORD) ((fileVersion>>32) & 0xFFFF);
    DWORD v1 = (DWORD) ((fileVersion>>48) & 0xFFFF);
    _snprintf_s(buffer, STACKWALK_MAX_NAMELEN-1, "%s:%s (%p), size: %d (result: %d), SymType: '%s', PDB: '%s', fileVersion: %d.%d.%d.%d\n", img, mod, (LPVOID) baseAddr, size, result, symType, pdbName, v1, v2, v3, v4);
  }
  OnOutput(buffer);
}

void StackWalker::OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry)
{
	__FAKEABLE_METHOD__(StackWalker, OnCallstackEntry, (eType, entry));

  CHAR buffer[STACKWALK_MAX_NAMELEN];
  if ( (eType != lastEntry) && (entry.offset != 0) )
  {
    if (entry.name[0] == 0)
      MyStrCpy(entry.name, STACKWALK_MAX_NAMELEN, "(function-name not available)");
    if (entry.undName[0] != 0)
      MyStrCpy(entry.name, STACKWALK_MAX_NAMELEN, entry.undName);
    if (entry.undFullName[0] != 0)
      MyStrCpy(entry.name, STACKWALK_MAX_NAMELEN, entry.undFullName);
    if (entry.moduleName[0] == 0)
      MyStrCpy(entry.moduleName, STACKWALK_MAX_NAMELEN, "(<unknown>)");

    if (entry.lineFileName[0] != 0)
		_snprintf_s(buffer, STACKWALK_MAX_NAMELEN-1, "0x%p (%s) [%s:%d] %s \n", (LPVOID) entry.offset, entry.moduleName, entry.lineFileName, entry.lineNumber, entry.name);
	else
		_snprintf_s(buffer, STACKWALK_MAX_NAMELEN-1, "0x%p (%s) %s\n", (LPVOID) entry.offset, entry.moduleName, entry.name);

	buffer[STACKWALK_MAX_NAMELEN-1] = 0;
	OnOutput(buffer);
  }
}

void StackWalker::OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr)
{
  CHAR buffer[STACKWALK_MAX_NAMELEN];
  _snprintf_s(buffer, STACKWALK_MAX_NAMELEN-1, "ERROR: %s, GetLastError: %d (Address: %p)\n", szFuncName, gle, (LPVOID) addr);
  OnOutput(buffer);
}

void StackWalker::OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName)
{
  CHAR buffer[STACKWALK_MAX_NAMELEN];
  _snprintf_s(buffer, STACKWALK_MAX_NAMELEN-1, "SymInit: Symbol-SearchPath: '%s', symOptions: %d, UserName: '%s'\n", szSearchPath, symOptions, szUserName);
  OnOutput(buffer);
  // Also display the OS-version
  OSVERSIONINFOW ver;
  ZeroMemory(&ver, sizeof(OSVERSIONINFOW));
  ver.dwOSVersionInfoSize = sizeof(ver);
#if PLATFORM_METRO
  _snprintf_s(buffer, STACKWALK_MAX_NAMELEN-1, "OS-Version: Unknown\n");
  OnOutput(buffer);
#else
  if (GetWindowsVersionImpl(&ver))
  {
    _snprintf_s(buffer, STACKWALK_MAX_NAMELEN-1, "OS-Version: %d.%d.%d\n", ver.dwMajorVersion, ver.dwMinorVersion, ver.dwBuildNumber);
    OnOutput(buffer);
  }
#endif
}

void StackWalker::OnOutput(LPCSTR buffer)
{
	__FAKEABLE_METHOD__(StackWalker, OnOutput, (buffer));
  OutputDebugStringA(buffer);
}

BOOL StackWalker::TryGetManagedStackFrame(DWORD_PTR frameAddress, CallstackEntry &callstackEntry)
{
#if !ENABLE_MONO || USE_CONSOLEBRANCH_MONO || UNITY_EXTERNAL_TOOL || NO_MONO_STACKTRACE
	return false;
#else
	ManagedStackFrame managedStackFrame;
    bool isManagedMethod = TryGetManagedStackFrameDetails((void*)frameAddress, managedStackFrame);
	if (isManagedMethod)
	{
		MyStrCpy(callstackEntry.name, STACKWALK_MAX_NAMELEN, managedStackFrame.methodName.c_str());
		MyStrCpy(callstackEntry.undFullName, STACKWALK_MAX_NAMELEN, managedStackFrame.signature.c_str());
		MyStrCpy(callstackEntry.moduleName, STACKWALK_MAX_NAMELEN, "Mono JIT Code");
		MyStrCpy(callstackEntry.lineFileName, STACKWALK_MAX_NAMELEN, managedStackFrame.sourceFile.c_str());
		callstackEntry.lineNumber = managedStackFrame.lineNumber;
	}

	return isManagedMethod;
#endif
}

#endif
