#include <stddef.h>

struct GpsProvider;
typedef unsigned int (__thiscall *GpsProviderRelease)(GpsProvider*);
typedef void (__stdcall *GpsLegacyCallback)(unsigned int eventCode, unsigned int argument);

struct GpsProviderTable {
    void* firstSlot;
    GpsProviderRelease release;
};

struct GpsOwnedStorage {
    int count;
    unsigned int capacity;
    unsigned short field08;
    unsigned short flags;
    void** entries;

    __forceinline void ReleaseBuffer();
};
typedef char GpsOwnedStorageSizeCheck[sizeof(GpsOwnedStorage) == 0x10 ? 1 : -1];

// The observed 24-byte provider layout.
struct GpsProvider {
    const GpsProviderTable* volatile table;
    unsigned int references;
    GpsOwnedStorage storage;

    unsigned int Release();
    __declspec(noinline) void ReleaseEntries();
};
typedef char GpsProviderSizeCheck[sizeof(GpsProvider) == 0x18 ? 1 : -1];

struct GpsLegacyRecord {
    unsigned int size;
    unsigned int field04;
    unsigned char reserved08[0x18];
    double field20;
    double field28;
    double field30;
    double field38;
    unsigned int field40;
    unsigned int field44;
    unsigned int field48;
    unsigned int field4c;
    unsigned char reserved50[0x30];
    double field80;
    unsigned char reserved88[8];
    double field90;
    double field98;
    double fielda0;
    unsigned int fielda8;
    void* waypointEntries;
};
typedef char GpsLegacyRecordSizeCheck[sizeof(GpsLegacyRecord) == 0xb0 ? 1 : -1];
typedef char GpsLegacyRecordPointerCheck[offsetof(GpsLegacyRecord, waypointEntries) == 0xac ? 1 : -1];

extern "C" {
__declspec(dllimport) void __stdcall PanelsOrdinal72(const char* name, void* provider);
__declspec(dllimport) void __stdcall PanelsOrdinal34(GpsLegacyRecord* record, unsigned int selector,
                                                      const char* name);
__declspec(dllimport) void __stdcall PanelsOrdinal38(const char* name);
__declspec(dllimport) void __stdcall ApiOrdinal232(unsigned int slot, GpsLegacyCallback callback);
__declspec(dllimport) void __stdcall ApiOrdinal231(unsigned int slot, GpsLegacyCallback callback);
__declspec(dllimport) void __stdcall ApiOrdinal106(unsigned int kind, unsigned int bytes,
                                                  void* output, unsigned int context);
__declspec(dllimport) void __stdcall ApiOrdinal108(void** allocation);
__declspec(dllimport) unsigned int __stdcall MultiplayerOrdinal4();
__declspec(noinline) void __stdcall DestroyGpsProvider(GpsProvider* provider);
extern const GpsProviderTable GpsProviderBaseTable;
extern const GpsProviderTable GpsProviderTableInstance;
extern unsigned int GpsAllocationContext;
extern unsigned int GpsMultiplayerValue;
extern GpsOwnedStorage* GpsLegacyStorage;
extern GpsLegacyRecord GpsLegacyData;
extern GpsLegacyRecord GpsLegacyRequest;
extern unsigned char GpsLegacyRequestEntries[];
__declspec(noinline) void __stdcall GpsLegacyDataLifecycle(unsigned int eventCode, unsigned int argument);
void __stdcall GpsLegacyDataUpdate(unsigned int eventCode, unsigned int argument);

void __cdecl RegisterGpsProvider()
{
    GpsProvider* allocation;
    ApiOrdinal106(1, sizeof(GpsProvider), &allocation, GpsAllocationContext);
    GpsProvider* const provider = allocation;
    if (provider == 0)
        return;

    provider->table = &GpsProviderBaseTable;
    provider->table = &GpsProviderTableInstance;
    provider->storage.entries = 0;
    provider->storage.count = 0;
    provider->storage.capacity = 0;
    provider->storage.field08 = 5;
    provider->storage.flags = 4;
    provider->references = 1;
    PanelsOrdinal72("FS9GPS", provider);
    provider->table->release(provider);
    ApiOrdinal231(0x10, GpsLegacyDataLifecycle);
    ApiOrdinal231(2, GpsLegacyDataUpdate);
    GpsMultiplayerValue = MultiplayerOrdinal4();
}

void __cdecl UnregisterGpsProvider()
{
    PanelsOrdinal72("FS9GPS", 0);
    ApiOrdinal232(2, GpsLegacyDataUpdate);
    ApiOrdinal232(0x10, GpsLegacyDataLifecycle);
}
}

void GpsOwnedStorage::ReleaseBuffer()
{
    void* allocation = entries;
    if (allocation != 0 && (flags & 1) != 0 && capacity != 0) {
        count = count > 0 ? 0 : count;
        ApiOrdinal108(&allocation);
        flags |= 1;
        entries = 0;
        capacity = 0;
    }
}

