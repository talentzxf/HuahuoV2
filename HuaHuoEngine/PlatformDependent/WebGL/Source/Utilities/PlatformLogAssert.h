#pragma once

#define UNITY_PLATFORM_PRINTF_CONSOLE_VALIST(type, log, list) PP_WRAP_CODE(WebGLPrintfConsolev(type, log, list); va_end(list); return;)
void WebGLPrintfConsolev(LogType logType, const char* log, va_list list);
