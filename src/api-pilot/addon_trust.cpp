#include <wincrypt.h>

extern "C" BYTE g_addonTrustSeedPilot[16]; // 2020e2ec

// The protected region can resume the ordinary path or complete the request.
// This interface models the two continuations without copying the gateway ABI.
extern "C" bool RunProtectedAddonTrustKeyBoundaryPilot(char* key, DWORD keyCapacity,
    const char* resolvedPath, char* cachedKey, DWORD cacheCapacity, int* result);

static const char g_trustedAddonSectionPilot[] = "Trusted";
static char g_cachedAddonTrustKeyPilot[0x131];
static size_t g_cachedAddonTrustPathLengthPilot;

static bool FinishAddonHashPilot(HANDLE file, HCRYPTHASH hash, BYTE (&digest)[20])
{
    BYTE buffer[2048];
    DWORD length = 0;
    while (ReadFile(file, buffer, sizeof(buffer), &length, NULL)) {
        if (!length) {
            DWORD digestLength = sizeof(digest);
            return CryptGetHashParam(hash, HP_HASHVAL, digest, &digestLength, 0) != FALSE &&
                digestLength == sizeof(digest);
        }
        if (!CryptHashData(hash, buffer, length, 0)) break;
    }
    return false;
}

static void AppendAddonTrustHashPilot(const char* path)
{
    HANDLE file = INVALID_HANDLE_VALUE;
    HCRYPTHASH hash = 0;
    char suffix[42] = ".";
    if (g_addonHashProviderPilot || CryptAcquireContextA(&g_addonHashProviderPilot,
        NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        BYTE digest[20];
        if (file != INVALID_HANDLE_VALUE &&
            CryptCreateHash(g_addonHashProviderPilot, CALG_SHA1, 0, 0, &hash) &&
            CryptHashData(hash, g_addonTrustSeedPilot, sizeof(g_addonTrustSeedPilot), 0) &&
            FinishAddonHashPilot(file, hash, digest)) {
            static const char alphabet[] = "thequickbrownlazy";
            for (size_t i = 0; i < sizeof(digest); ++i) {
                suffix[1 + i * 2] = alphabet[digest[i] >> 4];
                suffix[2 + i * 2] = alphabet[digest[i] & 0xf];
            }
        }
    }
    DWORD error = GetLastError();
    if (hash) CryptDestroyHash(hash);
    if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
    if (error == ERROR_SUCCESS)
        strcat_s(g_cachedAddonTrustKeyPilot, sizeof(g_cachedAddonTrustKeyPilot), suffix);
}

// 20123917. ECX/EDX are the optional key and normalized-path destinations.
extern "C" int __fastcall ResolveAddonTrustKeySourcePilot(char* key, char* resolvedPath,
    const char* path, DWORD pathCapacity, DWORD keyCapacity)
{
    char scratch[MAX_PATH];
    if (!resolvedPath) {
        resolvedPath = scratch;
        pathCapacity = sizeof(scratch);
    }
    unsigned int attributes;
    if (ResolvePathPilot(resolvedPath, pathCapacity, path, 0) != 0 ||
        QueryPathAttributesPilot(&attributes, resolvedPath, 0) != 0 ||
        attributes == INVALID_FILE_ATTRIBUTES) return 0;

    const size_t length = strnlen(resolvedPath, pathCapacity);
    if (length != g_cachedAddonTrustPathLengthPilot ||
        strncmp(resolvedPath, g_cachedAddonTrustKeyPilot, length) != 0) {
        g_cachedAddonTrustPathLengthPilot = length;
        strcpy_s(g_cachedAddonTrustKeyPilot, sizeof(g_cachedAddonTrustKeyPilot), resolvedPath);
        static const BYTE emptySeed[16] = {0};
        int result;
        if (memcmp(g_addonTrustSeedPilot, emptySeed, sizeof(emptySeed)) == 0 &&
            RunProtectedAddonTrustKeyBoundaryPilot(key, keyCapacity, resolvedPath,
                g_cachedAddonTrustKeyPilot, sizeof(g_cachedAddonTrustKeyPilot), &result))
            return result;
        AppendAddonTrustHashPilot(resolvedPath);
    }
    if (key) strcpy_s(key, keyCapacity, g_cachedAddonTrustKeyPilot);
    return 1;
}

// 20123ff6. Writes the numeric value zero after successful key resolution.
extern "C" void __stdcall ClearAddonTrustSourcePilot(const char* path)
{
    char key[0x131];
    if (ResolveAddonTrustKeySourcePilot(key, NULL, path, 0, sizeof(key)))
        WriteConfigurationIntPilot(g_trustedAddonSectionPilot, key, 0);
}

static int PromptAddonPublisherPilot(const char* resolvedPath)
{
    char* text = NULL;
    AllocateResourceStringPilot(0x1a, &text);
    std::string format(text ? text : "");
    void* resource = text;
    FreePrivateHeapSlotPilot(&resource);
    char compact[52] = {0};
    const char* displayPath = PathCompactPathExA(compact, resolvedPath, sizeof(compact), 0) ?
        compact : resolvedPath;
    std::string message = FormatAddonTemplatePilot(format.c_str(), compact, displayPath);
    return FSAlert(message.c_str(), 0xc0, AddonAlertDefaultPilot);
}

// 20123c41 / ordinal 305. A cached negative value or positive mode match
// normally returns before publisher verification. Bit 31 requests the provider path.
extern "C" int __stdcall CheckAddonTrustSourcePilot(int reset, int mode, const char* path, DWORD* flags)
{
    if (reset) {
        ResetAddonTrustEntriesSourcePilot(mode);
        return 1;
    }

    char resolvedPath[MAX_PATH], key[0x131];
    if (!ResolveAddonTrustKeySourcePilot(key, resolvedPath, path, sizeof(resolvedPath), sizeof(key)))
        return 0;
    int stored = 0;
    const bool read = ReadSelectedConfigurationIntPilot(g_trustedAddonSectionPilot, key, &stored, NULL, 0) == 0;
    const int rememberedMode = static_cast<int>(static_cast<unsigned int>(mode) + 3);
    const bool accepted = read && (stored == mode || stored == rememberedMode);
    const bool decided = accepted || (read && stored < 0);
    if (decided && (!flags || (*flags & 0x80000000u) == 0)) return accepted;

    if (!EnsureAddonTrustProviderPilot()) return 0;
    RegisterAddonTrustShutdownPilot();
    g_addonTrustProviderPhasePilot = 7;
    if (!VerifyAddonPublisherPilot(resolvedPath, true, false)) {
        if (ClassifyAddonPublisherPilot(resolvedPath) == 3) {
            if (flags) *flags |= 1;
        } else {
            if (decided) return accepted;
            if (!VerifyAddonPublisherPilot(resolvedPath, false, true)) {
                WriteConfigurationIntPilot(g_trustedAddonSectionPilot, key,
                    static_cast<int>(0u - static_cast<unsigned int>(mode)));
                return 0;
            }
            if (!VerifyAddonPublisherPilot(resolvedPath, true, false) &&
                PromptAddonPublisherPilot(resolvedPath) != 0x40) {
                WriteConfigurationIntPilot(g_trustedAddonSectionPilot, key, rememberedMode);
                return 1;
            }
        }
    }
    WriteConfigurationIntPilot(g_trustedAddonSectionPilot, key, mode);
    return 1;
}
