extern "C" int __cdecl QueryPanelMenuMode34Pilot(); // window.dll ordinal 34
extern "C" int __cdecl QueryPanelMenuMode35Pilot(); // window.dll ordinal 35
extern "C" int __cdecl QueryPanelMenuMode36Pilot(); // window.dll ordinal 36

DWORD g_panelMenuOptionEnabledPilot = 1; // 201f70b4
static char g_panelMenuTextPilot[MAX_PATH]; // 2020d770

static DWORD AllocatePanelMenuHandlePilot(ClipWindowObservedPilot* window)
{
    void* menu = FindPanelMenuInterfacePilot(window);
    if (menu == NULL) return 0;
    typedef HRESULT (__stdcall *AllocateMenuPilot)(void*, ClipWindowObservedPilot*, DWORD*);
    DWORD handle = 0;
    return SUCCEEDED(GraphicsClipSlotPilot<AllocateMenuPilot>(menu, 0x6c)(menu, window, &handle)) ? handle : 0;
}

// 2013ad62.
extern "C" void __stdcall AppendPanelMenuItemPilot(
    ClipWindowObservedPilot* window, DWORD handle, DWORD flags, DWORD item, const char* text)
{
    if (handle == 0) return;
    void* menu = FindPanelMenuInterfacePilot(window);
    if (menu == NULL) return;
    typedef void (__stdcall *AppendMenuPilot)(void*, DWORD, DWORD, DWORD, const char*);
    GraphicsClipSlotPilot<AppendMenuPilot>(menu, 0x88)(menu, handle, flags, item, text);
}

static void SetPanelMenuItemStatePilot(PanelUpdateContextObservedPilot* context,
    unsigned int slot, DWORD handle, DWORD item, DWORD state)
{
    void* menu = FindPanelMenuInterfacePilot(context->window);
    if (menu == NULL) return;
    typedef void (__stdcall *SetMenuStatePilot)(void*, DWORD, DWORD, DWORD);
    GraphicsClipSlotPilot<SetMenuStatePilot>(menu, slot)(menu, handle, item, state);
}

static void AppendPanelMenuLabelPilot(PanelUpdateContextObservedPilot* context,
    DWORD handle, DWORD flags, DWORD item, UINT resource, char* text, int capacity)
{
    LoadSplashStringPilot(resource, text, capacity);
    AppendPanelMenuItemPilot(context->window, handle, flags, item, text);
}

