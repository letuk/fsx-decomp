// API-side menu host lookup. These names are reconstructed.
void* g_defaultPanelMenuHostPilot; // 2020e090, populated by menu-host creation
static ATOM g_panelMenuHostAtomPilot; // 2020e094
static const GUID g_panelMenuInterfaceIdPilot = {
    0x4875a937, 0xdebd, 0x43d4, {0xbf, 0x61, 0x37, 0xe3, 0x4b, 0x59, 0x51, 0xd8}
}; // 200f3614

static const GUID g_panelMenuHostClassIdPilot = {
    0x8be5831f, 0xa49b, 0x4726, {0x99, 0x24, 0xe6, 0xef, 0x6e, 0x2e, 0x7a, 0xfe}
}; // 200f3604

static LPCSTR PanelMenuHostPropertyPilot()
{
    if (g_panelMenuHostAtomPilot == 0) g_panelMenuHostAtomPilot = GlobalAddAtomA("MenuHostWindow");
    return MAKEINTATOM(g_panelMenuHostAtomPilot);
}

// 20138ee1. Original HWND in EAX and requested interface ID in EDI.
extern "C" void* __stdcall QueryPanelMenuHostSourcePilot(HWND hwnd, const GUID* interfaceId)
{
    void* host = g_defaultPanelMenuHostPilot;
    if (hwnd != NULL) {
        HWND root = GetAncestor(hwnd, GA_ROOT);
        host = GetPropA(root, PanelMenuHostPropertyPilot());
    }
    if (host == NULL) return NULL;
    typedef HRESULT (__stdcall *QueryInterfacePilot)(void*, const GUID*, void**);
    void* result;
    HRESULT status = GraphicsClipSlotPilot<QueryInterfacePilot>(host, 0)(host, interfaceId, &result);
    return SUCCEEDED(status) ? result : NULL;
}

// 20138f47. The original panel pointer arrives in EAX.
extern "C" void* __stdcall FindPanelMenuInterfacePilot(ClipWindowObservedPilot* window)
{
    if (window == NULL) return QueryPanelMenuHostSourcePilot(NULL, &g_panelMenuInterfaceIdPilot);
    if (window->link == NULL || window->link->window == NULL) return NULL;
    return QueryPanelMenuHostSourcePilot(window->link->window, &g_panelMenuInterfaceIdPilot);
}

// 20139211. Keep an existing root-window property.
extern "C" HANDLE __stdcall AttachPanelMenuHostSourcePilot(HWND hwnd, HANDLE host)
{
    HWND root = GetAncestor(hwnd, GA_ROOT);
    HANDLE current = GetPropA(root, PanelMenuHostPropertyPilot());
    if (current == NULL && IsWindow(root)) {
        SetPropA(root, PanelMenuHostPropertyPilot(), host);
        return host;
    }
    return current;
}

// 2013928e. Removal returns the value observed before RemovePropA.
extern "C" HANDLE __stdcall DetachPanelMenuHostSourcePilot(HWND hwnd)
{
    HWND root = GetAncestor(hwnd, GA_ROOT);
    HANDLE host = GetPropA(root, PanelMenuHostPropertyPilot());
    if (host != NULL) RemovePropA(root, PanelMenuHostPropertyPilot());
    return host;
}

// API caller sequence in 2013f296 and 201401d3.
extern "C" void __stdcall ReleasePanelContextGraphicsSourcePilot(ClipWindowObservedPilot* window, DWORD handle)
{
    if (handle == 0) return;
    void* menu = FindPanelMenuInterfacePilot(window);
    if (menu == NULL) return;
    typedef void (__stdcall *ReleaseMenuHandlePilot)(void*, DWORD);
    GraphicsClipSlotPilot<ReleaseMenuHandlePilot>(menu, 0x70)(menu, handle);
}

