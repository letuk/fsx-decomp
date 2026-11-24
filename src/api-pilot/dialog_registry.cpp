#include "heap_boundary.h"
#include "window_controller.h"
#include <string.h>
#include <stdlib.h>

struct CallbackArrayObservedPilot {
    int count;
    DWORD capacity, growth, stride;
    int iterationDepth, hasPendingRemovals;
    void* records;
};
struct CallbackArrayRemovalPilot {
    CallbackArrayObservedPilot* owner;
    int index;
};
C_ASSERT(sizeof(CallbackArrayObservedPilot) == 0x1c);
C_ASSERT(offsetof(CallbackArrayObservedPilot, records) == 0x18);
C_ASSERT(sizeof(CallbackArrayRemovalPilot) == 8);

static CallbackArrayObservedPilot g_dialogWindowsPilot = {0, 0, 32, sizeof(HWND), 0, 0, NULL}; // 201f7540
static CallbackArrayObservedPilot g_callbackRemovalsPilot = {
    0, 0, 32, sizeof(CallbackArrayRemovalPilot), 0, 0, NULL
}; // 201f7098

// Reconstruction-only helper for the shared record address calculation.
static BYTE* CallbackArrayRecordPilot(const CallbackArrayObservedPilot* array, DWORD index)
{
    return static_cast<BYTE*>(array->records) + index * array->stride;
}

// Original: api.dll 0x2013bdcc (ordinal 172)
extern "C" void* __stdcall GetCallbackArrayRecordPilot(const CallbackArrayObservedPilot* array, DWORD index)
{
    if (index >= static_cast<DWORD>(array->count))
        return NULL;
    return CallbackArrayRecordPilot(array, index);
}

// 2013bc26.
extern "C" void* __stdcall AppendCallbackArrayRecordPilot(CallbackArrayObservedPilot* array)
{
    if (static_cast<DWORD>(array->count) == array->capacity) {
        array->capacity += array->growth;
        DWORD bytes = array->stride * array->capacity;
        if (bytes < array->capacity) return NULL;
        if (array->count == 0)
            AllocatePrivateHeapMemoryPilot(1, bytes, &array->records, 0);
        else
            ReallocatePrivateHeapMemoryBoundaryPilot(1, bytes, &array->records);
    }
    int index = array->count++;
    void* record = CallbackArrayRecordPilot(array, index);
    memset(record, 0, array->stride);
    return record;
}

static void RemoveCallbackArrayIndexPilot(CallbackArrayObservedPilot* array, int index)
{
    if (array->iterationDepth != 0) {
        array->hasPendingRemovals = 1;
        CallbackArrayRemovalPilot* removal = static_cast<CallbackArrayRemovalPilot*>(
            AppendCallbackArrayRecordPilot(&g_callbackRemovalsPilot));
        removal->index = index;
        removal->owner = array;
    } else {
        --array->count;
        if (index < array->count)
            memmove(CallbackArrayRecordPilot(array, index), CallbackArrayRecordPilot(array, index + 1),
                (array->count - index) * array->stride);
    }
}

// 2013bd13.
extern "C" void __stdcall RemoveCallbackArrayRecordPilot(CallbackArrayObservedPilot* array, void* record)
{
    DWORD offset = reinterpret_cast<DWORD>(record) - reinterpret_cast<DWORD>(array->records);
    RemoveCallbackArrayIndexPilot(array, static_cast<int>(offset / array->stride));
}

typedef void* (__stdcall *CallbackArrayVisitorPilot)(void*, void*);

// 2013bdee. Pending entries use the traversal ordinal for the skip test,
// even when the visited physical record is selected in reverse order.
extern "C" void* __stdcall VisitCallbackArrayPilot(CallbackArrayObservedPilot* array,
    void* context, CallbackArrayVisitorPilot visitor, int reverse)
{
    ++array->iterationDepth;
    void* result = NULL;
    for (int ordinal = 0; ordinal < array->count; ++ordinal) {
        if (array->hasPendingRemovals != 0) {
            const CallbackArrayRemovalPilot* removals =
                static_cast<const CallbackArrayRemovalPilot*>(g_callbackRemovalsPilot.records);
            int pending = g_callbackRemovalsPilot.count - 1;
            while (pending >= 0 &&
                (removals[pending].owner != array || removals[pending].index != ordinal)) --pending;
            if (pending >= 0) continue;
        }
        int index = reverse != 0 ? array->count - ordinal - 1 : ordinal;
        result = visitor(CallbackArrayRecordPilot(array, index), context);
        if (result != NULL) break;
    }
    if (--array->iterationDepth == 0 && array->hasPendingRemovals != 0) {
        for (int i = g_callbackRemovalsPilot.count - 1; i >= 0; --i) {
            const CallbackArrayRemovalPilot* removal =
                static_cast<const CallbackArrayRemovalPilot*>(g_callbackRemovalsPilot.records) + i;
            if (removal->owner == array) {
                RemoveCallbackArrayIndexPilot(array, removal->index);
                RemoveCallbackArrayIndexPilot(&g_callbackRemovalsPilot, i);
            }
        }
        array->hasPendingRemovals = 0;
    }
    return result;
}

