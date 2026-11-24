#include <windows.h>
#include <unknwn.h>

extern "C" void __cdecl FreeOwnedPrivateHeapPointerPilot(void* pointer); // api.dll 0x200f40a9
extern "C" HRESULT __stdcall FreePrefixedBufferStoragePilot(void* pointer); // api.dll 0x20150c15
extern "C" BOOL __stdcall FreeProcessHeapStoragePilot(void* pointer); // api.dll 0x2014e75f
extern "C" bool __stdcall HandleValueRejectedPilot(HANDLE value); // api.dll 0x20173c46

class InterfacePointerOwnerPilot {
public:
    InterfacePointerOwnerPilot* TakeFrom(IUnknown** source);
    static void __stdcall ReleaseIfPresent(InterfacePointerOwnerPilot* owner);

private:
    void Attach(IUnknown* replacement);
    void Release();
    IUnknown* value;
};

class PrivateHeapPointerOwnerPilot {
public:
    PrivateHeapPointerOwnerPilot* ReplaceAndReturn(void* replacement);
    PrivateHeapPointerOwnerPilot* ReplaceThroughOwner(void* replacement);

private:
    void Replace(void* replacement);
    void Release();
    void* value;
};

class PrefixedBufferOwnerPilot {
public:
    void Replace(void* replacement);
    PrefixedBufferOwnerPilot* ReplaceAndReturn(void* replacement);
    PrefixedBufferOwnerPilot* ReplaceThroughOwner(void* replacement);

private:
    void Release();
    void* value;
};

class MappedViewOwnerPilot {
public:
    void Release();
    void Replace(const void* replacement);
    MappedViewOwnerPilot* TakeFrom(const void** source);

private:
    friend class MappedFileOwnerPilot;
    const void* view;
};

#pragma optimize("s", on)
// Original: api.dll 0x20161815
__declspec(noinline) InterfacePointerOwnerPilot* InterfacePointerOwnerPilot::TakeFrom(IUnknown** source)
{
    IUnknown* transferred = *source;
    *source = NULL;
    Attach(transferred);
    return this;
}

// Original: api.dll 0x20161834
__declspec(noinline) PrivateHeapPointerOwnerPilot* PrivateHeapPointerOwnerPilot::ReplaceAndReturn(void* replacement)
{
    Replace(replacement);
    return this;
}

// Original: api.dll 0x2016184b
__declspec(noinline) void PrefixedBufferOwnerPilot::Replace(void* replacement)
{
    Release();
    value = replacement;
}

// Original: api.dll 0x20161862
__declspec(noinline) PrefixedBufferOwnerPilot* PrefixedBufferOwnerPilot::ReplaceAndReturn(void* replacement)
{
    Replace(replacement);
    return this;
}

// Original: api.dll 0x201611be
__declspec(noinline) void InterfacePointerOwnerPilot::Attach(IUnknown* replacement)
{
    Release();
    value = replacement;
}

// Original: api.dll 0x201611d5
__declspec(noinline) void PrivateHeapPointerOwnerPilot::Replace(void* replacement)
{
    Release();
    value = replacement;
}

// Original: api.dll 0x201611ec
__declspec(noinline) void MappedViewOwnerPilot::Release()
{
    if (view != NULL) {
        UnmapViewOfFile(view);
        view = NULL;
    }
}

// Original: api.dll 0x20161203
__declspec(noinline) void InterfacePointerOwnerPilot::Release()
{
    if (value != NULL) {
        value->Release();
        value = NULL;
    }
}

// Original: api.dll 0x20161030
__declspec(noinline) void PrivateHeapPointerOwnerPilot::Release()
{
    if (value != NULL) {
        FreeOwnedPrivateHeapPointerPilot(value);
        value = NULL;
    }
}

// Original: api.dll 0x2015115f
__declspec(noinline) void PrefixedBufferOwnerPilot::Release()
{
    if (value != NULL) {
        FreePrefixedBufferStoragePilot(value);
        value = NULL;
    }
}

// Original: api.dll 0x20161219
__declspec(noinline) void MappedViewOwnerPilot::Replace(const void* replacement)
{
    Release();
    view = replacement;
}

