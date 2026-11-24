struct PanelInputMessagePilot {
    HWND window;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    POINT point;
};
C_ASSERT(offsetof(PanelInputMessagePilot, point) == 0x10);
static const UINT PanelMouseWheelMessagePilot = 0x20a;
static const int PanelMouseWheelDeltaPilot = 120;

extern DWORD g_inputCursorPilot; // 2020e078
extern DWORD g_inputHoverPilot; // 2020e074
extern BYTE g_inputSuppressPostprocessingPilot; // 2020e084
extern "C" ClipWindowObservedPilot* __cdecl GetAlternatePanelInputWindowPilot(); // window.dll ordinal 23
extern "C" void __stdcall DispatchPanelContextClickPilot(
    ClipGeometryLinkObservedPilot*, const PanelInputMessagePilot*, DWORD, DWORD); // 2012d08c

struct PanelInputHeaderPilot {
    HWND window;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
};

struct PanelInteractionContextPilot {
    POINT relative;
    DWORD ownerWord;
    TimerInteractionObservedPilot* record;
    POINT original;
    PanelInputHeaderPilot* header;
};
C_ASSERT(sizeof(PanelInteractionContextPilot) == 0x1c);

// 20136e98. Original record in ECX, message in EAX and group on stack.
static bool DispatchCapturedPanelInputPilot(TimerInteractionObservedPilot* record,
    const PanelInputMessagePilot* input, DWORD group)
{
    if (record == NULL) return false;
    PanelInputHeaderPilot header = { input->window, input->message, input->wParam, input->lParam };
    PanelInteractionContextPilot context = {
        input->point, g_timerCaptureInputWordPilot, record, input->point, &header
    };
    if (record->releaseOwner != NULL) {
        context.relative.x -= record->releaseOwner->bounds.left;
        context.relative.y -= record->releaseOwner->bounds.top;
        context.ownerWord = record->releaseOwner->contextWord;
    }
    DWORD flags = record->flags;
    if ((flags & 0x400) != 0) {
        g_inputSuppressPostprocessingPilot = 1;
        if (header.message < 0x200 || header.message > 0x20d) return false;
        if (record->callback != NULL) record->callback(&context, 0x400);
        return true;
    }

    switch (header.message) {
    case WM_NCHITTEST: return flags != 0;
    case WM_MOUSEMOVE:
        if (g_timerActivityFirstPilot == record) {
            if (record->callback != NULL) record->callback(&context, g_timerFirstCallbackArgumentPilot);
            g_timerFirstCountdownPilot = g_timerFirstReloadPilot;
            memcpy(g_timerFirstCallbackContextPilot, &context, sizeof(context));
            return true;
        }
        return DispatchTimerInteractionPilot(record, 0x400000, 0, &context, 0, 3);
    case WM_LBUTTONDOWN: return DispatchTimerInteractionPilot(record, 0x20000000, 0x800000, &context, 1, 2);
    case WM_LBUTTONUP: return DispatchTimerInteractionPilot(record, 0x20000, 0, &context, 0, 2);
    case WM_LBUTTONDBLCLK: return DispatchTimerInteractionPilot(record, 0x4000000, 0, &context, 0, 2);
    case WM_RBUTTONUP: return DispatchTimerInteractionPilot(record, 0x80000, 0, &context, 0, 2);
    case WM_RBUTTONDBLCLK: return DispatchTimerInteractionPilot(record, 0x10000000, 0, &context, 0, 2);
    case WM_MBUTTONDOWN: return DispatchTimerInteractionPilot(record, 0x40000000, 0x1000000, &context, 1, 2);
    case WM_MBUTTONUP: return DispatchTimerInteractionPilot(record, 0x40000, 0, &context, 0, 2);
    case WM_MBUTTONDBLCLK: return DispatchTimerInteractionPilot(record, 0x8000000, 0, &context, 0, 2);
    case 0x468: return DispatchTimerInteractionPilot(record, 0x100000, 0, &context, 0, 1);
    case WM_RBUTTONDOWN:
    case WM_NCRBUTTONDOWN: {
        if (header.message == WM_RBUTTONDOWN) {
            if (DispatchTimerInteractionPilot(record, 0x80000000, 0x2000000, &context, 1, 2)) return true;
            if ((record->flags & 0x12000000) != 0) return false;
        }
        if (group == 0) return false;
        ClipWindowObservedPilot* target = g_timerInputWindowPilot;
        if (target != NULL && (target->flags & 0x100) != 0) target = GetAlternatePanelInputWindowPilot();
        if (target != NULL && target->link != NULL && target->link->geometryLink != NULL)
            DispatchPanelContextClickPilot(target->link->geometryLink, input, group, 1);
        return true;
    }
    case PanelMouseWheelMessagePilot: {
        int delta = static_cast<short>(HIWORD(input->wParam));
        DWORD action = (flags & 0x6000) == 0 ? 0x20000000 : (delta > 0 ? 0x4000 : 0x2000);
        if ((flags & action) == 0) return false;
        int magnitude = delta < 0 ? -delta : delta;
        for (int offset = 0; offset < magnitude; offset += PanelMouseWheelDeltaPilot)
            DispatchTimerInteractionPilot(context.record, action, 0, &context, 0, 3);
        return true;
    }
    default: return false;
    }
}

