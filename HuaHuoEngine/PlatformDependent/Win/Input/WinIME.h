#pragma once

#include "NewInput.h"

bool ProcessIMEMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT &result, win::NewInput* newInput);