void GpsProvider::ReleaseEntries()
{
    for (int index = 0; index < storage.count; ++index) {
        void* entry = storage.entries[index];
        ApiOrdinal108(&entry);
    }
    storage.count = 0;
}

struct GpsStorageCleanup {
    GpsOwnedStorage* storage;

    __forceinline ~GpsStorageCleanup() { storage->ReleaseBuffer(); }
};

void __stdcall DestroyGpsProvider(GpsProvider* provider)
{
    provider->table = &GpsProviderTableInstance;
    GpsStorageCleanup cleanup = { &provider->storage };
    (void)cleanup;
    provider->ReleaseEntries();
}

extern "C" __declspec(noinline) void __cdecl ReleaseGpsLegacyStorageBody(GpsOwnedStorage* storage)
{
    storage->ReleaseBuffer();
    void* allocation = storage;
    ApiOrdinal108(&allocation);
}

// The original caller passes the pointer in ESI and expects it back in EAX.
extern "C" __declspec(naked) void ReleaseGpsLegacyStorage()
{
    __asm {
        push esi
        call ReleaseGpsLegacyStorageBody
        add esp, 4
        mov eax, esi
        ret
    }
}

extern "C" __declspec(noinline) void __fastcall InitializeGpsLegacyDataBody(
    GpsLegacyRecord* record, unsigned int, void* entries)
{
    record->field20 = 0.0;
    record->field04 = 0;
    record->field28 = 0.0;
    record->field40 = 0;
    record->field30 = 0.0;
    record->field44 = 0;
    record->field38 = 0.0;
    record->field48 = 0;
    record->field80 = 0.0;
    record->field4c = 0;
    record->field90 = 0.0;
    record->fielda8 = 0;
    record->field98 = 0.0;
    record->fielda0 = 0.0;
    record->size = sizeof(GpsLegacyRecord);
    record->waypointEntries = entries;
}

// The original initializer receives its record in EAX and pops one stack argument.
extern "C" __declspec(naked) void InitializeGpsLegacyData()
{
    __asm {
        mov ecx, eax
        jmp InitializeGpsLegacyDataBody
    }
}

void __stdcall GpsLegacyDataLifecycle(unsigned int eventCode, unsigned int)
{
    if (eventCode == 0x36) {
        GpsOwnedStorage* storage = GpsLegacyStorage;
        if (storage == 0) {
            GpsOwnedStorage* allocation;
            ApiOrdinal106(1, sizeof(GpsOwnedStorage), &allocation, GpsAllocationContext);
            storage = allocation;
            if (storage != 0) {
                storage->flags = 4;
                storage->entries = 0;
                storage->count = 0;
                storage->capacity = 0;
                storage->field08 = 5;
            }
            GpsLegacyStorage = storage;
        }

        // The original reads the initialized storage directly after allocation.
        void* entries = storage->entries;
        __asm {
            push entries
            mov eax, offset GpsLegacyData
            call InitializeGpsLegacyData
        }

        GpsLegacyRequest.field20 = 0.0;
        GpsLegacyRequest.field28 = 0.0;
        GpsLegacyRequest.field30 = 0.0;
        GpsLegacyRequest.size = sizeof(GpsLegacyRecord);
        GpsLegacyRequest.field38 = 0.0;
        GpsLegacyRequest.field04 = 0;
        GpsLegacyRequest.field80 = 0.0;
        GpsLegacyRequest.field40 = 0;
        GpsLegacyRequest.field90 = 0.0;
        GpsLegacyRequest.field44 = 0;
        GpsLegacyRequest.field98 = 0.0;
        GpsLegacyRequest.field48 = 0;
        GpsLegacyRequest.fielda0 = 0.0;
        GpsLegacyRequest.field4c = 0;
        GpsLegacyRequest.fielda8 = 0;
        GpsLegacyRequest.waypointEntries = GpsLegacyRequestEntries;
        GpsLegacyDataUpdate(0, 0);
        PanelsOrdinal34(&GpsLegacyData, 0x26, "FS2002 GPS DATA");
        PanelsOrdinal34(&GpsLegacyRequest, 0x26, "FS2002 GPS DATA REQUEST");
    } else if (eventCode == 0x37) {
        PanelsOrdinal38("FS2002 GPS DATA REQUEST");
        PanelsOrdinal38("FS2002 GPS DATA");
        GpsOwnedStorage* const storage = GpsLegacyStorage;
        if (storage != 0) {
            __asm {
                mov esi, storage
                call ReleaseGpsLegacyStorage
            }
            GpsLegacyStorage = 0;
        }
    }
}

unsigned int GpsProvider::Release()
{
    const unsigned int remaining = --references;
    if (remaining == 0) {
        DestroyGpsProvider(this);
        void* allocation = this;
        ApiOrdinal108(&allocation);
    }
    return remaining;
}