struct PanelInputHitGroupPilot {
    RECT bounds;
    TimerInteractionObservedPilot* records;
};
C_ASSERT(offsetof(PanelInputHitGroupPilot, records) == 0x10);

static PanelInputHitGroupPilot* FindPanelHitGroupPilot(CallbackArrayObservedPilot* groups,
    TimerInteractionObservedPilot* records)
{
    for (int index = 0; index < groups->count; ++index) {
        PanelInputHitGroupPilot* candidate =
            reinterpret_cast<PanelInputHitGroupPilot*>(CallbackArrayRecordPilot(groups, index));
        if (candidate->records == records) return candidate;
    }
    return NULL;
}

extern "C" char g_savedWindowLocationPilot[]; // 2020d770

static void* AllocatePanelHitStoragePilot(SIZE_T bytes)
{
    if (g_heapInitializedPilot == 0) InitializeHeapManagerPilot();
    void* memory = HeapAlloc(g_privateHeapPilot, 0, bytes);
    if (memory == NULL) {
        memory = HeapAlloc(g_privateHeapPilot, 0, bytes);
        if (memory == NULL) {
            HMODULE module = g_languageModulePilot;
            if (module == NULL) module = GetModuleHandleA(NULL);
            LoadStringA(module, 8, g_savedWindowLocationPilot, MAX_PATH);
            MessageBoxA(NULL, g_savedWindowLocationPilot, NULL, MB_ICONHAND);
            TerminateProcess(GetCurrentProcess(), 1);
        }
    }
    return memory;
}

C_ASSERT(sizeof(TimerReleaseOwnerObservedPilot) == 0x20);

// 2013670f. The returned count includes the closing or terminating record.
extern "C" int __stdcall BuildPanelHitBoundsPilot(ClipWindowObservedPilot* window,
    TimerInteractionObservedPilot* records, LONG originX, LONG originY,
    RECT* groupBounds, DWORD contextWord, int update)
{
    int index = 0;
    for (; records[index].kind != 0 && records[index].kind != 3; ++index) {
        TimerInteractionObservedPilot* record = &records[index];
        if (record->kind == 4) continue;
        TimerReleaseOwnerObservedPilot* owner = record->releaseOwner;
        if (update == 0 || owner == NULL) {
            owner = static_cast<TimerReleaseOwnerObservedPilot*>(AllocatePanelHitStoragePilot(sizeof(*owner)));
            record->releaseOwner = owner;
            owner->hoverText = NULL;
        }
        RECT& bounds = owner->bounds;
        bounds.left = originX + record->x;
        bounds.top = originY + record->y;
        bounds.right = bounds.left + record->width + 1;
        bounds.bottom = bounds.top + record->height + 1;
        owner->originX = originX;
        owner->originY = originY;
        owner->contextWord = contextWord;

        if (groupBounds != NULL) {
            if (bounds.left < groupBounds->left) groupBounds->left = bounds.left;
            if (bounds.top < groupBounds->top) groupBounds->top = bounds.top;
            if (bounds.right > groupBounds->right) groupBounds->right = bounds.right;
            if (bounds.bottom > groupBounds->bottom) groupBounds->bottom = bounds.bottom;
        }
        if (record->kind == 1)
            index += BuildPanelHitBoundsPilot(window, record + 1, bounds.left, bounds.top,
                groupBounds, contextWord, update);

        RefreshPanelHoverTextPilot(window, reinterpret_cast<DWORD>(record));
    }
    return index + 1;
}

