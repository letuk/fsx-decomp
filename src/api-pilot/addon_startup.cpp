#include <vector>
#include <mbstring.h>
#include <shlwapi.h>

extern "C" DWORD g_addonTrustResetSelector201f9130Pilot;
extern "C" DWORD g_skipAddonStartup201f7d80Pilot;
extern "C" DWORD g_addonProgress201f8e90Pilot;
extern "C" DWORD g_addonPrompt201f8fe0Pilot;
extern "C" int __stdcall CheckAddonTrustSourcePilot(int, int, const char*, DWORD*); // 20123c41
extern "C" void __stdcall ClearAddonTrustSourcePilot(const char*); // 20123ff6
extern "C" int __fastcall ResolveAddonTrustKeySourcePilot(char*, char*, const char*, DWORD, DWORD); // 20123917
extern "C" void* __cdecl GetAddonPanelsBoundaryPilot(); // panels.dll ordinal 93, IAT 200e13b0
extern "C" int __cdecl FSCreateModeless(const char*, int, int, int);
extern "C" void __cdecl FSShowModeless(int, int);
extern "C" void __cdecl FSUpdateModelessMessage(int, const char*);
extern "C" void __cdecl FSCloseModeless(int);
enum FSAlertType { AddonAlertDefaultPilot = 0 };
int __stdcall FSAlert(const char*, int, FSAlertType);

// 20124051. The profile API returns a bounded, NUL-separated list of key/value pairs.
extern "C" void __stdcall ResetAddonTrustEntriesSourcePilot(int mode)
{
    std::vector<char> entries(32);
    DWORD length;
    do {
        entries.resize(entries.size() * 2);
        length = GetPrivateProfileSectionA("Trusted", &entries[0],
            static_cast<DWORD>(entries.size()), g_configurationFilePilot);
    } while (length == entries.size() - 2 && entries.size() < 0x100000);

    for (size_t offset = 0; offset < length && entries[offset];) {
        char* entry = &entries[offset];
        char* end = static_cast<char*>(memchr(entry, '\0', length - offset));
        if (end == NULL) break;
        char* separator = reinterpret_cast<char*>(_mbsstr(reinterpret_cast<unsigned char*>(entry),
            reinterpret_cast<const unsigned char*>("=")));
        if (separator == NULL) break;
        *separator = '\0';
        int state = atoi(separator + 1);
        const unsigned int value = static_cast<unsigned int>(state);
        const unsigned int selector = static_cast<unsigned int>(mode);
        if (mode == 0 || selector == 0u - value || selector == value - 3u)
            WriteConfigurationIntPilot("Trusted", entry, 0);
        offset += end - entry + 1;
    }
}

static const GUID g_addonManualLoadPilot = {0x9169fecc, 0x0e1a, 0x495b, {0x90, 0x02, 0xba, 0x99, 0xc3, 0x98, 0x2a, 0x95}};
static const GUID g_addonNamePilot = {0x8fd32f84, 0xadf1, 0x4e82, {0xb1, 0x9b, 0x4b, 0xbd, 0xab, 0x30, 0x18, 0x23}};
static const GUID g_addonPathPilot = {0x9e362607, 0xa7b1, 0x4049, {0xa4, 0xf9, 0x14, 0xbb, 0x37, 0x14, 0x68, 0xae}};
static const GUID g_addonCommandLinePilot = {0x9a9ed21d, 0x71ba, 0x44a5, {0xb8, 0xcc, 0xef, 0xef, 0x27, 0xa1, 0x93, 0xc7}};
static const GUID g_addonConsolePilot = {0x822033c3, 0x04c9, 0x4569, {0x86, 0x35, 0x3e, 0xe3, 0x4b, 0x11, 0x3a, 0x32}};
static const GUID g_addonDllTypePilot = {0xeabd5edb, 0x08dc, 0x4fd3, {0x9f, 0x0e, 0x68, 0x20, 0x91, 0x92, 0xcf, 0xa4}};
static const GUID g_addonStartProcPilot = {0x82a0b36d, 0x1c75, 0x4da0, {0xa2, 0x79, 0x08, 0x67, 0x8d, 0x3b, 0x22}};
static const GUID g_addonStopProcPilot = {0xd31b6a4a, 0xb063, 0x48bd, {0xa6, 0x72, 0x62, 0x31, 0x57, 0x1a, 0xa0, 0x82}};
static const GUID g_addonEntryPilot = {0x564b5cf6, 0x2fa6, 0x4988, {0xa7, 0xfe, 0x47, 0x20, 0xba, 0x09, 0xc9, 0x3f}};