typedef HRESULT (__stdcall *PanelMenuHostFactoryPilot)(const GUID*, void**);
struct PanelMenuHostCreateObservedPilot {
    DWORD fields;
    PanelMenuHostFactoryPilot factory;
    const GUID* classId;
    DWORD reserved[4];
    HWND parent;
};
C_ASSERT(sizeof(PanelMenuHostCreateObservedPilot) == 0x20);
C_ASSERT(offsetof(PanelMenuHostCreateObservedPilot, parent) == 0x1c);

struct PanelMenuHostInstanceObservedPilot {
    void** table;
    void* base;
    DWORD field08;
    DWORD field0c;
    DWORD field10;
};
C_ASSERT(sizeof(PanelMenuHostInstanceObservedPilot) == 0x14);

extern void* g_defaultPanelMenuHostTablePilot[]; // 200f3370
extern void* g_windowPanelMenuHostTablePilot[]; // 200f3410
extern "C" HRESULT __stdcall CreateSimXuiPanelHostSourcePilot(const PanelMenuHostCreateObservedPilot*, void**);
extern "C" HRESULT __stdcall CreateSimXuiPanelHostBaseSourcePilot(const GUID*, void**);

static HRESULT AllocatePanelMenuHostInstancePilot(const GUID* classId, void** output, void** table)
{
    void* memory;
    AllocatePrivateHeapMemoryPilot(1, sizeof(PanelMenuHostInstanceObservedPilot),
        &memory, g_factoryZeroInitializeBoundaryPilot);
    PanelMenuHostInstanceObservedPilot* host = static_cast<PanelMenuHostInstanceObservedPilot*>(memory);
    if (host != NULL) {
        host->base = NULL;
        host->field08 = 0;
        host->field0c = 1;
        host->field10 = 0;
        host->table = table;
    }
    *output = host;
    if (host == NULL) return E_OUTOFMEMORY;
    HRESULT status = CreateSimXuiPanelHostBaseSourcePilot(classId, &host->base);
    if (FAILED(status))
        GraphicsClipSlotPilot<GraphicsClipWordPilot>(host, 0x98)(host, 1);
    return status;
}

// 2013aeab and 2013af2f use distinct tables.
static HRESULT __stdcall CreateDefaultPanelMenuHostInstancePilot(const GUID* classId, void** output)
{
    return AllocatePanelMenuHostInstancePilot(classId, output, g_defaultPanelMenuHostTablePilot);
}

static HRESULT __stdcall CreateWindowPanelMenuHostInstancePilot(const GUID* classId, void** output)
{
    return AllocatePanelMenuHostInstancePilot(classId, output, g_windowPanelMenuHostTablePilot);
}

static void* QueryCreatedPanelMenuHostPilot(void* host, const GUID& id)
{
    typedef HRESULT (__stdcall *QueryInterfacePilot)(void*, const GUID*, void**);
    void* result;
    if (SUCCEEDED(GraphicsClipSlotPilot<QueryInterfacePilot>(host, 0)(host, &id, &result)))
        return result;
    typedef void (__stdcall *HostPairPilot)(void*, void*);
    typedef void (__stdcall *HostMethodPilot)(void*);
    GraphicsClipSlotPilot<HostPairPilot>(host, 0x58)(host, host);
    GraphicsClipSlotPilot<HostMethodPilot>(host, 0x28)(host);
    return NULL;
}

// 2013a20b. The owner remains responsible for clearing the published host.
extern "C" void* __cdecl GetOrCreateDefaultPanelMenuHostSourcePilot()
{
    void* host = g_defaultPanelMenuHostPilot;
    if (host == NULL) {
        PanelMenuHostCreateObservedPilot request = {
            0x3b, CreateDefaultPanelMenuHostInstancePilot, &g_panelMenuHostClassIdPilot, {0}, NULL
        };
        CreateSimXuiPanelHostSourcePilot(&request, &host);
        g_defaultPanelMenuHostPilot = host;
        typedef HWND (__stdcall *ReadHostWindowPilot)(void*);
        HWND hwnd = GraphicsClipSlotPilot<ReadHostWindowPilot>(host, 0x44)(host);
        AttachPanelMenuHostSourcePilot(hwnd, host);
    }
    return QueryCreatedPanelMenuHostPilot(host, g_panelMenuHostClassIdPilot);
}