// 20136645. The original window arrives in EAX.
extern "C" int __stdcall RegisterPanelHitGroupPilot(ClipWindowObservedPilot* window,
    TimerInteractionObservedPilot* records, LONG originX, LONG originY, DWORD contextWord, int update)
{
    if (window == NULL || window->link == NULL) return -1;
    ClipWindowLinkObservedPilot* link = window->link;
    if (link->hoverNotificationWindow == NULL) CreatePanelHoverWindowPilot(window);

    CallbackArrayObservedPilot* groups = reinterpret_cast<CallbackArrayObservedPilot*>(&link->field2c);
    PanelInputHitGroupPilot* group = FindPanelHitGroupPilot(groups, records);
    if (update == 0) {
        if (group != NULL) return -1;
        group = static_cast<PanelInputHitGroupPilot*>(AppendCallbackArrayRecordPilot(groups));
    } else {
        if (group == NULL || records->releaseOwner == NULL) return -1;
        originX = records->releaseOwner->originX;
        originY = records->releaseOwner->originY;
    }
    SetRect(&group->bounds, window->surfaceWidth, window->surfaceHeight, 0, 0);
    group->records = records;
    return BuildPanelHitBoundsPilot(window, records, originX, originY, &group->bounds, contextWord, update) != 0 ?
        0 : -1;
}

// 201362f6 / 146 and 20136317 / 147.
extern "C" int __stdcall AddPanelHitGroupPilot(ClipWindowObservedPilot* window,
    TimerInteractionObservedPilot* records, LONG x, LONG y)
{
    return RegisterPanelHitGroupPilot(window, records, x, y, 0, 0);
}

extern "C" int __stdcall RefreshPanelHitGroupPilot(ClipWindowObservedPilot* window,
    TimerInteractionObservedPilot* records)
{
    return RegisterPanelHitGroupPilot(window, records, 0, 0, records->releaseOwner->contextWord, 1);
}

// 20136339 / 186 and 2013635c / 187.
extern "C" int __stdcall AddPanelHitGroupWithContextPilot(ClipWindowObservedPilot* window,
    TimerInteractionObservedPilot* records, LONG x, LONG y, DWORD contextWord)
{
    return RegisterPanelHitGroupPilot(window, records, x, y, contextWord, 0);
}

extern "C" int __stdcall RefreshPanelHitGroupWithContextPilot(ClipWindowObservedPilot* window,
    TimerInteractionObservedPilot* records, DWORD contextWord)
{
    return RegisterPanelHitGroupPilot(window, records, 0, 0, contextWord, 1);
}

// 20136b21. Clear a published hover before disposing of its record.
static int ClearRemovedPanelHoverPilot(ClipWindowObservedPilot* window, DWORD recordKey)
{
    int result = 0;
    if (g_currentPanelHoverPilot == recordKey) {
        if (g_activeClipWindowPilot != NULL) result = SetPanelHoverRecordPilot(g_activeClipWindowPilot, 0);
        g_activeClipWindowPilot = NULL;
        g_currentPanelHoverPilot = 0;
    }
    if (window != NULL) {
        ClipWindowLinkObservedPilot* link = window->link;
        if (link == NULL) return -1;
        if (link->hoverRecord == recordKey) {
            result = SetPanelHoverRecordPilot(window, 0);
            link->hoverRecord = 0;
        }
    }
    return result;
}

