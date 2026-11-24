#include <windows.h>
#include <stddef.h>
#include <vector>

struct G2dControllerObservedPilot {
    void** table;
    DWORD arrayAllocator;
    void** windowsBegin;
    void** windowsEnd;
    void** windowsCapacityEnd;
    DWORD windowTransitionState;
    BYTE apiWindowState;
    BYTE deferredSuspend;
    BYTE suspendGate;
    BYTE deferredRefresh;
    BYTE reportRefresh;
    BYTE initialized;
};
C_ASSERT(offsetof(G2dControllerObservedPilot, windowsBegin) == 8);
C_ASSERT(offsetof(G2dControllerObservedPilot, initialized) == 0x1d);

// Source boundaries owned by api.dll. Final import binding remains external.
extern "C" int __cdecl InitializeApiGraphicsServicesBoundaryPilot(); // ordinal 276
extern "C" int __cdecl QueryApiQuitBoundaryPilot(); // ordinal 6
extern "C" void* __cdecl ReadApiWindowControllerBoundaryPilot(); // ordinal 318
extern "C" void __cdecl NotifyApiGraphicsRefreshBoundaryPilot(); // ordinal 263
extern "C" void __stdcall ReportApiGraphicsRefreshBoundaryPilot(DWORD, DWORD, DWORD); // ordinal 233
extern "C" int __stdcall LoadApiGraphicsStringBoundaryPilot(UINT, char*, int); // ordinal 154
extern "C" char* __stdcall CopyApiGraphicsStringBoundaryPilot(char*, const char*, unsigned int); // ordinal 157
extern "C" int __stdcall PostApiGraphicsCloseBoundaryPilot(LPARAM); // ordinal 5

extern G2dControllerObservedPilot* g_g2dControllerPilot; // 20709bd8, creation and publication boundary
extern "C" void __stdcall UpdateG2dWindowPlacementBoundaryPilot(void*); // 206976d0
extern "C" void __stdcall CollectG2dReplacementWindowsBoundaryPilot(
    G2dControllerObservedPilot*, std::vector<HWND>*); // 20698a70, source-level interface

template<class Method>
static Method G2dSlotPilot(void* receiver, unsigned int offset)
{
    return reinterpret_cast<Method>((*static_cast<void***>(receiver))[offset / sizeof(void*)]);
}

typedef bool (__thiscall *G2dFlagMethodPilot)(void*);
typedef int (__thiscall *G2dStatusMethodPilot)(void*);
typedef void (__thiscall *G2dNotifyMethodPilot)(void*);
typedef void (__thiscall *G2dWordMethodPilot)(void*, DWORD);

extern "C" G2dControllerObservedPilot* __cdecl GetG2dControllerSourcePilot() // 20698050 / ordinal 4
{
    return g_g2dControllerPilot;
}

// 206987b0. The copied range keeps the iteration independent of slot callbacks.
static void SuspendG2dWindowsPilot(G2dControllerObservedPilot* controller)
{
    if (controller->initialized && !G2dSlotPilot<G2dFlagMethodPilot>(controller, 0x44)(controller)) return;
    if (!controller->windowsBegin || controller->windowsBegin == controller->windowsEnd) return;
    controller->windowTransitionState = 0;
    std::vector<void*> windows(controller->windowsBegin, controller->windowsEnd);
    for (size_t index = 0; index < windows.size(); ++index) {
        void* window = windows[index];
        if (window) G2dSlotPilot<G2dWordMethodPilot>(window, 0x9c)(window, 1);
    }
}

// 20698780 / slot +0x14.
extern "C" void __fastcall SuspendG2dControllerSourcePilot(G2dControllerObservedPilot* controller)
{
    if (controller->suspendGate) {
        controller->deferredSuspend = 1;
        return;
    }
    SuspendG2dWindowsPilot(controller);
    controller->apiWindowState = 0;
}

// 20698730 / slot +0x10.
extern "C" int __fastcall BeginG2dControllerTransitionSourcePilot(G2dControllerObservedPilot* controller)
{
    if ((controller->initialized && !G2dSlotPilot<G2dFlagMethodPilot>(controller, 0x44)(controller)) ||
        controller->suspendGate) return 0;
    SuspendG2dControllerSourcePilot(controller);
    if (InitializeApiGraphicsServicesBoundaryPilot() != 0) return -1;
    int status = G2dSlotPilot<G2dStatusMethodPilot>(controller, 0x4c)(controller);
    if (!status) controller->initialized = 1;
    return status;
}

