#include <fdi.h>
// fdi.h is the installed VC8 SDK contract. The implementation callbacks remain
// unresolved original dependencies, not synthetic implementations.
extern FNALLOC(FdiAllocateBoundaryPilot);
extern FNFREE(FdiFreeBoundaryPilot);
extern FNOPEN(FdiOpenBoundaryPilot);
extern UINT __cdecl FdiReadStreamBoundaryPilot(INT_PTR, void*, UINT);
extern UINT __cdecl FdiWriteStreamBoundaryPilot(INT_PTR, void*, UINT);
extern int __cdecl FdiCloseStreamBoundaryPilot(INT_PTR);
extern long __cdecl FdiSeekStreamBoundaryPilot(INT_PTR, long, int);
extern int __cdecl InitializeApplicationBodyBoundaryPilot();
extern HINSTANCE g_applicationInstanceBoundaryPilot;
extern UINT g_startupArgumentBoundaryPilot;
extern int g_startupModeBoundaryPilot;
extern void* g_errorCallbackBoundaryPilot;
extern int g_exceptionReportingEnabledBoundaryPilot;
extern HANDLE g_singleInstanceEventBoundaryPilot;
extern ERF g_fdiErrorBoundaryPilot;
extern HFDI g_fdiContextBoundaryPilot;
#include "callback_registry_boundary.h"
extern void __cdecl OnApplicationEventBoundaryPilot(ULONG_PTR, ULONG_PTR);
// Minimum observed owner: first DWORD points to a dynamic callback list.
// Native thiscall uses ECX; original optimized helper uses ESI. Do not hide that
// encoding/ABI residual behind relocation normalization.
extern CallbackRegistryOwnerBoundaryPilot g_initializeCallbackRegistryBoundaryPilot;
int ApplicationBoundaryPilot::Initialize(HINSTANCE instance, UINT startupArgument,
    int startupMode, void* errorCallback, HANDLE singleInstanceHandle)
{
    g_applicationInstanceBoundaryPilot = instance;
    g_startupArgumentBoundaryPilot = startupArgument;
    g_startupModeBoundaryPilot = startupMode;
    g_errorCallbackBoundaryPilot = errorCallback;
    g_exceptionReportingEnabledBoundaryPilot = 1;
    g_singleInstanceEventBoundaryPilot = singleInstanceHandle;
    g_fdiContextBoundaryPilot = FDICreate(&FdiAllocateBoundaryPilot, &FdiFreeBoundaryPilot,
        &FdiOpenBoundaryPilot, &FdiReadStreamBoundaryPilot, &FdiWriteStreamBoundaryPilot,
        &FdiCloseStreamBoundaryPilot, &FdiSeekStreamBoundaryPilot,
        1, &g_fdiErrorBoundaryPilot);
    int result = InvokeApplicationCallbackBoundaryPilot(&InitializeApplicationBodyBoundaryPilot);
    if (result != 0)
        g_initializeCallbackRegistryBoundaryPilot.Register(&OnApplicationEventBoundaryPilot, 0, 0, 0);
    return result;
}
