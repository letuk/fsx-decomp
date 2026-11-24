extern "C" HMODULE g_languageModulePilot; // 2020e028
extern "C" char g_savedWindowLocationPilot[]; // shared scratch 2020d770

static CallbackArrayObservedPilot g_resourceModulesPilot = {
    0, 0, 32, sizeof(HMODULE), 0, 0, NULL
}; // 201f70b8

// 201384ea / export 152. Duplicate module handles are allowed.
extern "C" int __stdcall RegisterResourceModulePilot(HMODULE module, HMODULE* token)
{
    HMODULE* record = static_cast<HMODULE*>(AppendCallbackArrayRecordPilot(&g_resourceModulesPilot));
    *record = module;
    *token = module;
    return 0;
}

// 20138509 / export 153. Uses the direct DWORD specialization of 2013bfe4.
extern "C" int __stdcall UnregisterResourceModulePilot(HMODULE token)
{
    for (int i = 0; i < g_resourceModulesPilot.count; ++i) {
        BYTE* record = CallbackArrayRecordPilot(&g_resourceModulesPilot, i);
        if (*reinterpret_cast<HMODULE*>(record) == token) {
            RemoveCallbackArrayRecordPilot(&g_resourceModulesPilot, record);
            return 0;
        }
    }
    return -1;
}

struct ResourceStringRequestPilot {
    UINT resource;
    char* output;
    int capacity;
    int hits;
};

// 201386ea. The iterator uses the returned length to stop at the first hit.
static void* __stdcall VisitResourceStringPilot(void* record, void* context)
{
    ResourceStringRequestPilot* request = static_cast<ResourceStringRequestPilot*>(context);
    int length = LoadStringA(*static_cast<HMODULE*>(record), request->resource, request->output, request->capacity);
    if (length != 0) ++request->hits;
    return reinterpret_cast<void*>(length);
}

// 20138550 / export 154. The original leaves the length in EAX.
extern "C" int __stdcall LoadSplashStringPilot(unsigned int resource, char* output, int capacity)
{
    ResourceStringRequestPilot request = { resource, output, capacity, 0 };
    int length = reinterpret_cast<int>(VisitCallbackArrayPilot(&g_resourceModulesPilot, &request,
        VisitResourceStringPilot, 1));
    if (length == 0) return LoadStringA(g_languageModulePilot, resource, output, capacity);
    return length;
}

// 201385a8 / export 155. A missing string leaves a null output pointer.
extern "C" int __stdcall AllocateResourceStringPilot(UINT resource, char** output)
{
    char text[1024];
    *output = NULL;
    int length = LoadSplashStringPilot(resource, text, sizeof(text));
    if (length == 0) return 0;

    SIZE_T bytes = length + 1;
    if (g_heapInitializedPilot == 0) InitializeHeapManagerPilot();
    *output = static_cast<char*>(HeapAlloc(g_privateHeapPilot, 0, bytes));
    if (*output == NULL) {
        *output = static_cast<char*>(HeapAlloc(g_privateHeapPilot, 0, bytes));
        if (*output == NULL) {
            HMODULE module = g_languageModulePilot;
            if (module == NULL) module = GetModuleHandleA(NULL);
            LoadStringA(module, 8, g_savedWindowLocationPilot, MAX_PATH);
            MessageBoxA(NULL, g_savedWindowLocationPilot, NULL, MB_ICONHAND);
            TerminateProcess(GetCurrentProcess(), 1);
        }
    }
    CopyResolvedPathPilot(*output, text, bytes);
    return length;
}
