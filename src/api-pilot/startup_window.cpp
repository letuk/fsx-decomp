#include <windows.h>
extern "C" HGDIOBJ __cdecl CreateStartupImagePilot();
extern "C" DWORD WINAPI StartupWindowThreadPilot(void*);
extern "C" HGDIOBJ g_startupImagePilot;
extern "C" HANDLE g_startupThreadPilot;
extern "C" HWND g_startupWindowPilot;
extern "C" HWND __cdecl StartStartupWindowPilot()
{
    HANDLE readyEvent = NULL;
    readyEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (readyEvent == NULL) return NULL;
    g_startupImagePilot = CreateStartupImagePilot();
    if (g_startupImagePilot != NULL) {
        DWORD threadId;
        g_startupThreadPilot = CreateThread(NULL, 0, StartupWindowThreadPilot,
                                           &readyEvent, 0, &threadId);
        if (g_startupThreadPilot != NULL) {
            HANDLE handles[2] = { g_startupThreadPilot, readyEvent };
            DWORD waitResult = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
            if (waitResult != WAIT_OBJECT_0 + 1) {
                if (waitResult != WAIT_OBJECT_0)
                    PostThreadMessageA(threadId, WM_QUIT, 0, 0);
                CloseHandle(g_startupThreadPilot);
                g_startupThreadPilot = NULL;
            }
        }
    }
    if (readyEvent != NULL) CloseHandle(readyEvent);
    if (g_startupThreadPilot == NULL) {
        if (g_startupImagePilot != NULL) {
            DeleteObject(g_startupImagePilot);
            g_startupImagePilot = NULL;
        }
        return NULL;
    }
    return g_startupWindowPilot;
}