// 201368ce. The capture and repeat comparisons apply at each recursion entry.
extern "C" int __stdcall ReleasePanelHitBoundsPilot(ClipWindowObservedPilot* window,
    TimerInteractionObservedPilot* records)
{
    if (records == g_timerActivityFirstPilot) EndTimerCaptureSourcePilot();
    if (records == g_timerActivitySecondPilot) g_timerActivitySecondPilot = NULL;

    int index = 0;
    for (; records[index].kind != 0 && records[index].kind != 3; ++index) {
        TimerInteractionObservedPilot* record = &records[index];
        if (record->kind == 4) continue;
        ClearRemovedPanelHoverPilot(window, reinterpret_cast<DWORD>(record));
        TimerReleaseOwnerObservedPilot* owner = record->releaseOwner;
        if (owner != NULL) FreePrivateHeapSlotPilot(&owner->hoverText);
        if (record->releaseOwner != NULL) {
            HeapFree(g_privateHeapPilot, 0, record->releaseOwner);
            record->releaseOwner = NULL;
        }
        if (record->kind == 1) index += ReleasePanelHitBoundsPilot(window, record + 1);
    }
    return index + 1;
}

// 20136ab8. The original window arrives in EAX.
extern "C" int __stdcall DestroyPanelHoverWindowPilot(ClipWindowObservedPilot* window)
{
    ClipWindowLinkObservedPilot* link = window->link;
    if (link == NULL) return -1;
    DestroyWindow(link->hoverNotificationWindow);
    link->hoverNotificationWindow = NULL;
    link->hoverRecord = 0;
    return 0;
}

// 2013637b / 148. The original requires an attached link on a nonnull window.
extern "C" int __stdcall RemovePanelHitGroupPilot(ClipWindowObservedPilot* window,
    TimerInteractionObservedPilot* records)
{
    if (window == NULL) return -1;
    CallbackArrayObservedPilot* groups = reinterpret_cast<CallbackArrayObservedPilot*>(&window->link->field2c);
    g_panelHoverResetPilot = 1;
    PanelInputHitGroupPilot* group = FindPanelHitGroupPilot(groups, records);
    if (group == NULL) return -1;
    RemoveCallbackArrayRecordPilot(groups, group);
    if (ReleasePanelHitBoundsPilot(window, records) == 0) return -1;
    if (groups->count == 0) DestroyPanelHoverWindowPilot(window);
    return 0;
}

// 20136409 / 214. The first 80 bytes determine whether an update is needed.
extern "C" int __stdcall SetPanelHitHoverTextPilot(ClipWindowObservedPilot* window,
    TimerInteractionObservedPilot* record, const char* text)
{
    if (window == NULL || record == NULL || window->link == NULL || record->releaseOwner == NULL)
        return -1;

    TimerReleaseOwnerObservedPilot* owner = record->releaseOwner;
    if (text == NULL) {
        if (owner->hoverText == NULL) return 0;
        FreePrivateHeapSlotPilot(&owner->hoverText);
    } else {
        if (owner->hoverText != NULL && strncmp(text, static_cast<const char*>(owner->hoverText), 0x50) == 0)
            return 0;
        if (owner->hoverText == NULL) owner->hoverText = AllocatePanelHitStoragePilot(0x51);
        CopyResolvedPathPilot(static_cast<char*>(owner->hoverText), text, 0x51);
    }
    RefreshPanelHoverTextPilot(window, reinterpret_cast<DWORD>(record));
    return 0;
}

