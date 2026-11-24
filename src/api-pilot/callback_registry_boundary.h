#pragma once
#include <windows.h>
typedef void (__cdecl *RegisteredCallbackBoundaryPilot)(ULONG_PTR, ULONG_PTR);
struct CallbackListEntryBoundaryPilot {
    RegisteredCallbackBoundaryPilot callback;
    DWORD flags;
    ULONG_PTR context1;
    ULONG_PTR context2;
};
// Observed header only. Variable-capacity entries follow it in raw heap storage.
struct CallbackListDataBoundaryPilot {
    DWORD dispatchDepth;
    DWORD count;
    DWORD capacity;
    DWORD pendingCompaction;
};
typedef char CallbackEntryMustBe16[(sizeof(CallbackListEntryBoundaryPilot) == 16) ? 1 : -1];
typedef char CallbackHeaderMustBe16[(sizeof(CallbackListDataBoundaryPilot) == 16) ? 1 : -1];
// Minimum observed pointer owner, not an identified original C++ class.
class CallbackRegistryOwnerBoundaryPilot {
public:
    CallbackListDataBoundaryPilot* data;
    int __thiscall Register(RegisteredCallbackBoundaryPilot callback, int withContext,
        ULONG_PTR context1, ULONG_PTR context2);
};
int __stdcall RegisterCallbackListOwnerBoundaryPilot(
    CallbackListDataBoundaryPilot** owner, RegisteredCallbackBoundaryPilot callback,
    int withContext, ULONG_PTR context1, ULONG_PTR context2);