// Original: api.dll 0x20161230
__declspec(noinline) MappedViewOwnerPilot* MappedViewOwnerPilot::TakeFrom(const void** source)
{
    const void* transferred = *source;
    *source = NULL;
    Replace(transferred);
    return this;
}

// Original: api.dll 0x2016124f
extern "C" LONG __stdcall ShareReferenceCounterPilot(LONG* counter, LONG** slot)
{
    *slot = counter;
    return InterlockedIncrement(counter);
}

class ProcessHeapPointerOwnerPilot {
public:
    void Replace(void* replacement);
    ProcessHeapPointerOwnerPilot* ReplaceAndReturn(void* replacement);

private:
    friend class ProcessHeapBufferOwnerPilot;
    void Release();
    void* value;
};

// Original: api.dll 0x20161047
__declspec(noinline) void ProcessHeapPointerOwnerPilot::Release()
{
    if (value != NULL) {
        FreeProcessHeapStoragePilot(value);
        value = NULL;
    }
}

// Original: api.dll 0x2016194d
__declspec(noinline) void ProcessHeapPointerOwnerPilot::Replace(void* replacement)
{
    Release();
    value = replacement;
}

struct CriticalSectionStoragePilot {
    volatile LONG activeUsers;
    HANDLE idleEvent;
    CRITICAL_SECTION section;

    void EnterWhenIdle();
};

class CriticalSectionLeasePilot {
public:
    void Release();
    void Replace(CriticalSectionStoragePilot* replacement);

private:
    CriticalSectionStoragePilot* storage;
};

// Original: api.dll 0x2016105d
__declspec(noinline) void CriticalSectionLeasePilot::Release()
{
    if (storage != NULL) {
        LeaveCriticalSection(&storage->section);
        storage = NULL;
    }
}

// Original: api.dll 0x20161937
extern "C" HRESULT __stdcall TransferInterfacePointerPilot(InterfacePointerOwnerPilot* owner, IUnknown** source)
{
    owner->TakeFrom(source);
    return S_OK;
}

class FileMappingHandleOwnerPilot {
public:
    FileMappingHandleOwnerPilot* TakeFrom(FileMappingHandleOwnerPilot* source);
    FileMappingHandleOwnerPilot* ReplaceAndReturn(HANDLE replacement);
    FileMappingHandleOwnerPilot* ReplaceThroughOwner(HANDLE replacement);

private:
    friend class MappedFileOwnerPilot;
    void Close();
    void Replace(HANDLE replacement);
    HANDLE value;
};

// Original: api.dll 0x20160f6d
__declspec(noinline) void FileMappingHandleOwnerPilot::Close()
{
    const HANDLE current = value;
    if (!HandleValueRejectedPilot(current)) {
        CloseHandle(current);
        value = INVALID_HANDLE_VALUE;
    }
}

// Original: api.dll 0x20160f45
__declspec(noinline) void FileMappingHandleOwnerPilot::Replace(HANDLE replacement)
{
    Close();
    value = HandleValueRejectedPilot(replacement) ? INVALID_HANDLE_VALUE : replacement;
}

// Original: api.dll 0x20161077
__declspec(noinline) FileMappingHandleOwnerPilot* FileMappingHandleOwnerPilot::ReplaceAndReturn(HANDLE replacement)
{
    Replace(replacement);
    return this;
}

// Original: api.dll 0x20161669
FileMappingHandleOwnerPilot* FileMappingHandleOwnerPilot::ReplaceThroughOwner(HANDLE replacement)
{
    ReplaceAndReturn(replacement);
    return this;
}

// Original: api.dll 0x20161680
__declspec(noinline) FileMappingHandleOwnerPilot* FileMappingHandleOwnerPilot::TakeFrom(FileMappingHandleOwnerPilot* source)
{
    const HANDLE transferred = source->value;
    source->value = INVALID_HANDLE_VALUE;
    Replace(transferred);
    return this;
}

class MappedFileOwnerPilot {
public:
    MappedFileOwnerPilot* TakeFrom(MappedFileOwnerPilot* source);
    void Close();
    static void __stdcall CloseIfPresent(MappedFileOwnerPilot* owner);

private:
    MappedViewOwnerPilot view;
    FileMappingHandleOwnerPilot handle;
};

// Original: api.dll 0x20161964
__declspec(noinline) MappedFileOwnerPilot* MappedFileOwnerPilot::TakeFrom(MappedFileOwnerPilot* source)
{
    view.TakeFrom(&source->view.view);
    handle.TakeFrom(&source->handle);
    return this;
}