// The alternating cursor kinds distinguish the two directions of a wheel pair.
static int SelectPanelWheelRecordPilot(const TimerInteractionObservedPilot* records, int index, int delta)
{
    if ((records[index].flags & 0x8000) != 0) ++index;
    const TimerInteractionObservedPilot& record = records[index];
    if ((record.flags & 0x6000) != 0 || (record.flags & 0x20000000) == 0) return index;
    if ((record.flags & 0x8000) != 0 || record.kind != 2 ||
        (record.cursorKind != 1 && record.cursorKind != 2)) return -1;
    if ((record.flags & 0x10000) != 0) delta = -delta;
    DWORD direction = delta < 0 ? 2 : 1;
    if (record.cursorKind == direction) return index;
    if (index == 0) return -1;

    bool next = records[index + 1].kind == 2 && records[index + 1].cursorKind == direction;
    bool previous = records[index - 1].kind == 2 && records[index - 1].cursorKind == direction;
    if (!previous) return next ? index + 1 : -1;
    if (!next) return index - 1;
    int distance = 1;
    while (distance < index) {
        const TimerInteractionObservedPilot& prior = records[index - distance];
        DWORD expected = (distance & 1) != 0 ? direction : record.cursorKind;
        if (prior.kind != 2 || prior.cursorKind != expected) break;
        ++distance;
    }
    return (distance & 1) != 0 ? index + 1 : index - 1;
}

// 20137248. The last eligible hit wins within a group.
static void* __stdcall VisitPanelInputHitGroupPilot(void* entry, void* argument)
{
    PanelInputHitGroupPilot* group = static_cast<PanelInputHitGroupPilot*>(entry);
    const PanelInputMessagePilot* input = static_cast<const PanelInputMessagePilot*>(argument);
    if (!PtInRect(&group->bounds, input->point)) return NULL;
    int depth = 0;
    int skippedDepth = 0;
    int selected = -1;
    DWORD inputGroup = 0;
    for (int index = 0; group->records[index].kind != 0; ++index) {
        TimerInteractionObservedPilot* record = &group->records[index];
        if (record->kind == 3) {
            if (--depth < skippedDepth) skippedDepth = 0;
            continue;
        }
        if (record->kind == 1) ++depth;
        if (skippedDepth != 0 || (record->kind != 1 && record->kind != 2)) continue;
        if (record->releaseOwner == NULL || !PtInRect(&record->releaseOwner->bounds, input->point)) {
            if (record->kind == 1) skippedDepth = depth;
            continue;
        }
        selected = index;
        if (record->inputGroup != 0 || record->releaseOwner->hoverText != NULL)
            g_inputHoverPilot = reinterpret_cast<DWORD>(record);
        if (record->inputGroup != 0) inputGroup = record->inputGroup;
    }
    if (selected < 0) return NULL;
    TimerInteractionObservedPilot* cursorRecord = input->message == WM_NCHITTEST ? NULL : &group->records[selected];
    if (input->message == PanelMouseWheelMessagePilot)
        selected = SelectPanelWheelRecordPilot(group->records, selected, static_cast<short>(HIWORD(input->wParam)));
    bool handled = selected >= 0 && DispatchCapturedPanelInputPilot(&group->records[selected], input, inputGroup);
    if (cursorRecord != NULL && g_inputCursorPilot == 0)
        g_inputCursorPilot = ReadPanelInteractionCursorPilot(cursorRecord);
    return handled ? group : NULL;
}

// 20137a2c. The first four DWORDs of the release owner also form a RECT.
static void UpdatePanelInputRepeatPilot(ClipWindowObservedPilot* window, POINT point)
{
    if (g_timerActivitySecondPilot == NULL ||
        reinterpret_cast<DWORD>(window) != g_timerSavedRepeatInputWordPilot) return;

    const RECT* bounds = reinterpret_cast<const RECT*>(g_timerActivitySecondPilot->releaseOwner);
    if (bounds != NULL && PtInRect(bounds, point)) {
        POINT relative = { point.x - bounds->left, point.y - bounds->top };
        memcpy(g_timerSecondCallbackContextPilot, &relative, sizeof(relative));
    } else {
        EndTimerRepeatSourcePilot();
    }
}