static bool g_g2dRefreshingControllerPilot; // 20709bf0

class G2dRefreshScopePilot {
public:
    G2dRefreshScopePilot() { g_g2dRefreshingControllerPilot = true; }
    ~G2dRefreshScopePilot() { g_g2dRefreshingControllerPilot = false; }
private:
    G2dRefreshScopePilot(const G2dRefreshScopePilot&);
    G2dRefreshScopePilot& operator=(const G2dRefreshScopePilot&);
};

// 20698ca0. Retain the order of the three original per-window passes.
static int RefreshG2dControllerWindowsPilot(G2dControllerObservedPilot* controller)
{
    void* apiController = ReadApiWindowControllerBoundaryPilot();
    if (G2dSlotPilot<G2dFlagMethodPilot>(apiController, 0)(apiController)) return 0;
    apiController = ReadApiWindowControllerBoundaryPilot();
    if (!G2dSlotPilot<G2dFlagMethodPilot>(apiController, 8)(apiController) || g_g2dRefreshingControllerPilot)
        return 0;

    G2dRefreshScopePilot refresh;
    apiController = ReadApiWindowControllerBoundaryPilot();
    controller->apiWindowState = apiController && G2dSlotPilot<G2dFlagMethodPilot>(apiController, 4)(apiController);
    controller->reportRefresh = 0;
    const size_t count = controller->windowsBegin ? controller->windowsEnd - controller->windowsBegin : 0;
    for (size_t index = 0; index < count; ++index) {
        void* window = controller->windowsBegin[index];
        if (!G2dSlotPilot<G2dFlagMethodPilot>(window, 0x48)(window))
            G2dSlotPilot<G2dNotifyMethodPilot>(window, 0xc4)(window);
        UpdateG2dWindowPlacementBoundaryPilot(window);
        G2dSlotPilot<G2dNotifyMethodPilot>(window, 0xac)(window);
    }

    std::vector<HWND> oldWindows;
    CollectG2dReplacementWindowsBoundaryPilot(controller, &oldWindows);
    NotifyApiGraphicsRefreshBoundaryPilot();
    for (size_t index = 0; index < count; ++index) {
        void* window = controller->windowsBegin[index];
        if (!G2dSlotPilot<G2dFlagMethodPilot>(window, 0x48)(window))
            G2dSlotPilot<G2dNotifyMethodPilot>(window, 0xc0)(window);
    }
    for (size_t index = 0; index < count; ++index) {
        void* window = controller->windowsBegin[index];
        if (!G2dSlotPilot<G2dFlagMethodPilot>(window, 0x48)(window))
            G2dSlotPilot<G2dNotifyMethodPilot>(window, 0xc8)(window);
    }
    for (size_t index = 0; index < oldWindows.size(); ++index)
        DestroyWindow(oldWindows[index]);
    if (controller->reportRefresh) ReportApiGraphicsRefreshBoundaryPilot(0x10, 0x181, 0);
    return 0;
}

// 20698c70 / slot +0x38.
extern "C" int __fastcall RefreshG2dControllerSourcePilot(G2dControllerObservedPilot* controller)
{
    if (QueryApiQuitBoundaryPilot()) return 0;
    if (controller->suspendGate) {
        controller->deferredRefresh = 1;
        return 0;
    }
    return RefreshG2dControllerWindowsPilot(controller);
}

static DWORD WINAPI ShowG2dFailureDialogPilot(void* parameters)
{
    return static_cast<DWORD>(MessageBoxIndirectA(static_cast<MSGBOXPARAMSA*>(parameters)));
}

// 20699720 / ordinal 65.
extern "C" void __cdecl ReportG2dStartupFailureSourcePilot()
{
    char title[256] = "Error starting graphics system";
    const char fallback[] =
        "Flight Simulator cannot communicate with your video card.\n"
        "This may be due to the drivers currently installed for your video card.\n\n"
        "Flight Simulator will now exit.";
    char message[2048] = {0};
    LoadApiGraphicsStringBoundaryPilot(0x9108, title, sizeof(title));
    if (!LoadApiGraphicsStringBoundaryPilot(0x9106, message, sizeof(message)))
        CopyApiGraphicsStringBoundaryPilot(message, fallback, sizeof(message));

    MSGBOXPARAMSA dialog = {0};
    dialog.cbSize = sizeof(dialog);
    dialog.lpszText = message;
    dialog.lpszCaption = title;
    dialog.dwStyle = 0x50030;
    DWORD threadId;
    HANDLE thread = CreateThread(NULL, 0, ShowG2dFailureDialogPilot, &dialog, 0, &threadId);
    if (thread) {
        WaitForSingleObject(thread, INFINITE);
        DWORD exitCode;
        GetExitCodeThread(thread, &exitCode);
        CloseHandle(thread);
    }
    PostApiGraphicsCloseBoundaryPilot(1);
}

