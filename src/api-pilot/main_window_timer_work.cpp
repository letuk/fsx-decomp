typedef void (WINAPI *TimerInteractionCallbackPilot)(void*, DWORD);

struct TimerReleaseOwnerObservedPilot {
    RECT bounds;
    LONG originX, originY;
    DWORD contextWord;
    void* hoverText;
};

struct TimerInteractionObservedPilot {
    DWORD kind;
    LONG x, y, width, height;
    DWORD cursorKind;
    DWORD inputGroup;
    DWORD flags;
    DWORD eventCode;
    TimerInteractionCallbackPilot callback;
    TimerReleaseOwnerObservedPilot* releaseOwner;
};
C_ASSERT(offsetof(TimerInteractionObservedPilot, callback) == 0x24);
C_ASSERT(offsetof(TimerInteractionObservedPilot, releaseOwner) == 0x28);
C_ASSERT(sizeof(TimerInteractionObservedPilot) == 0x2c);

extern TimerInteractionObservedPilot* g_timerActivityFirstPilot; // 2020e068
extern TimerInteractionObservedPilot* g_timerActivitySecondPilot; // 2020e08c
extern HWND g_timerCaptureWindowPilot; // 2020e070
extern DWORD g_timerFirstCallbackArgumentPilot; // 2020e06c
extern DWORD g_timerSecondFlagsPilot; // 2020d890
extern DWORD g_timerSecondInitialDelayPilot; // 2020d464
extern DWORD g_timerSecondCountdownPilot; // 201f995c
extern DWORD g_timerSecondReloadPilot; // 2020d34c
extern DWORD g_timerFirstCountdownPilot; // 201f7120
extern DWORD g_timerFirstReloadPilot; // 2020d898
extern BYTE g_timerFirstCallbackContextPilot[0x1c]; // 2020d89c
extern BYTE g_timerSecondCallbackContextPilot[0x1c]; // 2020d874
extern TimerInteractionObservedPilot* g_timerFirstCallbackRecordPilot; // 2020d8a8
extern int g_cursorSuppressedPilot; // 201f7160
extern "C" void __cdecl EndTimerCaptureSourcePilot(); // 20137891
extern "C" void __cdecl EndTimerRepeatSourcePilot(); // 201379d1
extern "C" void __stdcall BeginTimerCaptureSourcePilot(
    TimerInteractionObservedPilot*, void*, DWORD); // 201378f2
extern "C" void __stdcall BeginTimerRepeatSourcePilot(
    TimerInteractionObservedPilot*, DWORD, void*); // 20137988

static DWORD ReadPanelPointerKeysPilot()
{
    static const struct { int key; DWORD normal; DWORD swapped; } keys[] = {
        { VK_LBUTTON, 1, 2 }, { VK_RBUTTON, 2, 1 }, { VK_MBUTTON, 0x10, 0 },
        { 0x05, 0x20, 0 }, { 0x06, 0x40, 0 }, // XBUTTON1/2 are absent from the retained SDK headers.
        { VK_CONTROL, 8, 0 }, { VK_SHIFT, 4, 0 }
    }; // 200f3188
    bool swapped = GetSystemMetrics(SM_SWAPBUTTON) != 0;
    DWORD down = 0;
    for (unsigned int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        if (GetAsyncKeyState(keys[i].key) < 0)
            down |= swapped && keys[i].swapped != 0 ? keys[i].swapped : keys[i].normal;
    }
    return down;
}

// 20137a9a. GetAsyncKeyState reports the physical mouse buttons.
extern "C" void __stdcall ReconcileTimerInteractionPilot(int releaseWithoutButtons)
{
    if (g_timerActivityFirstPilot == NULL && g_timerActivitySecondPilot == NULL) return;
    DWORD down = ReadPanelPointerKeysPilot();
    if (g_timerActivityFirstPilot != NULL && GetCapture() != g_timerCaptureWindowPilot)
        EndTimerCaptureSourcePilot();
    if (releaseWithoutButtons != 0 && g_timerActivityFirstPilot != NULL && (down & 0x13) == 0)
        EndTimerCaptureSourcePilot();
    if (g_timerActivitySecondPilot != NULL) {
        DWORD flags = g_timerSecondFlagsPilot;
        if (((flags & 0x20000000) != 0 && (down & 1) == 0) ||
            ((flags & 0x40000000) != 0 && (down & 0x10) == 0) ||
            ((flags & 0x80000000) != 0 && (down & 2) == 0)) {
            EndTimerRepeatSourcePilot();
        }
    }
}

// 20136d48. Only AL of the original result is defined.
extern "C" bool __stdcall DispatchTimerInteractionPilot(TimerInteractionObservedPilot* record,
    DWORD flags, DWORD captureFlags, void* context, BYTE beginRepeat, DWORD eventSource)
{
    if ((record->flags & captureFlags) != 0)
        BeginTimerCaptureSourcePilot(record, context, captureFlags);
    if ((record->flags & flags) == 0) return false;

    DWORD event = record->eventCode;
    if (event == 0) {
        if (record->callback != NULL) record->callback(context, flags);
    } else if (((event >= 0x10000 && event <= 0x10547) ||
                (event >= 0x11000 && event <= 0x1ffff) || event - 0x1000 <= 0x700) &&
               g_mainWindowEventPilot != NULL) {
        g_mainWindowEventPilot(event, 0, eventSource);
    }
    if (beginRepeat != 0 && (record->flags & 0x200000) != 0)
        BeginTimerRepeatSourcePilot(record, flags, context);
    return true;
}

