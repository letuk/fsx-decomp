#include <windows.h>
#include <string.h>
extern "C" HMODULE g_languageModulePilot;
extern "C" char g_configurationFilePilot[];
extern "C" char g_defaultLanguageDllPilot[];
extern "C" char g_resolverBasePathPilot[];
extern "C" char g_sharedLanguagePathPilot[];
extern "C" char g_initializationErrorTextPilot[];
extern "C" char* __stdcall CopyResolvedPathPilot(char*, const char*, unsigned int);
extern "C" int __stdcall LoadSplashStringPilot(unsigned int, char*, int);
// Original200fb1ab uses EAX capacity/EDI output plus stack varargs.
extern "C" void __cdecl FormatLanguageErrorSourceBoundaryPilot(char*, unsigned int, const char*, ...);
// Normal business-flow model of2013e64f. Original integrity gateways and their
// failure paths remain outside this source-only boundary; no full-body claim.
extern "C" BOOL __cdecl LoadLanguageModuleSourceBoundaryPilot()
{
    char configuredPath[260];
    char errorText[256];
    const char* loadPath = g_sharedLanguagePathPilot;
    if (g_languageModulePilot != NULL)
        FreeLibrary(g_languageModulePilot);
    if (strlen(g_configurationFilePilot) != 0) {
        GetPrivateProfileStringA("LANGUAGE", "DLL", g_defaultLanguageDllPilot,
                                 configuredPath, 260, g_configurationFilePilot);
        if (configuredPath[0] != '\0' &&
            (configuredPath[1] == ':' || (configuredPath[0] == '\\' && configuredPath[1] == '\\'))) {
            loadPath = configuredPath;
        } else {
            CopyResolvedPathPilot(g_sharedLanguagePathPilot, g_resolverBasePathPilot, 260);
            unsigned int length = static_cast<unsigned int>(strlen(g_sharedLanguagePathPilot));
            if (length < 260)
                CopyResolvedPathPilot(g_sharedLanguagePathPilot + length, configuredPath, 260 - length);
        }
    } else {
        loadPath = g_defaultLanguageDllPilot;
    }
    g_languageModulePilot = LoadLibraryA(loadPath);
    if (g_languageModulePilot == NULL) {
        DWORD error = GetLastError();
        FormatLanguageErrorSourceBoundaryPilot(errorText, 256,
            "Couldn't Load Language Resource DLL\nError File: %s\nError Code: 0x%X (%d)\nProgram is aborting!",
            g_sharedLanguagePathPilot, error, error);
        MessageBoxA(NULL, errorText, "ERROR", 0x10);
        return FALSE;
    }
    LoadSplashStringPilot(3, g_initializationErrorTextPilot, 256);
    return TRUE;
}
