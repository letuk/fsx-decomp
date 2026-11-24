#include "window_controller.h"
#include <stddef.h>
#include <string.h>

// Observed fields starting at controller +0x38.
struct ControllerModeStatePilot {
    BYTE modeCallbacksEnabled;
    BYTE clipRefreshEnabled;
    BYTE clipRefreshOverride;
    BYTE hasVisibleWindow;
    BYTE unknown3c[8];
    int mode;
    LONG dialogModeCount;
};
C_ASSERT(offsetof(ControllerModeStatePilot, mode) == 0xc);
C_ASSERT(offsetof(ControllerModeStatePilot, dialogModeCount) == 0x10);

static ControllerModeStatePilot& ReadControllerModeStatePilot(IdleControllerBoundaryPilot* controller)
{
    return *reinterpret_cast<ControllerModeStatePilot*>(reinterpret_cast<BYTE*>(controller) + 0x38);
}

// API IAT 200e1338, g2d.dll ordinal 4. This is an external source boundary.
extern "C" void* __cdecl GetGraphicsControllerSourcePilot();
extern "C" void __cdecl RefreshControllerClipRegionsSourcePilot(); // 2011d836

// 20125b54, slot +0x14. Import lookup is short-circuited on byte +0x39.
bool IdleControllerBoundaryPilot::QueryClipRegionRefresh()
{
    ControllerModeStatePilot& state = ReadControllerModeStatePilot(this);
    return state.clipRefreshEnabled != 0 && GetGraphicsControllerSourcePilot() != NULL &&
        (state.clipRefreshOverride != 0 || state.modeCallbacksEnabled == 0);
}

// 201251ba. The original checks the output from GetClassNameA without
// separately checking that call's status.
extern "C" BOOL CALLBACK FindVisibleFloatingWindowPilot(HWND window, LPARAM context)
{
    char name[260];
    GetClassNameA(window, name, 256);
    if (_stricmp(name, "FS98FLOAT") != 0) return TRUE;
    HDC dc = GetDCEx(window, NULL, DCX_CACHE);
    RECT bounds;
    int clip = GetClipBox(dc, &bounds);
    ReleaseDC(window, dc);
    if (clip == NULLREGION) return TRUE;
    *reinterpret_cast<int*>(context) = 1;
    return FALSE;
}

// 20126096, slot +0x18. A clip result other than NULLREGION is treated
// as visible, including the zero error result.
bool IdleControllerBoundaryPilot::RefreshVisibleWindows()
{
    HWND window = *reinterpret_cast<HWND*>(reinterpret_cast<BYTE*>(this) + 4);
    HDC dc = GetDCEx(window, NULL, DCX_CACHE);
    RECT bounds;
    bool visible = GetClipBox(dc, &bounds) != NULLREGION;
    ReleaseDC(window, dc);
    if (QueryClipRegionRefresh()) RefreshControllerClipRegionsSourcePilot();
    if (!visible) {
        int found = 0;
        EnumThreadWindows(GetCurrentThreadId(), FindVisibleFloatingWindowPilot,
            reinterpret_cast<LPARAM>(&found));
        visible = found != 0;
    }
    ReadControllerModeStatePilot(this).hasVisibleWindow = visible;
    return visible;
}

// 20125b7f, slot +0x2c. Mode one counts enables. Mode two overrides the
// current mode and falls back to the mode-one count on disable.
void IdleControllerBoundaryPilot::SetDialogMode(unsigned int mode, bool enabled)
{
    if (mode == 0 || mode > 2) return;
    ControllerModeStatePilot& state = ReadControllerModeStatePilot(this);
    int nextMode = state.mode;
    if (enabled) {
        if (mode == 1) {
            state.dialogModeCount = static_cast<LONG>(static_cast<DWORD>(state.dialogModeCount) + 1);
            if (nextMode == 0) nextMode = 1;
        } else if (nextMode < 2) {
            nextMode = 2;
        }
    } else if (mode == 1) {
        if (state.dialogModeCount > 0 && --state.dialogModeCount == 0 && nextMode == 1)
            nextMode = 0;
    } else {
        nextMode = state.dialogModeCount != 0;
    }
    if (nextMode == state.mode) return;
    state.mode = nextMode;
    if (state.modeCallbacksEnabled != 0) {
        RefreshVisibleWindows();
        void* graphics = GetGraphicsControllerSourcePilot();
        typedef void (__thiscall *NotifyGraphicsModePilot)(void*);
        void** table = *static_cast<void***>(graphics);
        reinterpret_cast<NotifyGraphicsModePilot>(table[0x38 / sizeof(void*)])(graphics);
    }
}