static bool AddonPathExistsPilot(const char* path)
{
    unsigned int attributes;
    return QueryPathAttributesPilot(&attributes, path, 0) == 0 && attributes != INVALID_FILE_ATTRIBUTES;
}

static const char* ReadAddonTextPilot(ServiceListenerPropertySetPilot* set, const GUID& property,
    const char* fallback)
{
    const char* text = NULL;
    return SUCCEEDED(SimPropertySet_GetText(set, &property, 0, 0, &text)) && text ? text : fallback;
}

static bool IsAddonDisabledPilot(ServiceListenerPropertySetPilot* set)
{
    BOOL disabled = FALSE;
    return SUCCEEDED(SimPropertySet_GetBool(set, &g_listenerDisabledPilot, 0, 0, &disabled)) && disabled;
}

static std::string FormatAddonTemplateArgsPilot(const char* format, va_list args)
{
    char* message = NULL;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING, format,
        0, 0, reinterpret_cast<char*>(&message), static_cast<DWORD>(strlen(format) * 2), &args);
    std::string result(message ? message : "");
    if (message) LocalFree(message);
    return result;
}

static std::string FormatAddonTemplatePilot(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    std::string result = FormatAddonTemplateArgsPilot(format, args);
    va_end(args);
    return result;
}

static std::string FormatAddonMessagePilot(unsigned int resource, int capacity, ...)
{
    std::vector<char> format(capacity, '\0');
    LoadSplashStringPilot(resource, &format[0], capacity);
    va_list args;
    va_start(args, capacity);
    std::string result = FormatAddonTemplateArgsPilot(&format[0], args);
    va_end(args);
    return result;
}

// 200fcfd8. Read the fixed file version used for the prompt and the DLL policy.
static std::string ReadAddonFileVersionPilot(const char* path)
{
    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSizeA(path, &handle);
    if (size) {
        std::vector<char> block(size);
        VS_FIXEDFILEINFO* info = NULL;
        UINT infoSize = 0;
        if (GetFileVersionInfoA(path, handle, size, &block[0]) &&
            VerQueryValueA(&block[0], "\\", reinterpret_cast<void**>(&info), &infoSize) &&
            infoSize == sizeof(*info) && info->dwSignature == 0xfeef04bd) {
            char version[24];
            sprintf_s(version, sizeof(version), "%u.%u.%u.%u", unsigned(HIWORD(info->dwFileVersionMS)),
                unsigned(LOWORD(info->dwFileVersionMS)), unsigned(HIWORD(info->dwFileVersionLS)),
                unsigned(LOWORD(info->dwFileVersionLS)));
            return version;
        }
    }
    char fallback[256] = {0};
    LoadSplashStringPilot(0xf660, fallback, sizeof(fallback));
    return fallback;
}

// 200fd09e. The last translation with a value wins independently for each field.
static void ReadAddonVersionTextPilot(const char* path, std::string (&fields)[3])
{
    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSizeA(path, &handle);
    if (!size) return;
    std::vector<char> block(size);
    if (!GetFileVersionInfoA(path, handle, size, &block[0])) return;

    WORD* translations = NULL;
    UINT bytes = 0;
    if (!VerQueryValueA(&block[0], "\\VarFileInfo\\Translation",
        reinterpret_cast<void**>(&translations), &bytes)) return;
    static const char* const names[] = {"CompanyName", "ProductName", "ProductVersion"};
    for (UINT i = 0; i < bytes / 4; ++i) {
        for (UINT field = 0; field < 3; ++field) {
            char query[50];
            if (sprintf_s(query, sizeof(query), "\\StringFileInfo\\%04x%04x\\%s",
                unsigned(translations[i * 2]), unsigned(translations[i * 2 + 1]), names[field]) < 0) continue;
            char* text = NULL;
            UINT length = 0;
            if (VerQueryValueA(&block[0], query, reinterpret_cast<void**>(&text), &length))
                fields[field] = text;
        }
    }
}