// 20141195. ESI is the context, with the menu handle on the stack.
static void RefreshPanelMenuStatePilot(PanelUpdateContextObservedPilot* context, DWORD handle)
{
    SetPanelMenuItemStatePilot(context, 0x78, handle, 0x201, 0);
    SetPanelMenuItemStatePilot(context, 0x78, handle, 0x300, 0);
    if (context->window->panelKind == 3) {
        char text[MAX_PATH];
        void* view = *reinterpret_cast<void**>(reinterpret_cast<BYTE*>(context->window) + 0xd0);
        GraphicsClipSlotPilot<GraphicsClipWordPilot>(view, 0x84)(view, handle);
        AppendPanelMenuItemPilot(context->window, handle, MF_SEPARATOR, 0, NULL);
        if (g_idleControllerPilot != NULL && !g_idleControllerPilot->QueryField38())
            AppendPanelMenuLabelPilot(context, handle, 0, 0x201, 0x157b, text, 0x100);
        AppendPanelMenuLabelPilot(context, handle, 0, 0x202, 0x157c, text, 0x100);
        AppendPanelMenuItemPilot(context->window, handle, MF_SEPARATOR, 0, NULL);
        if (g_countedChildPanelWindowsPilot < 2) SetPanelMenuItemStatePilot(context, 0x7c, handle, 0x202, 1);
        LoadSplashStringPilot(0x1509, text, 0x100);
        int option = QueryPanelMenuMode36Pilot();
        AppendPanelMenuItemPilot(context->window, handle, option != 0 ? MF_CHECKED : 0, 0x1509, text);

        DWORD submenu = AllocatePanelMenuHandlePilot(context->window);
        int mode = QueryPanelMenuMode35Pilot();
        for (int i = 0; i < 4; ++i) {
            UINT item = 0x1558 + i;
            AppendPanelMenuLabelPilot(context, submenu, mode == i ? MF_CHECKED : 0, item, item, text, 0x100);
            if (i == 0) AppendPanelMenuItemPilot(context->window, submenu, MF_SEPARATOR, 0, NULL);
        }
        AppendPanelMenuLabelPilot(context, handle, MF_POPUP, submenu, 0x150a, text, 0x100);

        submenu = AllocatePanelMenuHandlePilot(context->window);
        mode = QueryPanelMenuMode34Pilot();
        const int displayModes[] = {1, 0, 2};
        for (int i = 0; i < 3; ++i) {
            UINT item = 0x150c + i;
            AppendPanelMenuLabelPilot(context, submenu, mode == displayModes[i] ? MF_CHECKED : 0, item, item, text, 0x100);
        }
        AppendPanelMenuLabelPilot(context, handle, MF_POPUP, submenu, 0x150b, text, 0x100);
        AppendPanelMenuItemPilot(context->window, handle, MF_SEPARATOR, 0, NULL);
        AppendPanelMenuLabelPilot(context, handle, 0, 0x300, 0x157d, text, 0x100);
        AppendPanelMenuItemPilot(context->window, handle, MF_SEPARATOR, 0, NULL);
        int controls = QueryControlsCursorOverridePilot();
        AppendPanelMenuLabelPilot(context, handle, controls != 0 ? MF_CHECKED : 0, 0x400, 0x157e, text, 0x100);
    }
    if ((context->window->flags & 0x40000000) != 0)
        SetPanelMenuItemStatePilot(context, 0x78, handle, 0x201, MF_CHECKED);
    if (g_panelMenuOptionEnabledPilot == 0)
        SetPanelMenuItemStatePilot(context, 0x78, handle, 0x300, MF_CHECKED);
    DWORD state = g_idleControllerPilot->QueryField38() ? 1 : 0;
    SetPanelMenuItemStatePilot(context, 0x7c, handle, 0x201, state);
}

extern "C" LRESULT __stdcall PanelChildMenuSourcePilot(
    PanelUpdateContextObservedPilot* context, UINT, WPARAM wParam, LPARAM)
{
    RefreshPanelMenuStatePilot(context, static_cast<DWORD>(wParam));
    return 0;
}

// 2012b637. The original context arrives in EAX.
extern "C" void __stdcall BuildPanelMenuSourcePilot(PanelUpdateContextObservedPilot* context)
{
    ReleasePanelContextGraphicsSourcePilot(context->window, context->contextField44);
    context->contextField44 = 0;
    DWORD handle = AllocatePanelMenuHandlePilot(context->window);
    context->contextField44 = handle;
    if (handle == 0) return;
    if (context->window->panelKind != 3) {
        bool hasFirstItem = g_idleControllerPilot != NULL && !g_idleControllerPilot->QueryField38();
        if (hasFirstItem) {
            LoadSplashStringPilot(0x157b, g_panelMenuTextPilot, sizeof(g_panelMenuTextPilot));
            AppendPanelMenuItemPilot(context->window, context->contextField44, 0, 0x201, g_panelMenuTextPilot);
        }
        bool hasSecondItem = (context->window->flags & 0x200000) == 0;
        if (hasSecondItem) {
            LoadSplashStringPilot(0x157c, g_panelMenuTextPilot, sizeof(g_panelMenuTextPilot));
            AppendPanelMenuItemPilot(context->window, context->contextField44, 0, 0x202, g_panelMenuTextPilot);
        }
        if (hasFirstItem || hasSecondItem)
            AppendPanelMenuItemPilot(context->window, context->contextField44, MF_SEPARATOR, 0, NULL);
        LoadSplashStringPilot(0x157d, g_panelMenuTextPilot, sizeof(g_panelMenuTextPilot));
        AppendPanelMenuItemPilot(context->window, context->contextField44, 0, 0x300, g_panelMenuTextPilot);
    }
    RefreshPanelMenuStatePilot(context, context->contextField44);
}

