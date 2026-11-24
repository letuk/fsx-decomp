#include <windows.h>
extern "C" HINSTANCE g_mainWindowInstancePilot;
extern "C" LRESULT CALLBACK MainChildWindowProcedurePilot(HWND, UINT, WPARAM, LPARAM);
extern "C" LRESULT CALLBACK MainWindowProcedurePilot(HWND, UINT, WPARAM, LPARAM);
extern "C" LRESULT CALLBACK DialogPanelWindowProcedurePilot(HWND, UINT, WPARAM, LPARAM);
ATOM g_dialogPanelWindowClassAtomPilot; // 2020de84
extern "C" LRESULT CALLBACK PanelShadowWindowProcedurePilot(HWND, UINT, WPARAM, LPARAM);
extern "C" LRESULT CALLBACK FloatingPanelWindowProcedurePilot(HWND, UINT, WPARAM, LPARAM);

// FS98FLOAT registration block in 2012205d.
extern "C" bool __cdecl RegisterFloatingPanelWindowClassPilot()
{
    WNDCLASSA windowClass = {0x4003, FloatingPanelWindowProcedurePilot, 0, 0,
        g_mainWindowInstancePilot, NULL, LoadCursorA(NULL, MAKEINTRESOURCEA(0x7f00)),
        static_cast<HBRUSH>(GetStockObject(4)), NULL, "FS98FLOAT"};
    return RegisterClassA(&windowClass) != 0;
}

extern "C" bool __cdecl RegisterPanelShadowWindowClassPilot()
{
    WNDCLASSA windowClass = { 0x4003, PanelShadowWindowProcedurePilot, 0, 0,
        g_mainWindowInstancePilot, NULL, LoadCursorA(NULL, MAKEINTRESOURCEA(0x7f00)),
        static_cast<HBRUSH>(GetStockObject(4)), NULL, "FS98SHADOW" };
    return RegisterClassA(&windowClass) != 0;
}

// FS98DLG3D block in 2012205d.
extern "C" bool __cdecl RegisterDialogPanelWindowClassPilot()
{
    WNDCLASSA windowClass = { 0x4003, DialogPanelWindowProcedurePilot, 0, 0,
        g_mainWindowInstancePilot, NULL, LoadCursorA(NULL, MAKEINTRESOURCEA(0x7f00)),
        static_cast<HBRUSH>(GetStockObject(4)), NULL, "FS98DLG3D" };
    g_dialogPanelWindowClassAtomPilot = RegisterClassA(&windowClass);
    return g_dialogPanelWindowClassAtomPilot != 0;
}

// FS98MAIN registration block of 2012205d. Caller owns error reporting.
extern "C" bool __cdecl RegisterMainWindowClassPilot()
{
    WNDCLASSA windowClass;
    windowClass.style = 3;
    windowClass.lpfnWndProc = MainWindowProcedurePilot;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = g_mainWindowInstancePilot;
    windowClass.hIcon = LoadIconA(windowClass.hInstance, MAKEINTRESOURCEA(0x65));
    windowClass.hCursor = LoadCursorA(NULL, MAKEINTRESOURCEA(0x7f00));
    windowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(4));
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = "FS98MAIN";
    return RegisterClassA(&windowClass) != 0;
}
extern "C" bool __cdecl RegisterMainChildClassPilot()
{
    WNDCLASSA windowClass;
    windowClass.style = 0x4003;
    windowClass.lpfnWndProc = MainChildWindowProcedurePilot;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = g_mainWindowInstancePilot;
    windowClass.hIcon = NULL;
    windowClass.hCursor = LoadCursorA(NULL, MAKEINTRESOURCEA(0x7f00));
    windowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(4));
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = "FS98CHILD";
    return RegisterClassA(&windowClass) != 0;
}

extern "C" DWORD __stdcall ReportPanelWindowErrorPilot(DWORD code, DWORD argument, DWORD flags);

// Class registration phase of 2012205d. The enclosing startup owns its other phases.
extern "C" bool __cdecl RegisterStartupWindowClassesPilot()
{
    if (!RegisterMainWindowClassPilot()) {
        ReportPanelWindowErrorPilot(0xf002, 0, 0);
        return false;
    }
    if (!RegisterMainChildClassPilot() || !RegisterFloatingPanelWindowClassPilot() ||
        !RegisterPanelShadowWindowClassPilot() || !RegisterDialogPanelWindowClassPilot()) {
        ReportPanelWindowErrorPilot(0xf005, 0, 0);
        return false;
    }
    return true;
}