// 2013a5be. The original parent arrives in EDI.
extern "C" void* __stdcall CreateRootPanelMenuHostSourcePilot(HWND parent)
{
    PanelMenuHostCreateObservedPilot request = {
        0x3b, CreateWindowPanelMenuHostInstancePilot, &g_panelMenuHostClassIdPilot, {0}, parent
    };
    void* host;
    CreateSimXuiPanelHostSourcePilot(&request, &host);
    AttachPanelMenuHostSourcePilot(parent, host);
    return QueryCreatedPanelMenuHostPilot(host, g_panelMenuInterfaceIdPilot);
}

// 2013a534 adds a graphics controller notification after the same creation flow.
extern "C" void* __stdcall CreateAuxiliaryPanelMenuHostSourcePilot(HWND parent)
{
    void* result = CreateRootPanelMenuHostSourcePilot(parent);
    void* graphics = GetGraphicsControllerSourcePilot();
    GraphicsClipSlotPilot<GraphicsClipNotifyPilot>(graphics, 0x38)(graphics);
    return result;
}

extern "C" void __stdcall DestroyPanelMenuXuiObjectSourcePilot(DWORD); // 20149253
extern "C" void __stdcall UnregisterPanelMenuXuiClassSourcePilot(const wchar_t*); // 2014927d
DWORD g_defaultPanelMenuRegistrationPilot; // 2020e09c

// 20138f9b, slot zero in both tables.
extern "C" HRESULT __stdcall QueryPanelMenuHostInstancePilot(
    PanelMenuHostInstanceObservedPilot* host, const GUID* id, void** output)
{
    *output = NULL;
    if (memcmp(id, &g_panelMenuHostClassIdPilot, sizeof(*id)) != 0 &&
        memcmp(id, &g_panelMenuInterfaceIdPilot, sizeof(*id)) != 0)
        return E_NOINTERFACE;
    *output = host;
    return S_OK;
}

// 20138c2c, slot +0x44 in both tables.
extern "C" HWND __stdcall ReadPanelMenuHostWindowPilot(PanelMenuHostInstanceObservedPilot* host)
{
    typedef HWND (__stdcall *ReadWindowPilot)(void*);
    void* base = host->base;
    return GraphicsClipSlotPilot<ReadWindowPilot>(base, 0x44)(base);
}

static void DetachPanelMenuHostInstancePilot(PanelMenuHostInstanceObservedPilot* host, DWORD reason)
{
    if (host->field08 != 0) {
        DestroyPanelMenuXuiObjectSourcePilot(host->field08);
        host->field08 = 0;
    }
    DetachPanelMenuHostSourcePilot(ReadPanelMenuHostWindowPilot(host));
    void* base = host->base;
    typedef void (__stdcall *DetachHostPilot)(void*, DWORD);
    GraphicsClipSlotPilot<DetachHostPilot>(base, 0x58)(base, reason);
}

// 201395e9 and 2013a1ac, distinct slot +0x58 targets.
extern "C" void __stdcall DetachWindowPanelMenuHostInstancePilot(
    PanelMenuHostInstanceObservedPilot* host, DWORD reason)
{
    DetachPanelMenuHostInstancePilot(host, reason);
}

extern "C" void __stdcall DetachDefaultPanelMenuHostInstancePilot(
    PanelMenuHostInstanceObservedPilot* host, DWORD reason)
{
    DetachPanelMenuHostInstancePilot(host, reason);
    g_defaultPanelMenuHostPilot = NULL;
    UnregisterPanelMenuXuiClassSourcePilot(L"FSXMenuBar");
    g_defaultPanelMenuRegistrationPilot = 0;
}