static const char g_addonLaunchRegistryPilot[] =
    "SOFTWARE\\Microsoft\\Microsoft Games\\Flight Simulator\\10.0\\Launch";

static bool HasAddonLaunchMarkerPilot(const char* key)
{
    HKEY registry;
    DWORD state = 0, type = REG_DWORD, size = sizeof(state);
    if (RegOpenKeyExA(HKEY_CURRENT_USER, g_addonLaunchRegistryPilot, 0, KEY_READ, &registry) == ERROR_SUCCESS) {
        RegQueryValueExA(registry, key, NULL, &type, reinterpret_cast<BYTE*>(&state), &size);
        RegCloseKey(registry);
    }
    return state == 1;
}

static void SetAddonLaunchMarkerPilot(const char* key, bool active)
{
    HKEY registry;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, g_addonLaunchRegistryPilot, 0, NULL, 0,
        KEY_ALL_ACCESS, NULL, &registry, NULL) != ERROR_SUCCESS) return;
    DWORD state = 1;
    if (active)
        RegSetValueExA(registry, key, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&state), sizeof(state));
    else
        RegDeleteValueA(registry, key);
    RegCloseKey(registry);
}

struct LoadedAddonModulePilot {
    int type;
    HMODULE module;
    FARPROC start;
    FARPROC stop;
};
static std::vector<LoadedAddonModulePilot> g_loadedAddonModulesPilot;

static void ActivateAddonModulePilot(const char* path, const char* arguments, const char* version,
    const char* startName, const char* stopName, bool simConnect, const char* launchKey)
{
    int error = -1;
    HMODULE probe = LoadLibraryExA(path, NULL, DONT_RESOLVE_DLL_REFERENCES);
    HMODULE module = NULL;
    if (probe) {
        error = -2;
        bool duplicate = false;
        for (size_t i = 0; i < g_loadedAddonModulesPilot.size(); ++i)
            if (g_loadedAddonModulesPilot[i].module == probe) duplicate = true;
        bool hasExports = false;
        if (duplicate) {
            if (g_servicePacketLogLevelPilot < 4)
                LogServicePacketSourcePilot(0, 1, "DLL already loaded:  Path=\"%s\"", path);
        } else {
            FARPROC start = GetProcAddress(probe, startName);
            FARPROC stop = GetProcAddress(probe, stopName);
            hasExports = start && stop;
            if (g_servicePacketLogLevelPilot < 4) {
                if (!start) LogServicePacketSourcePilot(0, 1,
                    "DLL Start Proc not found:  StartProc=\"%s\"  Path=\"%s\"", startName, path);
                if (!stop) LogServicePacketSourcePilot(0, 1,
                    "DLL Stop Proc not found:  StopProc=\"%s\"  Path=\"%s\"", stopName, path);
            }
        }
        FreeLibrary(probe);
        if (*launchKey) SetAddonLaunchMarkerPilot(launchKey, true);
        if (hasExports) {
            error = -3;
            module = LoadLibraryA(path);
            if (!module && g_servicePacketLogLevelPilot < 4)
                LogServicePacketSourcePilot(0, 1, "DLL Failed to load. Check for missing dependencies.");
        }
    }

    if (module) {
        error = -4;
        FARPROC start = GetProcAddress(module, startName);
        FARPROC stop = GetProcAddress(module, stopName);
        if (g_servicePacketLogLevelPilot < 4) {
            if (!start) LogServicePacketSourcePilot(0, 1,
                "DLL Start Proc not found:  StartProc=\"%s\"  Path=\"%s\"  CommandLine=\"%s\"", startName, path, arguments);
            if (!stop) LogServicePacketSourcePilot(0, 1,
                "DLL Stop Proc not found:  StopProc=\"%s\"  Path=\"%s\"  CommandLine=\"%s\"", stopName, path, arguments);
        }
        if (start) {
            FARPROC panels = GetProcAddress(module, "Panels");
            if (panels) {
                if (g_servicePacketLogLevelPilot < 1)
                    LogServicePacketSourcePilot(0, 1, "Panels data export found and set to %p",
                        GetAddonPanelsBoundaryPilot());
                *reinterpret_cast<void**>(panels) = GetAddonPanelsBoundaryPilot();
            }
            if (simConnect)
                error = reinterpret_cast<int (__stdcall *)(const char*)>(start)(arguments);
            else {
                reinterpret_cast<void (__cdecl *)()>(start)();
                error = 0;
            }
            if (!error) {
                LoadedAddonModulePilot entry = { simConnect ? 0 : 1, module, start, stop };
                g_loadedAddonModulesPilot.push_back(entry);
                module = NULL;
                if (g_servicePacketLogLevelPilot < 2)
                    LogServicePacketSourcePilot(0, 1, "DLL Loaded:  Path=\"%s\"  Version=\"%s\"", path, version);
            }
        }
    }
    if (module) FreeLibrary(module);
    if (error && g_servicePacketLogLevelPilot < 4)
        LogServicePacketSourcePilot(0, 1, "DLL Load Failed:  Error=%d  Path=\"%s\"", error, path);
    if (*launchKey) SetAddonLaunchMarkerPilot(launchKey, false);
}

