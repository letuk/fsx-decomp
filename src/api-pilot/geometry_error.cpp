#include <windows.h>
#include <stdarg.h>
extern "C" int __stdcall LoadSplashStringPilot(unsigned int, char*, int);
// Original formatter20120210 receives capacity in EDX plus cdecl stack varargs.
extern "C" void __cdecl FormatGeometryMessageSourceBoundaryPilot(
    char* output, unsigned int capacity, const char* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    FormatMessageA(FORMAT_MESSAGE_FROM_STRING, format, 0, 0, output, capacity, &arguments);
    va_end(arguments);
}
extern "C" BOOL __stdcall OpenDisplaySettingsPilot(HWND, const char*, int, int, HANDLE*);
extern "C" unsigned int (__stdcall *g_initializationErrorCallbackPilot)(
    unsigned int, unsigned int, const char*, unsigned int, unsigned int, unsigned int);
extern "C" char g_initializationErrorTextPilot[];
extern "C" unsigned int g_initializationErrorContextPilot;
extern "C" unsigned int __stdcall HandleGeometryErrorPilot(unsigned int status)
{
    char message[512], format[512];
    unsigned int result = 1;
    switch (status) {
    case 0xb0001: LoadSplashStringPilot(0xf457, message, 512); break;
    case 0xb0002:
        LoadSplashStringPilot(0xf455, format, 512);
        FormatGeometryMessageSourceBoundaryPilot(message, 512, format, 1024, 720);
        break;
    default: return 0x80004005u;
    }
    if (g_initializationErrorCallbackPilot != NULL &&
        g_initializationErrorCallbackPilot(0, reinterpret_cast<unsigned int>(message),
            g_initializationErrorTextPilot, 4, 0x65, g_initializationErrorContextPilot) == 6) {
        if (!OpenDisplaySettingsPilot(NULL, "desk.cpl desk,@Settings /Action", 1, 1, NULL))
            result = 0x80004005u;
    }
    return result;
}