// 20137b6b. Countdown dispatch tests the value before decrementing.
extern "C" void __cdecl ProcessMainWindowTimerWorkPilot()
{
    ReconcileTimerInteractionPilot(0);
    if (g_timerActivitySecondPilot != NULL) {
        if (g_timerSecondInitialDelayPilot != 0) {
            --g_timerSecondInitialDelayPilot;
            g_timerSecondCountdownPilot = g_timerSecondReloadPilot;
        } else if (g_timerSecondCountdownPilot-- == 0) {
            DispatchTimerInteractionPilot(g_timerActivitySecondPilot, g_timerSecondFlagsPilot,
                0, g_timerSecondCallbackContextPilot, 0, 2);
            g_timerSecondCountdownPilot = g_timerSecondReloadPilot;
        }
    }
    if (g_timerActivityFirstPilot != NULL && (g_timerActivityFirstPilot->flags & 0x1000) != 0 &&
        g_timerFirstCountdownPilot-- == 0) {
        TimerInteractionCallbackPilot callback = g_timerFirstCallbackRecordPilot->callback;
        if (callback != NULL) callback(g_timerFirstCallbackContextPilot, g_timerFirstCallbackArgumentPilot);
        g_timerFirstCountdownPilot = g_timerFirstReloadPilot;
    }
    g_cursorSuppressedPilot = 100;
}

extern ClipWindowObservedPilot* g_timerInputWindowPilot; // 2020e07c
extern ClipWindowObservedPilot* g_timerCapturedInputWindowPilot; // 2020d8bc
extern DWORD g_timerCaptureInputWordPilot; // 2020e088
extern DWORD g_timerSavedCaptureInputWordPilot; // 2020d8b8
extern DWORD g_timerRepeatInputWordPilot; // 2020e080
extern DWORD g_timerSavedRepeatInputWordPilot; // 2020d894

static void DispatchTimerInteractionReleasePilot(TimerInteractionObservedPilot* record)
{
    if ((record->flags & 0x800) != 0 && record->releaseOwner != NULL) {
        struct {
            DWORD first, second, ownerWord;
            TimerInteractionObservedPilot* record;
            BYTE unknown10[0xc];
        } context;
        context.first = 0;
        context.second = 0;
        context.ownerWord = record->releaseOwner->contextWord;
        context.record = record;
        DispatchTimerInteractionPilot(record, 0x800, 0, &context, 0, 0);
    }
}

// 20137891. Dispatch while the capture record is still published.
extern "C" void __cdecl EndTimerCaptureSourcePilot()
{
    if (g_timerActivityFirstPilot == NULL) return;
    DispatchTimerInteractionReleasePilot(g_timerActivityFirstPilot);
    g_timerActivityFirstPilot = NULL;
    ReleaseCapture();
}

// 201379d1.
extern "C" void __cdecl EndTimerRepeatSourcePilot()
{
    if (g_timerActivitySecondPilot == NULL) return;
    DispatchTimerInteractionReleasePilot(g_timerActivitySecondPilot);
    g_timerActivitySecondPilot = NULL;
}

// 201378f2. Original record and context arrive in ECX and EDX.
extern "C" void __stdcall BeginTimerCaptureSourcePilot(
    TimerInteractionObservedPilot* record, void* context, DWORD flags)
{
    ClipWindowObservedPilot* window = g_timerInputWindowPilot;
    if (record != NULL && window != NULL &&
        (g_timerActivityFirstPilot != record || g_timerCapturedInputWindowPilot != window)) {
        g_timerActivityFirstPilot = record;
        g_timerFirstCallbackArgumentPilot = flags;
        DWORD inputWord = g_timerCaptureInputWordPilot;
        memcpy(g_timerFirstCallbackContextPilot, context, sizeof(g_timerFirstCallbackContextPilot));
        g_timerCapturedInputWindowPilot = window;
        ClipWindowLinkObservedPilot* link = window->link;
        g_timerFirstReloadPilot = 1;
        g_timerFirstCountdownPilot = 1;
        g_timerSavedCaptureInputWordPilot = inputWord;
        g_timerCaptureWindowPilot = NULL;
        if (link != NULL) {
            g_timerCaptureWindowPilot = link->window;
            SetCapture(g_timerCaptureWindowPilot);
        }
    }
    EndTimerRepeatSourcePilot();
}

// 20137988. Original record and flags arrive in EAX and ECX.
extern "C" void __stdcall BeginTimerRepeatSourcePilot(
    TimerInteractionObservedPilot* record, DWORD flags, void* context)
{
    if (record == NULL) return;
    g_timerSecondFlagsPilot = flags;
    memcpy(g_timerSecondCallbackContextPilot, context, sizeof(g_timerSecondCallbackContextPilot));
    g_timerActivitySecondPilot = record;
    g_timerSavedRepeatInputWordPilot = g_timerRepeatInputWordPilot;
    g_timerSecondInitialDelayPilot = 10;
    g_timerSecondReloadPilot = 1;
}
