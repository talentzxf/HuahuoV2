#pragma once

#include <windef.h>
#include <winuser.h>

#if (WINVER < 0x0601)
// From WinUser.h
#define WM_TOUCH                    0x0240
#define WM_GESTURE                  0x0119
#define WM_GESTURENOTIFY            0x011A

#define TOUCHEVENTF_MOVE            0x0001
#define TOUCHEVENTF_DOWN            0x0002
#define TOUCHEVENTF_UP              0x0004
#define TOUCHEVENTF_PRIMARY         0x0010

#define TOUCHINPUTMASKF_CONTACTAREA 0x0004

/*
 * RegisterTouchWindow flag values
 */
#define TWF_FINETOUCH       (0x00000001)
#define TWF_WANTPALM        (0x00000002)

#define SM_DIGITIZER          94

#define NID_MULTI_INPUT       0x00000040
#define NID_READY             0x00000080

/*
 * Touch input handle
 */
DECLARE_HANDLE(HTOUCHINPUT);

typedef struct tagTOUCHINPUT
{
    LONG x;
    LONG y;
    HANDLE hSource;
    DWORD dwID;
    DWORD dwFlags;
    DWORD dwMask;
    DWORD dwTime;
    ULONG_PTR dwExtraInfo;
    DWORD cxContact;
    DWORD cyContact;
} TOUCHINPUT, *PTOUCHINPUT;
typedef TOUCHINPUT const * PCTOUCHINPUT;

#endif

extern BOOL WinGetUserTouchInputInfo(
    __in HTOUCHINPUT hTouchInput,               // input event handle; from touch message lParam
    __in UINT cInputs,                          // number of elements in the array
    __out_ecount(cInputs) PTOUCHINPUT pInputs,  // array of touch inputs
    __in int cbSize);                           // sizeof(TOUCHINPUT)
extern BOOL WinCloseUserTouchInputHandle(__in HTOUCHINPUT hTouchInput);
