#include "UnityPrefix.h"
#include "PlatformDependent/Win/Handle.h"
#include "External/Unicode/UTF8.h"
#include "Runtime/Input/InputManager.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Misc/PlayerSettings.h"
#include "Runtime/BaseClasses/ManagerContext.h"
#if UNITY_EDITOR
#include "Editor/Platform/Interface/GUIView.h"
static GUIView* g_LastView = NULL;
#endif
#include "Runtime/Input/GUIEventManager.h"
#include <windows.h>
#include "NewInput.h"

void ProcessMessageForInput(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

#define kTextMenuOffset 30

void AddWCharToInputString(WCHAR wide)
{
    CHAR utf8[5] = {};

    if (0 != WideCharToMultiByte(CP_UTF8, 0, &wide, 1, utf8, sizeof(utf8), NULL, NULL))
    {
        GetInputManager().GetInputString() += utf8;
    }
}

bool ProcessIMEMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT &result, win::NewInput* newInput)
{
    win::ImmGetContextHandle hImc(hWnd, ImmGetContext(hWnd));

    if (GetManagerPtrFromContext(ManagerContext::kInputManager) == NULL)
        return false;

    if (!GetInputManager().GetEnableIMEComposition())
    {
        if (hImc.IsOpen())
        {
            hImc.Close();
            ImmAssociateContext(hWnd, NULL);
        }
    }
    else
    {
        if (!hImc.IsOpen())
        {
            ImmAssociateContextEx(hWnd, NULL, IACE_DEFAULT);
            hImc.Attach(hWnd, ImmGetContext(hWnd));
        }
    }

    switch (message)
    {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            int numBytes = ImmGetCompositionStringW(hImc, GCS_COMPSTR, NULL, 0);
            if (numBytes > 0)
            {
                std::vector<UInt16> utf16;
                utf16.resize(numBytes / 2);
                ImmGetCompositionStringW(hImc, GCS_COMPSTR, &(utf16[0]), numBytes);

                for (int i = 0; i < utf16.size(); ++i)
                {
                    if (newInput != NULL)
                    {
                        for (int i = 0; i < utf16.size(); ++i)
                            QueueTextInputEvent(kInputEventText, newInput->GetKeyboardId(), win::GetCurrentEventTimeInUnityTime(), utf16[i]);
                    }
                }

                // Notify IME that composition results should be processed and composition ended.
                ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
                char empty;
                ImmSetCompositionStringW(hImc, SCS_SETSTR, &empty, 0, &empty, 0);
            }
            return false;
    }

    #if UNITY_EDITOR
    GUIView *view = (GUIView*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (view != GetKeyGUIView())
        return false;
    g_LastView = view;
    #endif

    switch (message)
    {
        case WM_IME_STARTCOMPOSITION:
            result = 0;
            return true;

        case WM_IME_COMPOSITION:
        {
            int numBytes = ImmGetCompositionStringW(hImc, GCS_COMPSTR, NULL, 0);
            if (numBytes > 0)
            {
                dynamic_array<UInt16> utf16(kMemTempAlloc);
                utf16.resize_uninitialized(numBytes / 2);
                ImmGetCompositionStringW(hImc, GCS_COMPSTR, utf16.begin(), numBytes);

                ConvertUTF16toUTF8(utf16, GetInputManager().GetCompositionString());

                if (newInput != NULL)
                {
                    ImeCompositionInputEventData::QueueEvent(newInput->GetKeyboardId(), win::GetCurrentEventTimeInUnityTime(), utf16.data(), utf16.size());

                    ImeCompositionStringInputEventData oldEvent(newInput->GetKeyboardId(), win::GetCurrentEventTimeInUnityTime(), utf16);
                    QueueInputEvent(oldEvent);
                }

                // Send a KeyDown event with character == 0. This allows input text fields which require an event
                // to refresh to update properly. This includes GUI.TextField in the Editor (standalone is refreshed each frame)
                // and UI.InputFields
                // See case 711719
                InputEvent ie;
                ie.Init();
                ie.character = 0;
                ie.type = InputEvent::kKeyDown;
                #if UNITY_EDITOR
                view->ProcessInputEvent(ie);
                #else
                GetGUIEventManager().QueueEvent(ie);
                #endif
            }

            if (lParam & GCS_RESULTSTR)
            {
                int numBytes = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, NULL, 0);
                if (numBytes > 0)
                {
                    std::vector<UInt16> utf16;
                    utf16.resize(numBytes / 2);
                    ImmGetCompositionStringW(hImc, GCS_RESULTSTR, &(utf16[0]), numBytes);

                    for (int i = 0; i < utf16.size(); ++i)
                    {
                        #if UNITY_EDITOR
                        view->ProcessEventMessages(WM_CHAR, (WPARAM)utf16[i], 0);
                        #else
                        ProcessMessageForInput(hWnd, WM_CHAR, (WPARAM)utf16[i], 0);
                        #endif

                        if (!GetPlayerSettings().GetDisableOldInputManagerSupport())
                            AddWCharToInputString(utf16[i]);
                        if (newInput != NULL)
                            QueueTextInputEvent(kInputEventText, newInput->GetKeyboardId(), win::GetCurrentEventTimeInUnityTime(), utf16[i]);
                    }
                }
            }
        }
            result = 0;
            return true;

        case WM_IME_ENDCOMPOSITION:
        {
            GetInputManager().GetCompositionString().clear();

            if (newInput != NULL)
            {
                ImeCompositionInputEventData::QueueEvent(newInput->GetKeyboardId(), win::GetCurrentEventTimeInUnityTime(), NULL, 0);

                ImeCompositionStringInputEventData eventData(newInput->GetKeyboardId(), win::GetCurrentEventTimeInUnityTime());
                QueueInputEvent(eventData);
            }

            // Send a KeyDown event with character == 0. This allows input text fields which require an event
            // to refresh to update properly. This includes GUI.TextField in the Editor (standalone is refreshed each frame)
            // and UI.InputFields
            InputEvent ie;
            ie.Init();
            ie.character = 0;
            ie.type = InputEvent::kKeyDown;
            #if UNITY_EDITOR
            view->ProcessInputEvent(ie);
            #else
            GetGUIEventManager().QueueEvent(ie);
            #endif
        }
            return false;

        case WM_IME_REQUEST:
        {
            switch (wParam)
            {
                case IMR_QUERYCHARPOSITION:
                    IMECHARPOSITION* chpos = (IMECHARPOSITION*)lParam;
                    chpos->pt.x = GetInputManager().GetTextFieldCursorPos().x;
                    chpos->pt.y = GetInputManager().GetTextFieldCursorPos().y;
                    RECT rect;
                    GetWindowRect(hWnd, &rect);
                    chpos->pt.x += rect.left;
                    chpos->pt.y += rect.top + kTextMenuOffset;
                    result = true;
                    return true;
            }
        }
            return false;

        case WM_IME_NOTIFY:
            if (wParam == IMN_SETCONVERSIONMODE)
            {
                DWORD dwConversion;
                DWORD dwSentence;

                ImmGetConversionStatus(hImc, &dwConversion, &dwSentence);
                bool isIMESelected = ((dwConversion & IME_CMODE_NATIVE) != 0);
                GetInputManager().SetIMEIsSelected(isIMESelected);
                if (newInput != NULL)
                    newInput->SetKeyboardIMEIsSelected(isIMESelected);
            }

            if (GetScreenManager().IsFullscreen())
            {
                switch (wParam)
                {
                    case IMN_OPENCANDIDATE:
                    case IMN_CHANGECANDIDATE:
                    case IMN_CLOSECANDIDATE:
                        result = 0;
                        return true;
                }
            }
            break;
    }
    return false;
}
