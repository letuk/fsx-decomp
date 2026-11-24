#pragma once

struct G2dShaderBindingPilot {
    std::string name;
    UINT bindPoint;
    UINT bindCount;
    UINT type;
    UINT constantBufferSize;
};
C_ASSERT(sizeof(G2dShaderBindingPilot) == 0x2c);

struct G2dShaderBindingEntryPilot {
    std::string key;
    G2dShaderBindingPilot binding;
};
C_ASSERT(sizeof(G2dShaderBindingEntryPilot) == 0x48);

struct G2dShaderBindingsPilot {
    DWORD count;
    DWORD capacity;
    short growth;
    WORD flags;
    G2dShaderBindingEntryPilot* entries;
};
C_ASSERT(sizeof(G2dShaderBindingsPilot) == 0x10);
C_ASSERT(offsetof(G2dShaderBindingsPilot, entries) == 0x0c);

// 206807c0. A miss yields the insertion index.
static DWORD FindG2dShaderBindingIndexPilot(const G2dShaderBindingsPilot& bindings,
                                           const std::string& key)
{
    DWORD first = 0;
    DWORD last = bindings.count;
    while (first < last) {
        const DWORD middle = first + (last - first) / 2;
        if (bindings.entries[middle].key < key) first = middle + 1;
        else last = middle;
    }
    return first;
}

// 20680590. The caller only reads the value during this lookup.
static const G2dShaderBindingPilot* FindG2dShaderBindingPilot(
    const G2dShaderBindingsPilot& bindings, const std::string& key)
{
    const DWORD index = FindG2dShaderBindingIndexPilot(bindings, key);
    if (index == bindings.count || bindings.entries[index].key != key) return NULL;
    return &bindings.entries[index].binding;
}

static void FreeG2dShaderBindingEntriesPilot(G2dShaderBindingEntryPilot* entries, DWORD count)
{
    for (DWORD i = 0; i < count; ++i) entries[i].~G2dShaderBindingEntryPilot();
    void* storage = entries;
    FreeG2dHeapSlotBoundaryPilot(&storage);
}

static const DWORD kG2dShaderBindingLimitPilot = 0x1c71c71;

// 20680880. Growth for the shader collection initialized with flag 4.
static void GrowG2dShaderBindingsPilot(G2dShaderBindingsPilot& bindings)
{
    DWORD capacity = bindings.capacity ? bindings.capacity * 2 : bindings.growth;
    if (capacity > kG2dShaderBindingLimitPilot) capacity = kG2dShaderBindingLimitPilot;
    G2dShaderBindingEntryPilot* entries = static_cast<G2dShaderBindingEntryPilot*>(
        AllocateG2dWindowStoragePilot(capacity * sizeof(*entries)));
    if (!entries) throw std::bad_alloc();
    DWORD copied = 0;
    try {
        for (; copied < bindings.count; ++copied)
            new (entries + copied) G2dShaderBindingEntryPilot(bindings.entries[copied]);
    } catch (...) {
        FreeG2dShaderBindingEntriesPilot(entries, copied);
        throw;
    }
    if (bindings.entries && (bindings.flags & 1))
        FreeG2dShaderBindingEntriesPilot(bindings.entries, bindings.count);
    bindings.entries = entries;
    bindings.capacity = capacity;
    bindings.flags |= 1;
}

// 20680710. Preserve the first value when the key is repeated.
static void InsertG2dShaderBindingPilot(G2dShaderBindingsPilot& bindings,
                                       const G2dShaderBindingPilot& binding)
{
    const DWORD index = FindG2dShaderBindingIndexPilot(bindings, binding.name);
    if (index < bindings.count && bindings.entries[index].key == binding.name) return;
    if (bindings.count == kG2dShaderBindingLimitPilot) throw std::bad_alloc();

    const G2dShaderBindingEntryPilot entry = {binding.name, binding};
    if (bindings.count == bindings.capacity) GrowG2dShaderBindingsPilot(bindings);
    G2dShaderBindingEntryPilot* entries = bindings.entries;
    new (entries + bindings.count) G2dShaderBindingEntryPilot(entry);
    for (DWORD i = bindings.count; i > index; --i) {
        G2dShaderBindingEntryPilot& right = entries[i];
        G2dShaderBindingEntryPilot& left = entries[i - 1];
        right.key.swap(left.key);
        right.binding.name.swap(left.binding.name);
        std::swap(right.binding.bindPoint, left.binding.bindPoint);
        std::swap(right.binding.bindCount, left.binding.bindCount);
        std::swap(right.binding.type, left.binding.type);
        std::swap(right.binding.constantBufferSize, left.binding.constantBufferSize);
    }
    ++bindings.count;
}

static void ClearG2dShaderBindingsPilot(G2dShaderBindingsPilot& bindings)
{
    if (!bindings.entries || !(bindings.flags & 1)) return;
    FreeG2dShaderBindingEntriesPilot(bindings.entries, bindings.count);
    bindings.entries = NULL;
    bindings.capacity = bindings.count = 0;
}
