#ifndef FSX_APPLICATION_INTERFACE_PILOT_H
#define FSX_APPLICATION_INTERFACE_PILOT_H
#include "error_callback_pilot.h"
// Reconstructed minimum vtable shape; not an original/complete class declaration.
typedef MSG* (WINAPI *CurrentMessageCallbackPilot)();
class ApplicationPilot {
public:
    virtual int Initialize(HINSTANCE instance, ULONG_PTR startupArgument, int startupMode,
                           ErrorCallbackPilot callback, HANDLE eventHandle) = 0;
    virtual void UnknownSlot04() = 0;
    virtual int RunMessageLoop(CurrentMessageCallbackPilot getCurrentMessage) = 0;
    virtual HWND GetWindowHandle(int windowId) = 0;
};
typedef ApplicationPilot* (__cdecl *GetApplicationPilotFunc)();
extern GetApplicationPilotFunc pfnGetApplication;
#endif