// 2013754f. The original receives hitWindow in EAX.
extern "C" int __stdcall DispatchPanelInputPilot(ClipWindowObservedPilot* window,
    const PanelInputMessagePilot* input, TimerInteractionObservedPilot* interaction,
    ClipWindowObservedPilot* hitWindow)
{
    if (window == NULL || input == NULL || window->link == NULL) return 0;
    UINT message = input->message;
    ClipWindowLinkObservedPilot* sourceLink = window->link;
    ClipWindowLinkObservedPilot* hitLink = hitWindow != NULL ? hitWindow->link : NULL;
    g_inputCursorPilot = 0;
    g_inputHoverPilot = 0;
    g_inputSuppressPostprocessingPilot = 0;
    g_timerInputWindowPilot = window;
    g_timerRepeatInputWordPilot = reinterpret_cast<DWORD>(hitWindow);

    if (g_timerActivityFirstPilot != NULL && window == g_timerCapturedInputWindowPilot) {
        interaction = g_timerActivityFirstPilot;
        g_timerCaptureInputWordPilot = g_timerSavedCaptureInputWordPilot;
    }
    bool handled = false;
    bool hitTest = message == WM_NCHITTEST;
    if (interaction != NULL) {
        DispatchCapturedPanelInputPilot(interaction, input, interaction->inputGroup);
        if (!hitTest) g_inputCursorPilot = ReadPanelInteractionCursorPilot(interaction);
        handled = true;
    } else if (hitWindow != NULL) {
        if (!hitTest) UpdatePanelInputRepeatPilot(hitWindow, input->point);
        if (hitLink != NULL)
            handled = VisitCallbackArrayPilot(reinterpret_cast<CallbackArrayObservedPilot*>(&hitLink->field2c),
                const_cast<PanelInputMessagePilot*>(input), VisitPanelInputHitGroupPilot, 1) != NULL;
        // 20137a83.
        if (!hitTest && handled && g_inputSuppressPostprocessingPilot == 0 &&
            g_timerActivitySecondPilot != NULL &&
            reinterpret_cast<DWORD>(hitWindow) != g_timerSavedRepeatInputWordPilot)
            EndTimerRepeatSourcePilot();
    } else if (!hitTest && g_timerActivitySecondPilot != NULL && g_timerSavedRepeatInputWordPilot != 0xffffffffu) {
        EndTimerRepeatSourcePilot();
    }

    g_timerInputWindowPilot = NULL;
    g_timerRepeatInputWordPilot = 0;
    bool rightClick = message == WM_RBUTTONDOWN || message == WM_NCRBUTTONDOWN;
    bool leftClick = message == WM_LBUTTONDOWN || message == WM_NCLBUTTONDOWN;
    if (!handled && (rightClick || leftClick)) {
        ClipWindowObservedPilot* target = (window->flags & 0x100) != 0 ?
            GetAlternatePanelInputWindowPilot() : window;
        if (target != NULL && target->link != NULL) {
            if (rightClick) {
                DispatchPanelContextClickPilot(target->link->geometryLink, input, 0, 0);
                handled = true;
            } else if (input->wParam == 1 || input->wParam == 2) {
                if ((target->flags & 0x40000000) != 0)
                    SetActiveWindow(target->link->auxiliaryWindow);
                else if (g_mainWindowEventPilot != NULL)
                    g_mainWindowEventPilot(0x20007, reinterpret_cast<DWORD>(target), 0);
            }
        }
    }
    if (!hitTest && g_inputSuppressPostprocessingPilot == 0) {
        if (sourceLink->inputCursor != g_inputCursorPilot) {
            sourceLink->inputCursor = g_inputCursorPilot;
            PostMessageA(sourceLink->window, WM_SETCURSOR, reinterpret_cast<WPARAM>(sourceLink->window), 1);
        }
        if (interaction == NULL) UpdatePanelHoverPilot(hitWindow, g_inputHoverPilot);
    }
    g_timerCaptureInputWordPilot = 0;
    return handled;
}