static void LaunchAddonProcessPilot(const char* path, const char* arguments, const char* version, DWORD flags)
{
    std::vector<char> command(strlen(path) + strlen(arguments) + 10);
    sprintf_s(&command[0], command.size(), "\"%s\" %s", path, arguments);
    STARTUPINFOA startup = {sizeof(startup)};
    PROCESS_INFORMATION process = {0};
    BOOL launched = CreateProcessA(NULL, &command[0], NULL, NULL, TRUE, flags,
        NULL, NULL, &startup, &process);
    if (launched && g_servicePacketLogLevelPilot < 2)
        LogServicePacketSourcePilot(0, 1, "Exe Launched:  Path=\"%s\"  CommandLine=\"%s\"  Version=\"%s\"",
            path, arguments, version);
    if (!launched && g_servicePacketLogLevelPilot < 4)
        LogServicePacketSourcePilot(0, 1, "Exe Launch Failed:  GetLastError=0x%08X  Path=\"%s\"  CommandLine=\"%s\"",
            GetLastError(), path, arguments);
    if (process.hProcess) CloseHandle(process.hProcess);
    if (process.hThread) CloseHandle(process.hThread);
}

class AddonProgressDialogPilot {
public:
    AddonProgressDialogPilot() : handle(0) {}
    ~AddonProgressDialogPilot() { if (handle) FSCloseModeless(handle); }
    void Create(const char* message) { if (!handle) handle = FSCreateModeless(message, 0, 0, 0); }
    void Hide() const { if (handle) FSShowModeless(handle, 0); }
    void Show(const char* message) const {
        if (handle) {
            FSShowModeless(handle, 1);
            FSUpdateModelessMessage(handle, message);
        }
    }
    void Pause() const { if (handle) Sleep(2000); }
private:
    int handle;
    AddonProgressDialogPilot(const AddonProgressDialogPilot&);
    AddonProgressDialogPilot& operator=(const AddonProgressDialogPilot&);
};

static bool ConfirmAddonLaunchPilot(const char* message, bool prompt, AddonProgressDialogPilot& progress)
{
    if (!prompt) {
        progress.Show(message);
        return true;
    }
    progress.Hide();
    return FSAlert(message, 0xc0, AddonAlertDefaultPilot) != 0x80;
}

