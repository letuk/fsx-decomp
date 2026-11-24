#pragma once
#include <windows.h>

// Observed virtual slots through +0x30. The controller remains external.
class IdleControllerBoundaryPilot {
public:
    virtual bool QueryField34() = 0;
    virtual bool QueryField38() = 0;
    virtual void Unknown08() = 0;
    virtual void Unknown0c() = 0;
    virtual bool QueryHiddenPosition() = 0;
    virtual bool QueryClipRegionRefresh();
    virtual bool RefreshVisibleWindows();
    virtual bool QueryActive() = 0;
    virtual void Unknown20() = 0;
    virtual void Unknown24() = 0;
    virtual void Unknown28() = 0;
    virtual void SetDialogMode(unsigned int mode, bool enabled);
    virtual int QueryMode() = 0;

    // Source targets of original slots +0x48 through +0x58.
    void SetCloseOverridePilot(DWORD enabled);
    void EnterModalWindowPilot();
    void LeaveModalWindowPilot();
    BYTE ReadModalWindowFlagPilot();
    void SetModalWindowFlagPilot(BYTE enabled);

    // Source target of original slot +0x64. Table placement remains external.
    HINSTANCE LaunchShellTargetPilot(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, int);
};
extern IdleControllerBoundaryPilot* g_idleControllerPilot; // 2020de98