// 2013afb3. Original receiver in ECX, flags on the stack.
extern "C" PanelMenuHostInstanceObservedPilot* __stdcall DeletePanelMenuHostInstancePilot(
    PanelMenuHostInstanceObservedPilot* host, DWORD flags)
{
    host->table = g_windowPanelMenuHostTablePilot;
    if ((flags & 1) != 0) HeapFree(g_privateHeapPilot, 0, host);
    return host;
}

// 20138b76, slot +0x28 in both tables.
extern "C" void __stdcall ReleasePanelMenuHostInstancePilot(PanelMenuHostInstanceObservedPilot* host)
{
    void* base = host->base;
    host->base = NULL;
    GraphicsClipSlotPilot<GraphicsClipWordPilot>(host, 0x98)(host, 1);
    if (base != NULL) {
        typedef void (__stdcall *ReleaseBasePilot)(void*);
        GraphicsClipSlotPilot<ReleaseBasePilot>(base, 0x28)(base);
    }
}

typedef DWORD (__stdcall *PanelHostFocusCallbackPilot)(DWORD, DWORD, DWORD, DWORD);
typedef void (__stdcall *PanelHostChainCallbackPilot)(UINT, void*, void*, void*);
extern "C" void __stdcall SimXuiPanelHostChainMessageSourcePilot(UINT, void*, void*, void*);
extern "C" void __stdcall RegisterPanelHostChainSourcePilot(
    void**, PanelHostChainCallbackPilot, DWORD, void*, DWORD); // 200f6451
extern "C" void __stdcall RegisterPanelHostFocusSourcePilot(PanelHostFocusCallbackPilot);
extern "C" int __stdcall IsPanelHostElementDescendantSourcePilot(DWORD, DWORD);
extern "C" HRESULT __stdcall CreatePanelHostCanvasSourcePilot(DWORD*);
extern "C" void __stdcall SetPanelHostCanvasWindowSourcePilot(DWORD, HWND);
extern "C" void __stdcall SetPanelHostCanvasBoundsSourcePilot(DWORD, float, float);
extern "C" void __stdcall ReleasePanelHostWindowRecordSourcePilot(ClipWindowObservedPilot**); // window.dll ordinal 14
extern "C" LONG __stdcall CreatePanelHostWindowRecordSourcePilot(ClipWindowObservedPilot*, HWND); // 2011cc66
extern "C" int __stdcall ScalePanelHostDimensionsSourcePilot(ClipWindowObservedPilot*, const SIZE*, SIZE*); // 2011d4cd
extern "C" void __stdcall FinalizePanelHostWindowRecordSourcePilot(ClipWindowObservedPilot*); // 2011d262

// 201392f6. The callback pops four words and uses the first and third.
extern "C" DWORD __stdcall ReconcilePanelMenuHostFocusPilot(
    DWORD previousElement, DWORD, DWORD nextElement, DWORD)
{
    ClipWindowObservedPilot* window = NULL;
    DWORD rootElement = 0;
    void* host = g_defaultPanelMenuHostPilot;
    if (host != NULL) {
        typedef ClipWindowObservedPilot* (__stdcall *ReadWindowRecordPilot)(void*);
        window = GraphicsClipSlotPilot<ReadWindowRecordPilot>(host, 0x40)(host);
        host = g_defaultPanelMenuHostPilot;
        if (host != NULL) {
            typedef DWORD (__stdcall *ReadElementPilot)(void*);
            rootElement = GraphicsClipSlotPilot<ReadElementPilot>(host, 0x3c)(host);
        }
    }
    DWORD element = nextElement != 0 ? nextElement : previousElement;
    if (window == NULL || element == 0 ||
        IsPanelHostElementDescendantSourcePilot(rootElement, element) == 0)
        return 0;
    HWND hwnd = window->link != NULL ? window->link->window : NULL;
    if (nextElement != 0) {
        if (hwnd != NULL) SetFocus(hwnd);
    } else if (GetFocus() == hwnd) {
        HWND parent = GetParent(hwnd);
        if (parent != NULL) {
            SetFocus(NULL);
            SetFocus(parent);
        }
    }
    return 0;
}