static void StartAddonEntryPilot(ServiceListenerPropertySetPilot* entry, bool dll, int index,
    bool showProgress, bool documentPrompt, AddonProgressDialogPilot& progress)
{
    if (IsAddonDisabledPilot(entry)) return;
    BOOL entryPrompt = FALSE;
    SimPropertySet_GetBool(entry, &g_addonManualLoadPilot, 0, 0, &entryPrompt);
    const char* name = ReadAddonTextPilot(entry, g_addonNamePilot, "");
    const char* path = ReadAddonTextPilot(entry, g_addonPathPilot, "");
    const char* arguments = ReadAddonTextPilot(entry, g_addonCommandLinePilot, "");
    DWORD creationFlags = 0x04000020;
    if (!dll) {
        BOOL console = FALSE;
        if (SUCCEEDED(SimPropertySet_GetBool(entry, &g_addonConsolePilot, 0, 0, &console)) || console)
            creationFlags = 0x04000030;
    }
    if (!dll && !AddonPathExistsPilot(path)) {
        if (g_servicePacketLogLevelPilot < 4)
            LogServicePacketSourcePilot(0, 1, "File not found: Path=\"%s\"", path);
        return;
    }
    size_t pathLength = strlen(path);
    const char* suffix = StrStrIA(path, dll ? ".dll" : ".exe");
    if (!suffix || pathLength < 4 || suffix != path + pathLength - 4) return;
    if (dll && !AddonPathExistsPilot(path)) {
        if (g_servicePacketLogLevelPilot < 4)
            LogServicePacketSourcePilot(0, 1, "File not found: Path=\"%s\"", path);
        return;
    }

    bool simConnect = false;
    const char* startName = NULL;
    const char* stopName = NULL;
    if (dll) {
        simConnect = _mbsicmp(
            reinterpret_cast<const unsigned char*>(ReadAddonTextPilot(entry, g_addonDllTypePilot, "")),
            reinterpret_cast<const unsigned char*>("SimConnect")) == 0;
        startName = ReadAddonTextPilot(entry, g_addonStartProcPilot, simConnect ? "SimConnectStart" : "DLLStart");
        stopName = ReadAddonTextPilot(entry, g_addonStopProcPilot, simConnect ? "SimConnectStop" : "DLLStop");
    }
    if (pathLength + strlen(arguments) - 5 > 0x7ff0) {
        if (g_servicePacketLogLevelPilot < 4) {
            if (dll) LogServicePacketSourcePilot(0, 1, "DLL Load Error:  Path=\"%s\"", path);
            else LogServicePacketSourcePilot(0, 1, "Exe Launch Error: Name=\"%s\"  Path=\"%s\"  CommandLine=\"%s\"",
                name, path, arguments);
        }
        return;
    }

    std::string version = ReadAddonFileVersionPilot(path);
    char compact[52] = {0};
    const char* displayPath = PathCompactPathExA(compact, path, 0x32, 0) ? compact : path;
    std::string message = FormatAddonMessagePilot(0xf661, 256, name, version.c_str(), index + 1, displayPath);
    const bool prompt = documentPrompt || entryPrompt != FALSE;
    if (showProgress && !prompt) progress.Create(message.c_str());

    if (!dll) {
        if (!ConfirmAddonLaunchPilot(message.c_str(), prompt, progress) ||
            !CheckAddonTrustSourcePilot(0, 1, path, NULL)) return;
        LaunchAddonProcessPilot(path, arguments, version.c_str(), creationFlags);
        progress.Pause();
        return;
    }

    char launchKey[512] = {0};
    ResolveAddonTrustKeySourcePilot(launchKey, NULL, path, 0, sizeof(launchKey));
    if (*launchKey && HasAddonLaunchMarkerPilot(launchKey)) {
        progress.Hide();
        std::string fields[3];
        ReadAddonVersionTextPilot(path, fields);
        char unknown[256] = {0};
        LoadSplashStringPilot(0xf660, unknown, sizeof(unknown));
        for (int i = 0; i < 3; ++i)
            if (fields[i].empty()) fields[i] = unknown;
        std::string warning = FormatAddonMessagePilot(0xf662, 2048,
            fields[0].c_str(), fields[1].c_str(), fields[2].c_str());
        if (MessageBoxA(NULL, warning.c_str(), NULL, 0x114) == IDNO) return;
    } else if (!ConfirmAddonLaunchPilot(message.c_str(), prompt, progress)) {
        return;
    }

    DWORD trustFlags = 0;
    if (version == "10.0.60905.0" || version == "10.0.61110.1" ||
        version == "10.0.61355.0" || version == "10.0.61357.0")
        trustFlags = 0x80000000;
    if (!CheckAddonTrustSourcePilot(0, 1, path, &trustFlags)) return;
    if ((trustFlags & 1) && version != "10.0.61637.0") {
        if (g_servicePacketLogLevelPilot < 2)
            LogServicePacketSourcePilot(0, 1, "SDK Addon not loaded - version mismatch:  Path=\"%s\"  Version=\"%s\"",
                path, version.c_str());
        ClearAddonTrustSourcePilot(path);
        return;
    }
    ActivateAddonModulePilot(path, arguments, version.c_str(), startName, stopName, simConnect, launchKey);
    progress.Pause();
}

