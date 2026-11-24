#include <windows.h>
// Reconstructed export264. Word meanings beyond the observed ABI provisional.
typedef unsigned int (__stdcall *InitializationErrorCallbackPilot)(
    unsigned int, unsigned int, const char*, unsigned int, unsigned int, unsigned int);
extern "C" InitializationErrorCallbackPilot g_initializationErrorCallbackPilot;
extern "C" char g_initializationErrorTextPilot[];
extern "C" unsigned int g_initializationErrorContextPilot;
extern "C" unsigned int __stdcall ReportInitializationErrorPilot(
    unsigned int errorCode, unsigned int detail, unsigned int argument)
{
    unsigned int result = 0;
    if (g_initializationErrorCallbackPilot != NULL)
        result = g_initializationErrorCallbackPilot(errorCode, detail,
            g_initializationErrorTextPilot, argument, 0x65,
            g_initializationErrorContextPilot);
    return result;
}
