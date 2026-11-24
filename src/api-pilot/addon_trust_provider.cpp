#include <wincrypt.h>
#include <wintrust.h>

typedef HRESULT (WINAPI *AddonVerifyTrustPilot)(HWND, GUID*, WINTRUST_DATA*);
typedef void (WINAPI *AddonGetPolicyFlagsPilot)(DWORD*);
typedef void (WINAPI *AddonSetPolicyFlagsPilot)(DWORD);
typedef CRYPT_PROVIDER_DATA* (WINAPI *AddonReadProviderDataPilot)(HANDLE);
typedef CRYPT_PROVIDER_SGNR* (WINAPI *AddonReadProviderSignerPilot)(CRYPT_PROVIDER_DATA*, DWORD, BOOL, DWORD);
typedef BOOL (WINAPI *AddonVerifyChainPolicyPilot)(LPCSTR, PCCERT_CHAIN_CONTEXT,
    PCERT_CHAIN_POLICY_PARA, PCERT_CHAIN_POLICY_STATUS);

struct AddonTrustProviderPilot {
    HMODULE winTrust;
    AddonVerifyTrustPilot verify;
    AddonGetPolicyFlagsPilot getPolicy;
    AddonSetPolicyFlagsPilot setPolicy;
    AddonReadProviderDataPilot providerData;
    AddonReadProviderSignerPilot signer;
    HMODULE crypt32;
    AddonVerifyChainPolicyPilot verifyChain;
};
C_ASSERT(sizeof(AddonTrustProviderPilot) == 0x20);
C_ASSERT(offsetof(AddonTrustProviderPilot, verifyChain) == 0x1c);
C_ASSERT(sizeof(WINTRUST_DATA) == 0x30);
C_ASSERT(sizeof(WINTRUST_FILE_INFO) == 0x10);
C_ASSERT(offsetof(CRYPT_PROVIDER_SGNR, pChainContext) == 0x28);

static AddonTrustProviderPilot* g_addonTrustProviderPilot; // 2020de90
static DWORD g_addonTrustProviderPhasePilot; // 2020de94
static HCRYPTPROV g_addonHashProviderPilot; // 2020de8c
static bool g_addonTrustShutdownRegisteredPilot; // 2020de1b
extern "C" HWND __cdecl GetFsUiTrustOwnerBoundaryPilot(); // fsui.dll ordinal 6, IAT 200e12e0

// 20147f31. An allocated provider can remain present with an empty table.
static void BindAddonTrustProviderPilot(AddonTrustProviderPilot& provider)
{
    provider.winTrust = LoadLibraryExW(L"WINTRUST.DLL", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!provider.winTrust) return;
    provider.verify = reinterpret_cast<AddonVerifyTrustPilot>(GetProcAddress(provider.winTrust, "WinVerifyTrustEx"));
    provider.getPolicy = reinterpret_cast<AddonGetPolicyFlagsPilot>(GetProcAddress(provider.winTrust, "WintrustGetRegPolicyFlags"));
    provider.setPolicy = reinterpret_cast<AddonSetPolicyFlagsPilot>(GetProcAddress(provider.winTrust, "WintrustSetRegPolicyFlags"));
    provider.providerData = reinterpret_cast<AddonReadProviderDataPilot>(GetProcAddress(provider.winTrust, "WTHelperProvDataFromStateData"));
    provider.signer = reinterpret_cast<AddonReadProviderSignerPilot>(GetProcAddress(provider.winTrust, "WTHelperGetProvSignerFromChain"));
    if (!provider.verify || !provider.getPolicy || !provider.setPolicy || !provider.providerData || !provider.signer) {
        FreeLibrary(provider.winTrust);
        ZeroMemory(&provider, sizeof(provider));
    }
}

static bool EnsureAddonTrustProviderPilot()
{
    if (!g_addonTrustProviderPilot) {
        void* storage = NULL;
        AllocatePrivateHeapMemoryPilot(1, sizeof(AddonTrustProviderPilot), &storage,
            g_factoryZeroInitializeBoundaryPilot);
        if (!storage) return false;
        AddonTrustProviderPilot* provider = new (storage) AddonTrustProviderPilot;
        ZeroMemory(provider, sizeof(*provider));
        BindAddonTrustProviderPilot(*provider);
        g_addonTrustProviderPilot = provider;
    }
    return true;
}