typedef void* (__stdcall *PreparePanelHostRecordPilot)(
    void*, void*, const PanelMenuHostCreateObservedPilot*, ClipWindowObservedPilot**);

// 201391d1. The original leaves the base call's EAX intact.
extern "C" void* __stdcall PreparePanelMenuHostRecordPilotTarget(
    PanelMenuHostInstanceObservedPilot* host, void* instance,
    const PanelMenuHostCreateObservedPilot* request, ClipWindowObservedPilot** output)
{
    void* base = host->base;
    ClipWindowObservedPilot* window = reinterpret_cast<ClipWindowObservedPilot*>(host);
    void* result = GraphicsClipSlotPilot<PreparePanelHostRecordPilot>(base, 8)(base, instance, request, &window);
    if (output != NULL) *output = window;
    window->flags = 0x830e4000;
    window->groupKey = 0x6000;
    return result;
}

// 20138fee, slot +4 in both host tables.
extern "C" void* __stdcall InitializePanelMenuHostPilot(
    PanelMenuHostInstanceObservedPilot* host, void* instance, const PanelMenuHostCreateObservedPilot* request)
{
    ClipWindowObservedPilot* window;
    void* result = GraphicsClipSlotPilot<PreparePanelHostRecordPilot>(host, 8)(host, instance, request, &window);
    if (result != NULL) return result;

    HWND parent = g_mainWindowHandlePilot; // 2020e010
    if ((request->fields & 0x20) != 0 && IsWindow(request->parent)) {
        DWORD& flags = *reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(window) + 0xc);
        flags |= GetPropA(request->parent, PanelWindowPropertyPilot()) != NULL ? 1 : 2;
        if ((flags & 3) != 0) parent = request->parent;
    }
    LONG status = CreatePanelHostWindowRecordSourcePilot(window, parent);
    if (status != 0) {
        ReleasePanelHostWindowRecordSourcePilot(&window);
        return reinterpret_cast<void*>(status);
    }

    HWND hwnd = window != NULL && window->link != NULL ? window->link->window : NULL;
    RECT bounds = {0, 0, 0, 0};
    GetClientRect(GetParent(hwnd), &bounds);
    LONG width = bounds.right - bounds.left;
    LONG height = bounds.bottom - bounds.top;
    const SIZE dimensions = {width, height};
    SIZE scaled = {0, 0};
    ScalePanelHostDimensionsSourcePilot(window, &dimensions, &scaled);
    BYTE* record = reinterpret_cast<BYTE*>(window);
    *reinterpret_cast<SIZE*>(record + 0x28) = scaled;
    const RECT client = {0, 0, width, height};
    *reinterpret_cast<RECT*>(record + 0xac) = client;
    *reinterpret_cast<RECT*>(record + 0xbc) = client;

    typedef void (__stdcall *AttachHostWindowPilot)(void*, HWND, void*);
    GraphicsClipSlotPilot<AttachHostWindowPilot>(instance, 0xc)(instance, hwnd, instance);
    FinalizePanelHostWindowRecordSourcePilot(window);
    RegisterPanelHostChainSourcePilot(&g_mainWindowCommandCallbacksPilot,
        SimXuiPanelHostChainMessageSourcePilot, 1, instance, 0);
    RegisterPanelHostFocusSourcePilot(ReconcilePanelMenuHostFocusPilot);
    if (SUCCEEDED(CreatePanelHostCanvasSourcePilot(&host->field08))) {
        SetPanelHostCanvasWindowSourcePilot(host->field08, hwnd);
        SetPanelHostCanvasBoundsSourcePilot(host->field08, static_cast<float>(width), static_cast<float>(height));
    }
    return NULL;
}
