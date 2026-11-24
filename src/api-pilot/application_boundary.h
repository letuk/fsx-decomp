#pragma once
#include <windows.h>
// Comparison-only class fragment; no complete original class/layout claim.
extern HWND g_mainWindowHandlePilot;
typedef MSG* (WINAPI *CurrentMessageCallbackBoundaryPilot)();
class ApplicationBoundaryPilot {
public:
    __declspec(dllexport) virtual int __thiscall Initialize(HINSTANCE instance, UINT startupArgument,
        int startupMode, void* errorCallback, HANDLE singleInstanceHandle);
    __declspec(dllexport) virtual int __thiscall Shutdown();
    __declspec(dllexport) virtual int __thiscall RunMessageLoop(CurrentMessageCallbackBoundaryPilot callback);
    __declspec(dllexport) virtual HWND __thiscall GetWindowHandle(int windowId);
};