DWORD g_panelMenuOptionNormalPilot; // 2020e01c
DWORD g_panelMenuOptionAlternatePilot = 1; // 201f716c
DWORD g_panelMenuOptionOverridePilot; // 2020e020

// 20141853. Original command in EAX, context in EDX and LPARAM in ESI.
extern "C" LRESULT __stdcall PanelChildCommandSourcePilot(
    PanelUpdateContextObservedPilot* context, UINT, WPARAM wParam, LPARAM lParam)
{
    DWORD command = static_cast<DWORD>(wParam);
    if (command >= 0x7801 && command <= 0x7eff) {
        typedef void (__stdcall *MenuCommandPilot)(DWORD);
        if (context->contextField44 != 0 && lParam != 0)
            reinterpret_cast<MenuCommandPilot>(lParam)(command);
        return 0;
    }
    switch (command) {
    case 0x201:
    case 0x202:
        if (g_mainWindowEventPilot != NULL)
            g_mainWindowEventPilot(command == 0x201 ? 0x20009 : 0x2000a,
                reinterpret_cast<DWORD>(context->window), 0);
        return 0;
    case 0x300:
        if (g_idleControllerPilot->QueryField38()) {
            g_panelMenuOptionAlternatePilot = g_panelMenuOptionAlternatePilot == 0;
            g_panelMenuOptionEnabledPilot =
                g_panelMenuOptionAlternatePilot == 0 && g_panelMenuOptionOverridePilot == 0;
        } else {
            g_panelMenuOptionNormalPilot = g_panelMenuOptionNormalPilot == 0;
            g_panelMenuOptionEnabledPilot = g_panelMenuOptionNormalPilot == 0;
        }
        return 0;
    case 0x500:
        DispatchCallbackListSourcePilot(&g_mainWindowCommandCallbacksPilot, 400, 0);
        return 0;
    case 0x400: case 0x1509: case 0x150c: case 0x150d: case 0x150e:
    case 0x1558: case 0x1559: case 0x155a: case 0x155b:
        if (g_mainWindowEventPilot != NULL) g_mainWindowEventPilot(command, 0, 0);
        return 0;
    default:
        return DefaultPanelMessagePilot(context, WM_COMMAND, wParam, lParam);
    }
}

DWORD g_panelContextClickEnabledPilot = 1; // 201f7170
DWORD g_panelContextClickPhasePilot = 1; // 201f9550
DWORD g_panelPendingPointerStatePilot; // 2020d128
LONG g_panelPendingPointerXPilot, g_panelPendingPointerYPilot; // 2020d12c, 2020d130

extern "C" void __stdcall DispatchSpecialPanelContextClickPilot(DWORD, DWORD); // 2012cfc5
extern "C" void* __stdcall CreateAuxiliaryPanelMenuHostSourcePilot(HWND); // 2013a534, HWND in EDI
extern "C" void* __stdcall CreateRootPanelMenuHostSourcePilot(HWND); // 2013a5be, HWND in EDI
extern "C" void __stdcall PrepareWindowPanelMenuSourcePilot(DWORD); // window.dll ordinal 4