// Backend objects publish different interfaces at different offsets.
struct G2dBackendControllerObservedPilot {
    G2dControllerObservedPilot common;
    DWORD backendField20;
    void* backendField24;
    DWORD backendField28;
};
C_ASSERT(offsetof(G2dBackendControllerObservedPilot, backendField24) == 0x24);

struct G2dDxgiControllerObservedPilot {
    G2dBackendControllerObservedPilot common;
    void* factory;
};
C_ASSERT(offsetof(G2dDxgiControllerObservedPilot, factory) == 0x2c);

extern "C" int __stdcall SetApiGraphicsPreparationBoundaryPilot(int enabled); // api.dll ordinal 216
extern "C" void __cdecl PrepareG2dDesktopBoundaryPilot(); // 206bea50
extern "C" void __cdecl BeginG2dLegacyPreparationBoundaryPilot(); // 206edb60
extern "C" void __cdecl EndG2dLegacyPreparationBoundaryPilot(); // 206edc80

#include "window_construction.h"
#include "window_initialization.h"

// 20696b90 and 20683270 / slot +0x44. Only AL is defined by the originals.
extern "C" bool __fastcall QueryG2dDxgiRestartSourcePilot(G2dControllerObservedPilot*)
{
    return false;
}

extern "C" bool __fastcall QueryG2dLegacyRestartSourcePilot(G2dControllerObservedPilot*)
{
    return true;
}

// 206988d0. A zero result acquires the shared backend initialization gate.
static bool BlockG2dBackendInitializationPilot(G2dControllerObservedPilot* controller)
{
    if (controller->suspendGate || QueryApiQuitBoundaryPilot() ||
        (controller->windowsBegin && controller->windowsBegin != controller->windowsEnd) ||
        !ReadApiWindowControllerBoundaryPilot()) return true;
    controller->suspendGate = 1;
    void* apiController = ReadApiWindowControllerBoundaryPilot();
    if (apiController) apiController = ReadApiWindowControllerBoundaryPilot();
    controller->apiWindowState = apiController && G2dSlotPilot<G2dFlagMethodPilot>(apiController, 4)(apiController);
    return false;
}

// 20698950. Preserve the error even when no secondary windows need attention.
static int CompleteG2dBackendWindowsPilot(G2dControllerObservedPilot* controller, int status, bool partial)
{
    if (controller->windowsBegin && controller->windowsBegin != controller->windowsEnd) {
        void* first = controller->windowsBegin[0];
        if (G2dSlotPilot<G2dFlagMethodPilot>(first, 0x48)(first)) {
            SuspendG2dWindowsPilot(controller);
            status = -1;
        }
    }
    if (partial) {
        for (size_t i = 1; controller->windowsBegin &&
            i < static_cast<size_t>(controller->windowsEnd - controller->windowsBegin); ++i) {
            void* window = controller->windowsBegin[i];
            G2dSlotPilot<G2dWordMethodPilot>(window, 0x8c)(window, 1);
        }
    }
    if (status == 0 && !controller->deferredSuspend) {
        if (controller->deferredRefresh) {
            RefreshG2dControllerWindowsPilot(controller);
            controller->deferredRefresh = 0;
        } else {
            CollectG2dReplacementWindowsBoundaryPilot(controller, NULL);
        }
    }
    return status;
}

// 20698a20 and the inline equivalent in 20689b80.
static int FinishG2dBackendInitializationPilot(G2dControllerObservedPilot* controller, int status)
{
    if (status && !QueryApiQuitBoundaryPilot()) ReportG2dStartupFailureSourcePilot();
    controller->suspendGate = 0;
    if (controller->deferredSuspend) {
        SuspendG2dControllerSourcePilot(controller);
        if (!status) status = 0x40002;
    }
    controller->deferredSuspend = 0;
    controller->deferredRefresh = 0;
    return status;
}

