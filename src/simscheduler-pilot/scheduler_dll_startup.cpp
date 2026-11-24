#include <windows.h>

extern "C" __declspec(dllimport) void __stdcall NotifySchedulerApiAttach();
extern "C" __declspec(dllimport) void __stdcall NotifySchedulerApiDetach();

extern "C" {
HMODULE schedulerModule;
}

extern "C" BOOL WINAPI SchedulerRawDllMain(HMODULE, DWORD reason, void*)
{
    switch (reason) {
    case DLL_PROCESS_DETACH:
        NotifySchedulerApiDetach();
        break;
    case DLL_PROCESS_ATTACH:
        NotifySchedulerApiAttach();
        return TRUE;
    }
    return TRUE;
}

extern "C" {
BOOL (WINAPI *_pRawDllMain)(HMODULE, DWORD, void*) = SchedulerRawDllMain;
}

extern "C" BOOL WINAPI DllMain(HMODULE module, DWORD reason, void*)
{
    if (reason == DLL_PROCESS_ATTACH) {
        schedulerModule = module;
        DisableThreadLibraryCalls(module);
    }
    return TRUE;
}