// 201234e2. Bind on demand and permit another attempt after a failed binding.
static bool EnsureAddonChainPolicyPilot(AddonTrustProviderPilot& provider)
{
    if (!provider.winTrust) return false;
    if (!provider.crypt32) {
        HMODULE crypt32 = LoadLibraryExW(L"Crypt32.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!crypt32) return false;
        provider.verifyChain = reinterpret_cast<AddonVerifyChainPolicyPilot>(
            GetProcAddress(crypt32, "CertVerifyCertificateChainPolicy"));
        if (!provider.verifyChain) {
            FreeLibrary(crypt32);
            return false;
        }
        provider.crypt32 = crypt32;
    }
    return true;
}

// 2012387e and the common release sequence shared by the two shutdown paths.
static void ReleaseAddonTrustProviderPilot()
{
    if (g_addonHashProviderPilot) {
        CryptReleaseContext(g_addonHashProviderPilot, 0);
        g_addonHashProviderPilot = 0;
    }
    AddonTrustProviderPilot* provider = g_addonTrustProviderPilot;
    if (provider) {
        if (provider->winTrust) FreeLibrary(provider->winTrust);
        if (provider->crypt32) FreeLibrary(provider->crypt32);
        HeapFree(g_privateHeapPilot, 0, provider);
        g_addonTrustProviderPilot = NULL;
    }
}

// 201238c8. The registered callback uses callee stack cleanup (RET 8).
extern "C" void __stdcall AgeAddonTrustProviderSourcePilot(ULONG_PTR, ULONG_PTR)
{
    if (!g_addonTrustProviderPhasePilot) return;
    if (g_addonTrustProviderPhasePilot == 1) ReleaseAddonTrustProviderPilot();
    --g_addonTrustProviderPhasePilot;
}

static RegisteredCallbackBoundaryPilot AddonTrustShutdownCallbackPilot()
{
    return reinterpret_cast<RegisteredCallbackBoundaryPilot>(AgeAddonTrustProviderSourcePilot);
}

static void RegisterAddonTrustShutdownPilot()
{
    if (!g_addonTrustShutdownRegisteredPilot) {
        g_addonTrustShutdownRegisteredPilot = true;
        g_initializeCallbackRegistryBoundaryPilot.Register(AddonTrustShutdownCallbackPilot(), 0, 0, 0);
    }
}

// 20123fa9. Original caller remains in the unreconstructed shutdown prefix.
extern "C" void __cdecl ShutDownAddonTrustProviderSourcePilot()
{
    ReleaseAddonTrustProviderPilot();
    RemoveMainCallbackListEntryPilot(&g_initializeCallbackRegistryBoundaryPilot.data,
        AddonTrustShutdownCallbackPilot(), 0, 0, 0);
}

// 201232d0. The controller predicate can be queried twice.
static HWND SelectAddonTrustOwnerPilot()
{
    HWND owner = GetFsUiTrustOwnerBoundaryPilot();
    if (owner) return owner;
    if (((g_idleControllerPilot && g_idleControllerPilot->QueryActive()) || g_idleUnthrottledPilot) &&
        g_idleControllerPilot && g_idleControllerPilot->QueryActive())
        return g_mainWindowHandlePilot;
    return GetDesktopWindow();
}

static bool ConvertAddonTrustPathPilot(const char* path, WCHAR (&widePath)[MAX_PATH])
{
    if (!path) return false;
    const size_t length = strnlen(path, MAX_PATH);
    if (!length || length == MAX_PATH) return false;
    ZeroMemory(widePath, sizeof(widePath));
    return MultiByteToWideChar(CP_ACP, 0, path, static_cast<int>(length), widePath,
        static_cast<int>(length)) != 0;
}

static const GUID g_addonVerifyActionPilot =
    {0x00aac56b, 0xcd44, 0x11d0, {0x8c, 0xc2, 0x00, 0xc0, 0x4f, 0xc2, 0x95, 0xee}};

static void InitializeAddonTrustRequestPilot(WINTRUST_DATA& request, WINTRUST_FILE_INFO& file,
    const WCHAR* path, DWORD uiChoice, DWORD stateAction)
{
    ZeroMemory(&file, sizeof(file));
    file.cbStruct = sizeof(file);
    file.pcwszFilePath = path;
    ZeroMemory(&request, sizeof(request));
    request.cbStruct = sizeof(request);
    request.dwUIChoice = uiChoice;
    request.dwUnionChoice = WTD_CHOICE_FILE;
    request.pFile = &file;
    request.dwStateAction = stateAction;
}

// 20123536. Plain verification leaves WinTrust state ownership with Windows.
static bool VerifyAddonPublisherPilot(const char* path, bool applyPolicyFlags, bool interactive)
{
    AddonTrustProviderPilot* provider = g_addonTrustProviderPilot;
    WCHAR widePath[MAX_PATH];
    if (!provider || !provider->winTrust || !ConvertAddonTrustPathPilot(path, widePath)) return false;

    WINTRUST_DATA request;
    WINTRUST_FILE_INFO file;
    InitializeAddonTrustRequestPilot(request, file, widePath, interactive ? WTD_UI_ALL : WTD_UI_NONE,
        WTD_STATEACTION_IGNORE);
    DWORD oldPolicy = 0;
    if (applyPolicyFlags) {
        provider->getPolicy(&oldPolicy);
        provider->setPolicy(oldPolicy | 0x40000);
    }
    HWND owner = SelectAddonTrustOwnerPilot();
    GUID action = g_addonVerifyActionPilot;
    if (interactive && g_idleControllerPilot) g_idleControllerPilot->EnterModalWindowPilot();
    HRESULT result = provider->verify(owner, &action, &request);
    if (interactive && g_idleControllerPilot) g_idleControllerPilot->LeaveModalWindowPilot();
    if (applyPolicyFlags) provider->setPolicy(oldPolicy);
    return SUCCEEDED(result);
}

static int ClassifyAddonSignerChainPilot(AddonTrustProviderPilot& provider, HANDLE state)
{
    CRYPT_PROVIDER_DATA* data = provider.providerData(state);
    if (!data) return 1;
    CRYPT_PROVIDER_SGNR* signer = provider.signer(data, 0, FALSE, 0);
    if (!signer) return 1;

    CERT_CHAIN_POLICY_PARA policy = {sizeof(policy)};
    CERT_CHAIN_POLICY_STATUS status = {sizeof(status)};
    if (!provider.verifyChain(CERT_CHAIN_POLICY_MICROSOFT_ROOT, signer->pChainContext, &policy, &status))
        return 1;
    if (!status.dwError) return 3;
    if (provider.verifyChain(CERT_CHAIN_POLICY_AUTHENTICODE, signer->pChainContext, &policy, &status) &&
        !status.dwError) return 4;
    return 1;
}

// 20123694. Only the successful VERIFY path reaches the paired CLOSE.
static int ClassifyAddonPublisherPilot(const char* path)
{
    AddonTrustProviderPilot* provider = g_addonTrustProviderPilot;
    WCHAR widePath[MAX_PATH];
    if (!path || !provider || !EnsureAddonChainPolicyPilot(*provider) ||
        !ConvertAddonTrustPathPilot(path, widePath)) return 1;

    WINTRUST_DATA request;
    WINTRUST_FILE_INFO file;
    InitializeAddonTrustRequestPilot(request, file, widePath, WTD_UI_NONE, WTD_STATEACTION_VERIFY);
    HWND owner = SelectAddonTrustOwnerPilot();
    GUID action = g_addonVerifyActionPilot;
    HRESULT result = provider->verify(owner, &action, &request);
    if (result != S_OK) {
        if ((result >= TRUST_E_PROVIDER_UNKNOWN && result <= TRUST_E_SUBJECT_NOT_TRUSTED) ||
            result == TRUST_E_NOSIGNATURE) return 2;
        return 1;
    }
    int classification = ClassifyAddonSignerChainPilot(*provider, request.hWVTStateData);
    request.dwStateAction = WTD_STATEACTION_CLOSE;
    provider->verify(owner, &action, &request);
    return classification;
}
