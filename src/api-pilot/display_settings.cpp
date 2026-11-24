#include <shellapi.h>
#include <stdio.h>

// 20123315. Wait mode dispatches paint messages while the applet is open.
extern "C" BOOL __stdcall OpenDisplaySettingsPilot(
    HWND owner, const char* applet, int show, int wait, HANDLE* process)
{
    char systemDirectory[MAX_PATH];
    UINT length = GetSystemDirectoryA(systemDirectory, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) return FALSE;

    char executable[MAX_PATH], parameters[3 * MAX_PATH];
    if (_snprintf(executable, sizeof(executable), "%s\\rundll32.exe", systemDirectory) < 0 ||
        _snprintf(parameters, sizeof(parameters), "%s\\shell32.dll,Control_RunDLL %s\\%s",
            systemDirectory, systemDirectory, applet) < 0)
        return FALSE;
    executable[sizeof(executable) - 1] = '\0';
    parameters[sizeof(parameters) - 1] = '\0';

    SHELLEXECUTEINFOA launch = {0};
    launch.cbSize = sizeof(launch);
    launch.fMask = 0x800000;
    if (wait != 0 || process != NULL) launch.fMask |= SEE_MASK_NOCLOSEPROCESS;
    launch.hwnd = owner;
    launch.lpFile = executable;
    launch.lpParameters = parameters;
    launch.lpDirectory = systemDirectory;
    launch.nShow = show;
    BOOL started;
    if (g_idleControllerPilot == NULL) {
        started = ShellExecuteExA(&launch);
    } else {
        typedef BOOL (__thiscall *LaunchDisplayAppletPilot)(void*, SHELLEXECUTEINFOA*);
        started = GraphicsClipSlotPilot<LaunchDisplayAppletPilot>(g_idleControllerPilot, 0x68)(
            g_idleControllerPilot, &launch);
    }
    if (!started) return started;
    if (wait != 0) {
        if (launch.hProcess == NULL) return FALSE;
        for (;;) {
            DWORD result = MsgWaitForMultipleObjects(1, &launch.hProcess, FALSE, INFINITE, QS_PAINT);
            if (result != WAIT_OBJECT_0 + 1) {
                CloseHandle(launch.hProcess);
                return result == WAIT_FAILED ? FALSE : started;
            }
            MSG message;
            while (PeekMessageA(&message, NULL, WM_PAINT, WM_PAINT, PM_REMOVE)) {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
        }
    }
    if (process != NULL) {
        *process = launch.hProcess;
        CloseHandle(launch.hProcess);
    }
    return started;
}