// 2013246b / 63. A show request always appends, including duplicate HWNDs.
extern "C" int __stdcall SetRegisteredDialogVisiblePilot(HWND window, int show)
{
    HWND* entry = NULL;
    for (int i = 0; i < g_dialogWindowsPilot.count; ++i) {
        HWND* candidate = reinterpret_cast<HWND*>(CallbackArrayRecordPilot(&g_dialogWindowsPilot, i));
        if (*candidate == window) {
            entry = candidate;
            break;
        }
    }
    if (show != 0) {
        g_idleControllerPilot->SetDialogMode(1, true);
        entry = static_cast<HWND*>(AppendCallbackArrayRecordPilot(&g_dialogWindowsPilot));
        *entry = window;
        ShowWindow(window, SW_SHOW);
        UpdateWindow(*entry);
    } else {
        ShowWindow(entry == NULL ? window : *entry, SW_HIDE);
        UpdateWindow(entry == NULL ? window : *entry);
        g_idleControllerPilot->SetDialogMode(1, false);
        if (entry != NULL) RemoveCallbackArrayRecordPilot(&g_dialogWindowsPilot, entry);
    }
    return 0;
}

// 2013259f. Reload the HWND after the synchronous message.
extern "C" void* __stdcall DispatchRegisteredDialogMessagePilot(void* record, void* context)
{
    HWND* entry = static_cast<HWND*>(record);
    if (!IsWindow(*entry)) {
        RemoveCallbackArrayRecordPilot(&g_dialogWindowsPilot, entry);
        return NULL;
    }
    SendMessageA(*entry, 0x464, 0, 0);
    return IsDialogMessageA(*entry, static_cast<MSG*>(context)) ? entry : NULL;
}

// 20132546 / 64.
extern "C" int __stdcall DispatchRegisteredDialogsPilot(MSG* message)
{
    return VisitCallbackArrayPilot(&g_dialogWindowsPilot, message, DispatchRegisteredDialogMessagePilot, 0) != NULL;
}

// 2013256c. The controller hide callback leaves invalid HWNDs for dispatch.
extern "C" void* __stdcall HideRegisteredDialogPilot(void* record, void*)
{
    HWND* entry = static_cast<HWND*>(record);
    if (IsWindow(*entry)) {
        ShowWindow(*entry, SW_HIDE);
        RemoveCallbackArrayRecordPilot(&g_dialogWindowsPilot, entry);
    }
    return NULL;
}

extern "C" char g_configurationFilePilot[];
extern int g_savedControllerWindowModePilot; // 201f94fc

// 20125ee5, controller table slot +0x24. Explicit receiver in the source ABI.
extern "C" void __stdcall HideControllerWindowPilot(IdleControllerBoundaryPilot* controller)
{
    if (controller->QueryHiddenPosition()) return;
    BYTE* object = reinterpret_cast<BYTE*>(controller);
    HWND* window = reinterpret_cast<HWND*>(object + 4);
    if (!controller->QueryField34()) {
        g_savedControllerWindowModePilot = controller->QueryField38() ? 1 : (IsZoomed(*window) ? 2 : 0);
        char mode[2];
        _itoa_s(g_savedControllerWindowModePilot, mode, sizeof(mode), 10);
        WritePrivateProfileStringA("MAIN", "Maximized", mode, g_configurationFilePilot);
        if (controller->QueryField38()) {
            object[0x3f] = 1;
            SendMessageA(*window, WM_COMMAND, 0x1501, 0x50524f47);
            object[0x3f] = 0;
        }
    }
    SetWindowPos(*window, NULL, 20000, 20000, 0, 0, 0x15);
    VisitCallbackArrayPilot(&g_dialogWindowsPilot, NULL, HideRegisteredDialogPilot, 0);
}
