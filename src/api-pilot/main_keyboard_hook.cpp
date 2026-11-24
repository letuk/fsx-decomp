#include <windows.h>
// Observed slots through 0x30; unknown slots are placeholders, not recovered types.
class WindowModeBoundaryPilot {
public:
    virtual void UnknownSlot00() = 0;
    virtual bool QueryMode() = 0;
    virtual void UnknownSlot08() = 0;
    virtual void UnknownSlot0c() = 0;
    virtual void UnknownSlot10() = 0;
    virtual void UnknownSlot14() = 0;
    virtual void IdleMaintenance() = 0;
    virtual void UnknownSlot1c() = 0;
    virtual bool QueryBusy() = 0;
    virtual void UnknownSlot24() = 0;
    virtual void UnknownSlot28() = 0;
    virtual void UnknownSlot2c() = 0;
    virtual int QueryCursorState() = 0;
};
extern "C" WindowModeBoundaryPilot* g_windowModeBoundaryPilot;
extern "C" HWND g_hookMainWindowPilot;
extern "C" HHOOK g_keyboardHookPilot;
extern "C" LRESULT CALLBACK MainKeyboardHookPilot(int code, WPARAM key, LPARAM flags)
{
    if (code == 0) {
        unsigned int high = static_cast<unsigned int>(flags) >> 16;
        unsigned int alt = high & 0x2000;
        unsigned int released = high & 0x8000;
        if (g_windowModeBoundaryPilot->QueryMode() && GetFocus() != g_hookMainWindowPilot) {
            if (key == VK_TAB) {
                if (alt != 0 && released != 0)
                    PostMessageA(g_hookMainWindowPilot, WM_SYSKEYUP, key, flags);
            } else if (key == VK_ESCAPE) {
                if (alt != 0) {
                    if (released != 0)
                        PostMessageA(g_hookMainWindowPilot, WM_SYSKEYUP, key, flags);
                } else if (released != 0 && (GetKeyState(VK_CONTROL) & 0x8000) != 0) {
                    PostMessageA(g_hookMainWindowPilot, WM_SYSKEYUP, VK_ESCAPE, flags);
                }
            }
        }
    }
    return CallNextHookEx(g_keyboardHookPilot, code, key, flags);
}