static void PositionPanelMenuHostPilot(PanelUpdateContextObservedPilot* context)
{
    ClipWindowObservedPilot* window = context->window;
    void* menu = NULL;
    if ((window->flags & 0x40000000) != 0) {
        menu = FindPanelMenuInterfacePilot(window);
        if (menu == NULL && window->link->auxiliaryWindow != NULL)
            menu = CreateAuxiliaryPanelMenuHostSourcePilot(window->link->auxiliaryWindow);
    } else if (g_idleControllerPilot->QueryField38()) {
        HWND hwnd = window->link != NULL ? window->link->window : NULL;
        HWND root = GetAncestor(hwnd, GA_ROOT);
        if (ReadMainWindowSizingStatePilot(g_mainWindowControllerPilot).window != root) {
            menu = FindPanelMenuInterfacePilot(context->window);
            if (menu == NULL) menu = CreateRootPanelMenuHostSourcePilot(root);
        }
    }
    if (menu != NULL) {
        typedef HWND (__stdcall *ReadMenuWindowPilot)(void*);
        HWND hwnd = GraphicsClipSlotPilot<ReadMenuWindowPilot>(menu, 0x44)(menu);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    }
}

// 2013ad98 with the fixed arguments of the context-click caller.
static void ShowPanelContextMenuPilot(PanelUpdateContextObservedPilot* context, const POINT& point)
{
    DWORD handle = context->contextField44;
    if (handle == 0) return;
    ClipWindowObservedPilot* window = context->window;
    HWND hwnd = context->windowHandle;
    void* menu = FindPanelMenuInterfacePilot(window);
    if (menu == NULL) return;
    typedef void (__stdcall *ShowMenuPilot)(void*, DWORD, ClipWindowObservedPilot*, DWORD,
        LONG, LONG, DWORD, HWND, DWORD);
    GraphicsClipSlotPilot<ShowMenuPilot>(menu, 0x8c)(menu, handle, window, 0, point.x, point.y, 0, hwnd, 0);
}

// 2012d08c. EAX carries the group and ECX the mode.
extern "C" void __stdcall DispatchPanelContextClickPilot(
    ClipGeometryLinkObservedPilot* link, const PanelInputMessagePilot* input, DWORD group, DWORD mode)
{
    if (input == NULL || g_panelContextClickEnabledPilot == 0) return;
    PanelUpdateContextObservedPilot* context = reinterpret_cast<PanelUpdateContextObservedPilot*>(link);
    DWORD flags = context->window->flags;
    if ((flags & 0x2000) != 0) {
        DispatchSpecialPanelContextClickPilot(mode, group);
        return;
    }
    if ((flags & 0x20000) != 0) return;
    UINT message = input->message;
    bool screenCoordinates = message == WM_NCHITTEST || (message >= 0xa0 && message <= 0xa9) ||
        message == PanelMouseWheelMessagePilot;
    if (!screenCoordinates && (message < 0x200 || message > 0x209)) return;
    POINT point = { static_cast<short>(LOWORD(input->lParam)), static_cast<short>(HIWORD(input->lParam)) };
    if (screenCoordinates) ScreenToClient(input->window, &point);
    if (group != 0) {
        g_panelPendingPointerStatePilot = group;
        g_panelPendingPointerXPilot = point.x;
        g_panelPendingPointerYPilot = point.y;
    }
    if (g_mainWindowEventPilot != NULL) g_mainWindowEventPilot(0x20017, 0, 0);
    if (g_panelContextClickPhasePilot != 2) g_panelContextClickPhasePilot = 1;
    if (g_mainWindowEventPilot != NULL) g_mainWindowEventPilot(0x20002, 0, 0);
    if (g_panelContextClickPhasePilot != 2) g_panelContextClickPhasePilot = 0;
    if (g_applicationReadyPilot != 0 && g_idleControllerPilot->QueryField38())
        PrepareWindowPanelMenuSourcePilot(1);
    PositionPanelMenuHostPilot(context);
    BuildPanelMenuSourcePilot(context);
    ShowPanelContextMenuPilot(context, point);
    if (g_mainWindowEventPilot != NULL) {
        g_mainWindowEventPilot(0x20003, 0, 0);
        if (g_mainWindowEventPilot != NULL) g_mainWindowEventPilot(0x20018, 0, 0);
    }
}