// 20689b80 / D3D9 slot +0x4c.
extern "C" int __fastcall InitializeG2dLegacyBackendSourcePilot(G2dBackendControllerObservedPilot* backend)
{
    G2dControllerObservedPilot* controller = &backend->common;
    if (BlockG2dBackendInitializationPilot(controller)) return 0x40002;
    int status = 0;
    void* d3d = backend->backendField24;
    if (d3d) {
        int previous = SetApiGraphicsPreparationBoundaryPilot(1);
        PrepareG2dDesktopBoundaryPilot();
        BeginG2dLegacyPreparationBoundaryPilot();
        SetApiGraphicsPreparationBoundaryPilot(previous);

        typedef UINT (WINAPI *ReadAdapterCountPilot)(void*);
        UINT count = G2dSlotPilot<ReadAdapterCountPilot>(d3d, 0x10)(d3d);
        if (count > 9) count = 9;
        if (count) {
            bool partial = false;
            for (UINT index = 0; index < count; ++index) {
                void* window = CreateG2dLegacyWindowSourcePilot(index);
                if (!window) {
                    status = -1;
                    break;
                }
                AppendG2dBackendWindowSourcePilot(controller, window);
                if (InitializeG2dLegacyWindowSourcePilot(window) != 0) {
                    partial = true;
                    break;
                }
            }
            status = CompleteG2dBackendWindowsPilot(controller, status, partial);
        }
    }
    EndG2dLegacyPreparationBoundaryPilot();
    return FinishG2dBackendInitializationPilot(controller, status);
}

// 20693730 / DXGI slot +0x4c. The COM slot calls use stack-based receivers.
extern "C" int __fastcall InitializeG2dDxgiBackendSourcePilot(G2dDxgiControllerObservedPilot* backend)
{
    G2dControllerObservedPilot* controller = &backend->common.common;
    if (BlockG2dBackendInitializationPilot(controller)) return 0x40002;

    typedef HRESULT (WINAPI *EnumerateGraphicsObjectPilot)(void*, UINT, void**);
    typedef HRESULT (WINAPI *ReadGraphicsDescriptorPilot)(void*, void*);
    typedef ULONG (WINAPI *ReleaseGraphicsObjectPilot)(void*);
    UINT windowIndex = 0;
    int status = 0;
    bool partial = false;
    for (UINT adapterIndex = 0; windowIndex < 9 && !status; ++adapterIndex) {
        void* adapter = NULL;
        if (FAILED(G2dSlotPilot<EnumerateGraphicsObjectPilot>(backend->factory, 0x1c)(
            backend->factory, adapterIndex, &adapter))) break;

        DWORD adapterDescriptor[0x128 / sizeof(DWORD)];
        G2dSlotPilot<ReadGraphicsDescriptorPilot>(adapter, 0x20)(adapter, adapterDescriptor);
        void* firstOutputWindow = NULL;
        for (UINT outputIndex = 0; windowIndex < 9; ++outputIndex) {
            void* output = NULL;
            if (FAILED(G2dSlotPilot<EnumerateGraphicsObjectPilot>(adapter, 0x1c)(adapter, outputIndex, &output)))
                break;
            DWORD outputDescriptor[0x60 / sizeof(DWORD)];
            G2dSlotPilot<ReadGraphicsDescriptorPilot>(output, 0x1c)(output, outputDescriptor);
            void* window = CreateG2dDxgiWindowSourcePilot(adapterIndex, windowIndex, outputIndex,
                adapter, output, firstOutputWindow);
            if (!window) {
                status = -1;
                break;
            }
            if (outputIndex == 0) firstOutputWindow = window;
            status = InitializeG2dDxgiWindowSourcePilot(window);
            if (status) {
                G2dSlotPilot<G2dWordMethodPilot>(window, 0x9c)(window, 1);
                partial = true;
                break;
            }
            AppendG2dBackendWindowSourcePilot(controller, window);
            ++windowIndex;
        }
        G2dSlotPilot<ReleaseGraphicsObjectPilot>(adapter, 8)(adapter);
    }
    if (!G2dSlotPilot<G2dStatusMethodPilot>(controller, 0x0c)(controller)) return 0x20001;
    return FinishG2dBackendInitializationPilot(controller, CompleteG2dBackendWindowsPilot(controller, status, partial));
}
