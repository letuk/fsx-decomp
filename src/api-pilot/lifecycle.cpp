#include "application_boundary.h"
// Object-only reconstruction: dependencies deliberately remain undefined.
// They identify original functions; no substitute implementations are linked.
typedef int (__cdecl *ApplicationCallbackBoundaryPilot)();
extern int __cdecl ShutdownApplicationBodyPilot();
extern int __cdecl RunApplicationMessageLoopBodyPilot();
extern int __stdcall FilterApplicationExceptionPilot(unsigned long code, EXCEPTION_POINTERS* pointers);
extern CurrentMessageCallbackBoundaryPilot g_currentMessageCallbackPilot;
__declspec(dllexport) int __stdcall InvokeApplicationCallbackBoundaryPilot(ApplicationCallbackBoundaryPilot callback)
{
    __try {
        return callback();
    }
    __except (FilterApplicationExceptionPilot(GetExceptionCode(), GetExceptionInformation())) {
        return 0;
    }
}
int ApplicationBoundaryPilot::Shutdown()
{
    return InvokeApplicationCallbackBoundaryPilot(&ShutdownApplicationBodyPilot);
}
int ApplicationBoundaryPilot::RunMessageLoop(CurrentMessageCallbackBoundaryPilot callback)
{
    g_currentMessageCallbackPilot = callback;
    InvokeApplicationCallbackBoundaryPilot(&RunApplicationMessageLoopBodyPilot);
    return 0;
}

// Retain the prior outer-body COMDAT checks while adding the FDI dependency slice.
#include "fdi_stream_callbacks.cpp"
#include "initialize_boundary.cpp"