// Original: api.dll 0x20161e65
extern "C" HRESULT __stdcall TransferMappedFilePilot(MappedFileOwnerPilot* owner, MappedFileOwnerPilot* source)
{
    owner->TakeFrom(source);
    return S_OK;
}

#pragma optimize("", on)
// Original: api.dll 0x2014eb01
__declspec(noinline) void CriticalSectionStoragePilot::EnterWhenIdle()
{
    EnterCriticalSection(&section);
    if (activeUsers <= 0)
        return;
    const HANDLE event = idleEvent;
    WaitForSingleObject(event, INFINITE);
}

#pragma optimize("s", on)
// Original: api.dll 0x20161c75
void CriticalSectionLeasePilot::Replace(CriticalSectionStoragePilot* replacement)
{
    Release();
    storage = replacement;
    if (replacement != NULL)
        replacement->EnterWhenIdle();
}

class ActivityCounterPilot {
public:
    void AddActivity();
    void RemoveActivity();

private:
    CriticalSectionStoragePilot state;
    CRITICAL_SECTION counterSection;
};

// Original: api.dll 0x20161785
__declspec(noinline) void ActivityCounterPilot::AddActivity()
{
    EnterCriticalSection(&state.section);
    EnterCriticalSection(&counterSection);
    if (InterlockedIncrement(&state.activeUsers) == 1)
        ResetEvent(state.idleEvent);
    LeaveCriticalSection(&counterSection);
    LeaveCriticalSection(&state.section);
}

// Original: api.dll 0x201617cd
__declspec(noinline) void ActivityCounterPilot::RemoveActivity()
{
    EnterCriticalSection(&counterSection);
    if (InterlockedDecrement(&state.activeUsers) == 0)
        SetEvent(state.idleEvent);
    LeaveCriticalSection(&counterSection);
}

class ActivityLeasePilot {
public:
    void Replace(ActivityCounterPilot* replacement);

private:
    void Release();
    ActivityCounterPilot* counter;
};

// Original: api.dll 0x20161c95
__declspec(noinline) void ActivityLeasePilot::Release()
{
    if (counter != NULL) {
        counter->RemoveActivity();
        counter = NULL;
    }
}

// Original: api.dll 0x20161caa
void ActivityLeasePilot::Replace(ActivityCounterPilot* replacement)
{
    Release();
    counter = replacement;
    if (replacement != NULL)
        replacement->AddActivity();
}

// Original: api.dll 0x20161bdd
PrivateHeapPointerOwnerPilot* PrivateHeapPointerOwnerPilot::ReplaceThroughOwner(void* replacement)
{
    ReplaceAndReturn(replacement);
    return this;
}

class ProcessHeapBufferOwnerPilot {
public:
    void Reset();

private:
    ProcessHeapPointerOwnerPilot storage;
    DWORD stateAt04;
    DWORD stateAt08;
};

// Original: api.dll 0x20161bf4
void ProcessHeapBufferOwnerPilot::Reset()
{
    storage.Release();
    stateAt08 = 0;
    stateAt04 = 0;
}

// Original: api.dll 0x20161c5e
PrefixedBufferOwnerPilot* PrefixedBufferOwnerPilot::ReplaceThroughOwner(void* replacement)
{
    ReplaceAndReturn(replacement);
    return this;
}

// Original: api.dll 0x20161d26
ProcessHeapPointerOwnerPilot* ProcessHeapPointerOwnerPilot::ReplaceAndReturn(void* replacement)
{
    Replace(replacement);
    return this;
}

// Original: api.dll 0x20162571
__declspec(noinline) void MappedFileOwnerPilot::Close()
{
    handle.Close();
    view.Release();
}

// Original: api.dll 0x20162fd5
void __stdcall InterfacePointerOwnerPilot::ReleaseIfPresent(InterfacePointerOwnerPilot* owner)
{
    if (owner != NULL)
        owner->Release();
}

// Original: api.dll 0x20163585
void __stdcall MappedFileOwnerPilot::CloseIfPresent(MappedFileOwnerPilot* owner)
{
    if (owner != NULL)
        owner->Close();
}
#pragma optimize("", on)