// 200fbb94 and 200fc2b7 share the schema and property-set enumeration.
static void StartAddonsFromXmlPilot(bool dll, bool showProgress, bool forcePrompt)
{
    char path[MAX_PATH] = {0};
    BuildServiceListenerConfigurationPathBoundaryPilot(0x1a, path, sizeof(path), NULL, dll ? "DLL.xml" : "EXE.xml");
    if (!AddonPathExistsPilot(path)) return;
    ServiceListenerPropertySetOwnerPilot document;
    if (FAILED(SimPropertySet_ReadFile(path, &g_listenerSchemaPilot, 0, 2, 0,
        &g_listenerDocumentPilot, &document.value)) || !document.value || IsAddonDisabledPilot(document.value)) return;

    BOOL manualLoad = FALSE;
    SimPropertySet_GetBool(document.value, &g_addonManualLoadPilot, 0, 0, &manualLoad);
    typedef int (__stdcall *CountEntriesPilot)(ServiceListenerPropertySetPilot*, const GUID*);
    typedef HRESULT (__stdcall *ReadEntryPilot)(ServiceListenerPropertySetPilot*, const GUID*, DWORD,
        ServiceListenerPropertySetPilot**);
    int count = GraphicsClipSlotPilot<CountEntriesPilot>(document.value, 0x38)(document.value, &g_addonEntryPilot);
    AddonProgressDialogPilot progress;
    for (int index = 0; index < count; ++index) {
        ServiceListenerPropertySetOwnerPilot entry;
        if (SUCCEEDED(GraphicsClipSlotPilot<ReadEntryPilot>(document.value, 0x3c)(
            document.value, &g_addonEntryPilot, index, &entry.value)) && entry.value)
            StartAddonEntryPilot(entry.value, dll, index, showProgress, manualLoad || forcePrompt, progress);
    }
}

extern "C" void __stdcall StartExeAddonsSourcePilot(bool showProgress, bool forcePrompt)
{
    StartAddonsFromXmlPilot(false, showProgress, forcePrompt);
}

extern "C" void __stdcall StartDllAddonsSourcePilot(bool showProgress, bool forcePrompt)
{
    StartAddonsFromXmlPilot(true, showProgress, forcePrompt);
}

// 200fb207 / ordinal 322. Both readers consume only the low byte of each slot.
extern "C" void __cdecl StartApiAddonsSourcePilot()
{
    if (g_addonTrustResetSelector201f9130Pilot == 0) {
        ResetAddonTrustEntriesSourcePilot(1);
        ResetAddonTrustEntriesSourcePilot(2);
    } else {
        ResetAddonTrustEntriesSourcePilot(0);
    }
    if (g_skipAddonStartup201f7d80Pilot != 0) return;
    ConfigureServiceListenersSourcePilot();
    const bool showProgress = g_addonProgress201f8e90Pilot != 0;
    const bool forcePrompt = g_addonPrompt201f8fe0Pilot != 0;
    StartExeAddonsSourcePilot(showProgress, forcePrompt);
    StartDllAddonsSourcePilot(showProgress, forcePrompt);
}

// 200fb2ad. Dispatch remains an external SimConnect operation.
extern "C" void __cdecl StopApiAddonsSourcePilot()
{
    MainServiceCallback7ba9Pilot(0, 0);
    for (size_t i = g_loadedAddonModulesPilot.size(); i != 0; --i) {
        const LoadedAddonModulePilot& entry = g_loadedAddonModulesPilot[i - 1];
        if (entry.stop) reinterpret_cast<void (__cdecl *)()>(entry.stop)();
        FreeLibrary(entry.module);
    }
    std::vector<LoadedAddonModulePilot>().swap(g_loadedAddonModulesPilot);
}
