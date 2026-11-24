#include "scheduler_object.h"

#include "scheduler_context.h"

struct SchedulerTimeObserver;

#if defined(MATCH_CONTEXT_LTCG)
extern "C" {
__declspec(dllexport) SchedulerObject schedulerObject;
__declspec(dllexport) SchedulerContext* currentSchedulerContext;
__declspec(dllexport) SchedulerTimeObserver* simConnectFallbackSetting;
}
#define MATCH_CONTEXT_EXPORT __declspec(dllexport)
#elif !defined(MATCH_PENDING_LTCG) && !defined(MATCH_ENTRY_HELPERS_LTCG)
extern "C" SchedulerObject schedulerObject;
extern "C" unsigned long schedulerDispatchKey;
extern "C" SchedulerContext* currentSchedulerContext;
extern "C" SchedulerTimeObserver* simConnectFallbackSetting;
extern "C" void __stdcall UpdateSimConnectFallback(SchedulerContext*, SchedulerTimeObserver*);
#define MATCH_CONTEXT_EXPORT

extern "C" SchedulerObject* GetSchedulerObject()
{
    return &schedulerObject;
}

#pragma optimize("y", off)
extern "C" void __stdcall SetSchedulerDispatchKey(unsigned long key)
{
    schedulerDispatchKey = key;
}
#pragma optimize("y", on)

typedef void (__stdcall *SchedulerCallback)(unsigned int message, void* payload);
extern "C" __declspec(dllimport) void __stdcall RegisterApiCallback(
    unsigned int group, SchedulerCallback callback);
extern "C" void __stdcall SchedulerGroup20Callback(unsigned int message, void* payload);
extern "C" void __stdcall SchedulerGroup10Callback(unsigned int message, void* payload);

extern "C" void RegisterSchedulerCallbacks()
{
    RegisterApiCallback(0x20, SchedulerGroup20Callback);
    RegisterApiCallback(0x10, SchedulerGroup10Callback);
}

extern "C" int GetSchedulerFieldMinimum()
{
    return schedulerObject.table->readFieldMinimum(&schedulerObject);
}

extern "C" int GetSchedulerSimSpeed()
{
    return schedulerObject.table->readSimSpeed(&schedulerObject);
}

#pragma optimize("y", off)
extern "C" void __stdcall SetSchedulerSimSpeed(int speed)
{
    schedulerObject.table->writeSimSpeed(&schedulerObject, speed);
}

extern "C" double GetSchedulerSimTime()
{
    double time;
    return *schedulerObject.table->readSimTime(&schedulerObject, &time);
}

extern "C" void __stdcall SetSimConnectFallback(SchedulerTimeObserver* setting)
{
    UpdateSimConnectFallback(currentSchedulerContext, setting);
    simConnectFallbackSetting = setting;
}
#pragma optimize("y", on)

// Minimum list-node layout used by the wrapper at 0x20cc1490.
struct SchedulerListNode
{
    virtual void reservedSlot() = 0;
    virtual void onUnregister() = 0;
    SchedulerListNode* next;
};

extern "C" SchedulerListNode* schedulerListHead;
extern "C" __declspec(dllimport) void __stdcall UnregisterApiCallback(
    unsigned int group, SchedulerCallback callback);

extern "C" void UnregisterSchedulerCallbacks()
{
    UnregisterApiCallback(0x10, SchedulerGroup10Callback);
    UnregisterApiCallback(0x20, SchedulerGroup20Callback);
    for (SchedulerListNode* node = schedulerListHead; node; node = node->next) {
        node->onUnregister();
    }
}

#pragma pack(push, 4)
struct SchedulerTimeInput
{
    unsigned char unknown[0x64];
    double timeStep;
};
#pragma pack(pop)

extern "C" __declspec(dllimport) int CanSkipSchedulerUpdate();
extern "C" __declspec(dllimport) const SchedulerTimeInput* GetSchedulerTimeInput();

extern "C" void __stdcall SchedulerGroup20Callback(unsigned int, void*)
{
    if (CanSkipSchedulerUpdate()) {
        return;
    }
    const SchedulerTimeInput* input = GetSchedulerTimeInput();
    if (input) {
        schedulerObject.table->advanceTime(&schedulerObject, input->timeStep);
    }
}

// Ordinary C++ declarations for the rebuilt handlers. The original load and
// time-state handlers use compiler-assigned register arguments.
extern "C" void LoadSchedulerFlightState(void* payload);
extern "C" void UpdateSchedulerTimeState(SchedulerObject* object, int active);
extern "C" void __stdcall SaveSchedulerFlightState(void* payload);

#pragma optimize("y", off)
extern "C" void __stdcall SchedulerGroup10Callback(unsigned int event, void* payload)
{
    switch (event) {
    case 0x25:
        LoadSchedulerFlightState(payload);
        break;
    case 0x26:
        SaveSchedulerFlightState(payload);
        break;
    case 0x171:
        UpdateSchedulerTimeState(&schedulerObject, payload != 0);
        break;
    }
}
#pragma optimize("y", on)
#endif

struct SchedulerTimeSourceObject;

struct SchedulerTimeSourceTable
{
    unsigned char precedingSlots[0x4c];
    SchedulerContext* (__stdcall *getContext)(SchedulerTimeSourceObject*);
};

struct SchedulerTimeSourceObject
{
    const SchedulerTimeSourceTable* table;
};

struct SchedulerTimeSource
{
    unsigned char unknown[0x20];
    SchedulerTimeSourceObject* source;
};

struct SchedulerTimeObserverTable
{
    void (__stdcall *notify)(SchedulerTimeObserver*, double);
};

struct SchedulerTimeObserver
{
    const SchedulerTimeObserverTable* table;
};

struct SchedulerApiState
{
    unsigned char unknown[0x110];
    int timeUpdateSuppressed;
};

extern "C" __declspec(dllimport) SchedulerApiState schedulerApiState;
extern "C" __declspec(dllimport) void __stdcall DispatchSchedulerApiEvent(
    unsigned int event, unsigned int code, void* payload);

#if !defined(MATCH_PENDING_LTCG) && !defined(MATCH_ENTRY_HELPERS_LTCG)
#if defined(MATCH_CONTEXT_FRAME)
#pragma optimize("y", off)
#endif
static __declspec(noinline) void __stdcall SplitSchedulerStep(
    SchedulerContext* context, double requestedStep)
{
    context->substep = requestedStep;
    if (requestedStep > context->maximumStep) {
        context->substepCount = static_cast<int>(requestedStep / context->maximumStep) + 1;
        context->substep = requestedStep / context->substepCount;
        if (context->substep < context->minimumStep) {
            context->substepCount = static_cast<int>(requestedStep / context->minimumStep);
            context->substep = context->minimumStep;
        }
    } else if (requestedStep < context->minimumStep) {
        context->substepCount = 0;
        context->substep = 0.0;
    } else {
        context->substepCount = 1;
    }
}
#if defined(MATCH_CONTEXT_FRAME)
#pragma optimize("y", on)
#endif

static double ReadSchedulerTime()
{
    double buffer;
    return *schedulerObject.table->readSimTime(&schedulerObject, &buffer);
}

// Explicit arguments for the rebuilt implementation. The original passes
// the context in EAX and the remaining inputs on the stack.
#if defined(MATCH_CONTEXT_LTCG)
#if defined(MATCH_CONTEXT_FRAME)
#pragma optimize("y", off)
#endif
static __declspec(noinline) int __stdcall ProcessSchedulerContext(
#else
extern "C" int __stdcall ProcessSchedulerContext(
#endif
    SchedulerContext* context, int suppressed, int timeStateActive, double time)
{
    if (!(context->flags & 1) || !context->liveEntries.count) {
        context->time = time;
        return 1;
    }
    if ((suppressed && (context->mode & 3) != 3) ||
        (timeStateActive && (context->mode & 5) != 5)) {
        return 0;
    }

    context->flags |= 2;
    if (context->mode & 1) {
        if (suppressed) {
            context->time = ReadSchedulerTime();
        }
        double delta = time - context->time;
        if (suppressed) {
            delta += context->remainder;
        }
        SplitSchedulerStep(context, delta);
        const double covered = context->substepCount * context->substep;
        if (suppressed) {
            context->remainder = delta - covered;
        } else {
            context->time += covered;
            context->remainder = 0.0;
        }
        for (int step = 1; step <= context->substepCount; ++step) {
            const double entryTime = step < context->substepCount
                ? context->time + step * context->substep : context->time;
            for (int i = 0; i < context->liveEntries.count; ++i) {
                SchedulerTimedEntry* entry = context->liveEntries.records[i];
                if (!(entry->flags & 2)) {
                    SchedulerEntryCallback* callback = entry->callback;
                    callback->table->advance(callback, context->substep);
                    if (!suppressed) {
                        entry->time = entryTime;
                    }
                }
            }
        }
    } else {
        double elapsed = time - context->time;
        if (elapsed <= 0.0) {
            elapsed = 0.0;
        }
        int entriesToVisit = context->liveEntries.count;
        if (elapsed < context->minimumStep) {
            entriesToVisit = static_cast<int>(entriesToVisit * elapsed / context->minimumStep);
        }
        if (entriesToVisit > 0) {
            double maximumRemainder = 0.0;
            do {
                SchedulerTimedEntry* entry = context->liveEntries.records[context->cursor];
                const double current = suppressed ? ReadSchedulerTime() : entry->time;
                const double target = suppressed ? time + context->remainder : time;
                if (current < target) {
                    const double delta = target - current;
                    SplitSchedulerStep(context, delta);
                    const double remainder = suppressed
                        ? delta - context->substepCount * context->substep : 0.0;
                    for (int step = 1; step <= context->substepCount; ++step) {
                        if (!(entry->flags & 2)) {
                            SchedulerEntryCallback* callback = entry->callback;
                            callback->table->advance(callback, context->substep);
                            if (suppressed) {
                                double buffer;
                                schedulerObject.table->readSimTime(&schedulerObject, &buffer);
                                entry->time = buffer;
                            } else {
                                entry->time += context->substep;
                            }
                        }
                    }
                    if (remainder > maximumRemainder) {
                        maximumRemainder = remainder;
                    }
                }
                context->cursor = (context->cursor + 1) % context->liveEntries.count;
            } while (--entriesToVisit);
            if (suppressed) {
                context->remainder = maximumRemainder;
            } else {
                context->time = time;
                context->remainder = 0.0;
            }
        }
    }

    context->flags &= ~2u;
    for (int i = 0; i < context->pending.count; ++i) {
        const SchedulerPendingEntry& pending = context->pending.records[i];
        if (pending.add) {
#if defined(MATCH_NATIVE_CONTEXT)
            context->RegisterEntry(pending.entry, pending.flags);
#else
            context->table->addPendingEntry(context, pending.entry, pending.flags);
#endif
        } else {
#if defined(MATCH_NATIVE_CONTEXT)
            context->UnregisterEntry(pending.entry);
#else
            context->table->removePendingEntry(context, pending.entry);
#endif
        }
    }
    context->pending.count = 0;
    return 1;
}
#if defined(MATCH_CONTEXT_FRAME)
#pragma optimize("y", on)
#endif
#endif

#if !defined(MATCH_CONTEXT_LTCG)
#include <string.h>

extern "C" __declspec(dllimport) void __stdcall AllocatePendingStorage(
    int mode, unsigned int bytes, SchedulerPendingEntry** result, void* allocator);
extern "C" __declspec(dllimport) void __stdcall ReleasePendingStorage(SchedulerPendingEntry** buffer);
extern "C" __declspec(dllimport) void __stdcall MovePendingStorage(
    void* destination, const void* source, unsigned int bytes);

#if !defined(MATCH_ENTRY_HELPERS_LTCG)
#if defined(MATCH_PENDING_LTCG)
extern "C" {
__declspec(dllexport) void* schedulerPendingAllocator;
}
#else
extern "C" void* schedulerPendingAllocator;
#endif

#if defined(MATCH_PENDING_FRAME)
#pragma optimize("y", off)
#endif
static __declspec(noinline) int ResizePendingSchedulerEntries(SchedulerPendingQueue* queue, int requested)
{
    const int maximumRecords = 0x7ffffff;
    if (requested > maximumRecords) {
        return 0;
    }

    int capacity = 0;
    if (requested > 0) {
        if (queue->growth <= 0) {
            queue->growth = 5;
        }
        if (queue->flags & 4) {
            capacity = queue->growth;
            while (capacity < requested) {
                capacity *= 2;
            }
        } else {
            capacity = ((requested + queue->growth - 1) / queue->growth) * queue->growth;
        }
    }
    if (capacity == queue->capacity) {
        return 1;
    }

    SchedulerPendingEntry* replacement = 0;
    if (capacity > 0) {
        AllocatePendingStorage(1, capacity * sizeof(*replacement), &replacement, schedulerPendingAllocator);
        if (!replacement) {
            return 0;
        }
    }
    const int count = requested < queue->count ? requested : queue->count;
    if (count > 0) {
        memcpy(replacement, queue->records, count * sizeof(*replacement));
    }
    SchedulerPendingEntry* previous = queue->records;
    queue->count = count;
    ReleasePendingStorage(&previous);
    queue->capacity = capacity;
    queue->flags |= 1;
    queue->records = replacement;
    return 1;
}

#if defined(MATCH_PENDING_LTCG)
static __declspec(noinline) int __stdcall QueueSchedulerRegistration(
#else
extern "C" int __stdcall QueueSchedulerRegistration(
#endif
    SchedulerContext* context, void* entry, int classification, int flags, int add)
{
    SchedulerPendingQueue& queue = context->pending;
    for (int i = 0; i < queue.count; ++i) {
        if (queue.records[i].entry == entry) {
            if (queue.records[i].add == add) {
                return 0;
            }
            MovePendingStorage(queue.records + i, queue.records + i + 1,
                (queue.count - i - 1) * sizeof(*queue.records));
            --queue.count;
            return 1;
        }
    }

    const int maximumRecords = 0x7ffffff;
    if (queue.count < 0 || queue.count >= maximumRecords) {
        return 0;
    }
    const int newCount = queue.count + 1;
    if (newCount > queue.capacity && !ResizePendingSchedulerEntries(&queue, newCount)) {
        return 0;
    }
    SchedulerPendingEntry& pending = queue.records[queue.count];
    pending.add = add;
    pending.entry = entry;
    pending.flags = flags;
    pending.classification = classification;
    queue.count = newCount;
    return 1;
}
#if defined(MATCH_PENDING_FRAME)
#pragma optimize("y", on)
#endif
#endif

#if !defined(MATCH_CONTEXT_LTCG) && !defined(MATCH_PENDING_LTCG)
#if !defined(MATCH_ENTRY_HELPERS_LTCG)
extern "C" SchedulerTimedEntry* AcquireSchedulerEntry();
extern "C" void __stdcall AppendSchedulerEntry(SchedulerLiveEntryList*, SchedulerTimedEntry* const*);
extern "C" void __stdcall InsertSchedulerEntry(SchedulerLiveEntryList*, int, SchedulerTimedEntry* const*);
extern "C" int __stdcall InsertSchedulerEntryIndex(
    SchedulerEntryIndex*, void* const*, SchedulerTimedEntry* const*);
extern "C" void ReleaseSchedulerEntry(SchedulerTimedEntry*);
#endif

#if defined(MATCH_ENTRY_HELPERS_LTCG)
#pragma optimize("y", off)
#endif
static __declspec(noinline) bool FindSchedulerEntry(
    const SchedulerEntryIndex* index, void* const* key, SchedulerEntryIndexRecord** result)
{
    unsigned int first = 0;
    unsigned int last = index->count;
    while (first < last) {
        const unsigned int middle = first + (last - first) / 2;
        if (reinterpret_cast<unsigned long>(index->records[middle].key) <
            reinterpret_cast<unsigned long>(*key)) {
            first = middle + 1;
        } else {
            last = middle;
        }
    }
    SchedulerEntryIndexRecord* candidate = index->records + first;
    if (result) {
        *result = candidate;
    }
    return first < index->count && candidate->key == *key;
}

#if !defined(MATCH_INSERTION_LTCG)
// Original: simscheduler.dll 0x20cc3b70.
static __declspec(noinline) int RemoveSchedulerEntryReferences(
    SchedulerLiveEntryList* live, SchedulerTimedEntry* const* entry)
{
    int removed = 0;
    int i = live->count - 1;
    if (i >= 0) {
        SchedulerTimedEntry* const target = *entry;
        do {
            SchedulerTimedEntry** slot = live->records + i;
            if (*slot == target) {
                MovePendingStorage(slot, slot + 1, (live->count - i - 1) * sizeof(*slot));
                --live->count;
                ++removed;
            }
        } while (--i >= 0);
    }
    return removed;
}

static __declspec(noinline) int RemoveSchedulerIndexRecord(
    SchedulerEntryIndex* index, void* const* key)
{
    SchedulerEntryIndexRecord* record;
    if (!index->count || !FindSchedulerEntry(index, key, &record)) {
        return 0;
    }
    const unsigned int position = static_cast<unsigned int>(record - index->records);
    MovePendingStorage(record, record + 1, (index->count - position - 1) * sizeof(*record));
    --index->count;
    return 1;
}
#endif

#if defined(MATCH_ENTRY_HELPERS_LTCG)
#pragma optimize("y", on)
#if !defined(MATCH_INSERTION_LTCG)
extern "C" __declspec(dllexport) bool __stdcall ProbeSchedulerEntryLookup(
    const SchedulerEntryIndex* index, void* const* key, SchedulerEntryIndexRecord** result)
{
    return FindSchedulerEntry(index, key, result);
}

extern "C" __declspec(dllexport) int __stdcall ProbeSchedulerEntryRemoval(
    SchedulerLiveEntryList* live, SchedulerTimedEntry* const* entry)
{
    return RemoveSchedulerEntryReferences(live, entry);
}

extern "C" __declspec(dllexport) int __stdcall ProbeSchedulerIndexRemoval(
    SchedulerEntryIndex* index, void* const* key)
{
    return RemoveSchedulerIndexRecord(index, key);
}
#endif
#endif

#if !defined(MATCH_ENTRY_HELPERS_LTCG) || defined(MATCH_INSERTION_LTCG)
#if defined(MATCH_INSERTION_LTCG)
#define INSERTION_LINKAGE static __declspec(noinline)
extern "C" {
__declspec(dllexport) void* schedulerPendingAllocator;
}
#else
#define INSERTION_LINKAGE extern "C" __declspec(noinline)
#endif
#pragma optimize("y", off)
extern "C" __declspec(dllimport) void __stdcall AllocateSchedulerContainerStorage(
    int mode, unsigned int bytes, void** result, void* allocator);
extern "C" __declspec(dllimport) void __stdcall ReleaseSchedulerContainerStorage(void** buffer);

template <class Element, class Count>
static __forceinline int ResizeSchedulerContainer(
    Element*& records, int count, int& capacity, short& growth,
    unsigned short& flags, int requested, int maximum, Count& retained)
{
    if (requested > maximum) {
        return 0;
    }
    int newCapacity = 0;
    if (requested > 0) {
        if (growth <= 0) {
            growth = 5;
        }
        if (flags & 4) {
            newCapacity = growth;
            while (newCapacity < requested) {
                newCapacity *= 2;
            }
        } else {
            newCapacity = ((requested + growth - 1) / growth) * growth;
        }
    }
    if (newCapacity == capacity) {
        retained = count;
        return 1;
    }

    void* replacement = 0;
    if (newCapacity > 0) {
        AllocateSchedulerContainerStorage(1, newCapacity * sizeof(Element), &replacement, schedulerPendingAllocator);
        if (!replacement) {
            return 0;
        }
    }
    const int retainedCount = requested < count ? requested : count;
    if (retainedCount > 0) {
        memcpy(replacement, records, retainedCount * sizeof(Element));
    }
    void* previous = records;
    retained = retainedCount;
    ReleaseSchedulerContainerStorage(&previous);
    capacity = newCapacity;
    flags |= 1;
    records = static_cast<Element*>(replacement);
    return 1;
}

INSERTION_LINKAGE int ResizeLiveSchedulerEntries(SchedulerLiveEntryList* live, int requested)
{
    return ResizeSchedulerContainer(live->records, live->count, live->capacity,
        live->growth, live->flags, requested, 0x1fffffff, live->count);
}

INSERTION_LINKAGE int ResizeSchedulerEntryIndex(SchedulerEntryIndex* index, int requested)
{
    return ResizeSchedulerContainer(index->records, index->count, index->capacity,
        index->growth, index->flags, requested, 0x0fffffff, index->count);
}

INSERTION_LINKAGE void __stdcall AppendSchedulerEntry(SchedulerLiveEntryList* live, SchedulerTimedEntry* const* entry)
{
    if (live->count >= 0x1fffffff) {
        return;
    }
    const int expandedCount = live->count + 1;
    if (expandedCount > live->capacity && !ResizeLiveSchedulerEntries(live, expandedCount)) {
        return;
    }
    live->records[live->count++] = *entry;
}

INSERTION_LINKAGE void __stdcall InsertSchedulerEntry(
    SchedulerLiveEntryList* live, int position, SchedulerTimedEntry* const* entry)
{
    if (live->count >= 0x1fffffff) {
        return;
    }
    const int expandedCount = live->count + 1;
    if (expandedCount > live->capacity && !ResizeLiveSchedulerEntries(live, expandedCount)) {
        return;
    }
    SchedulerTimedEntry** slot = live->records + position;
    MovePendingStorage(slot + 1, slot, (live->count - position) * sizeof(*slot));
    ++live->count;
    *slot = *entry;
}

INSERTION_LINKAGE SchedulerEntryIndexRecord* AppendSchedulerIndexSlot(SchedulerEntryIndex* index)
{
    if (index->count >= 0x0fffffff) {
        return 0;
    }
    const int expandedCount = index->count + 1;
    if (expandedCount > index->capacity && !ResizeSchedulerEntryIndex(index, expandedCount)) {
        return 0;
    }
    return index->records + index->count++;
}

INSERTION_LINKAGE SchedulerEntryIndexRecord* InsertSchedulerIndexSlot(
    SchedulerEntryIndex* index, unsigned int position)
{
    if (index->count >= 0x0fffffff) {
        return 0;
    }
    const int expandedCount = index->count + 1;
    if (expandedCount > index->capacity && !ResizeSchedulerEntryIndex(index, expandedCount)) {
        return 0;
    }
    SchedulerEntryIndexRecord* slot = index->records + position;
    MovePendingStorage(slot + 1, slot, (index->count - position) * sizeof(*slot));
    ++index->count;
    return slot;
}

INSERTION_LINKAGE int __stdcall InsertSchedulerEntryIndex(
    SchedulerEntryIndex* index, void* const* key, SchedulerTimedEntry* const* entry)
{
    SchedulerEntryIndexRecord* slot;
    if (!index->count) {
        slot = AppendSchedulerIndexSlot(index);
    } else {
        SchedulerEntryIndexRecord* candidate;
        if (FindSchedulerEntry(index, key, &candidate)) {
            return 0;
        }
        slot = InsertSchedulerIndexSlot(index, static_cast<unsigned int>(candidate - index->records));
    }
    if (!slot) {
        return 0;
    }
    slot->key = *key;
    slot->entry = *entry;
    return 1;
}

#if defined(MATCH_INSERTION_LTCG)
extern "C" __declspec(dllexport) void __stdcall ProbeAppendSchedulerEntry(
    SchedulerLiveEntryList* live, SchedulerTimedEntry* const* entry)
{
    AppendSchedulerEntry(live, entry);
}

extern "C" __declspec(dllexport) void __stdcall ProbeInsertSchedulerEntry(
    SchedulerLiveEntryList* live, int position, SchedulerTimedEntry* const* entry)
{
    InsertSchedulerEntry(live, position, entry);
}

extern "C" __declspec(dllexport) int __stdcall ProbeInsertSchedulerIndex(
    SchedulerEntryIndex* index, void* const* key, SchedulerTimedEntry* const* entry)
{
    return InsertSchedulerEntryIndex(index, key, entry);
}

#else
#if !defined(MATCH_INSERTION_LTCG)
extern "C" const SchedulerContextTable schedulerContextTable;
extern "C" __declspec(dllimport) void __stdcall CopySchedulerContextName(
    char* destination, const char* source, unsigned int count);
extern "C" void ReleaseSchedulerContextStorage(SchedulerContext*);

// Reconstruction helper for simscheduler.dll 0x20cc28f0.
template <class Element, class Count>
static __forceinline void DestroySchedulerContainer(
    Element*& records, Count& count, int& capacity, unsigned short& storageFlags)
{
    if (records && (storageFlags & 1) && capacity) {
        void* previous = records;
        Count remaining = count;
        if (static_cast<int>(remaining) > 0) {
            remaining = 0;
        }
        count = remaining;
        ReleaseSchedulerContainerStorage(&previous);
        storageFlags |= 1;
        records = 0;
        capacity = 0;
    }
}

inline SchedulerPendingQueue::~SchedulerPendingQueue()
{
    DestroySchedulerContainer(records, count, capacity, flags);
}

extern "C" void ClearContextPendingStorage(SchedulerPendingQueue* queue)
{
    DestroySchedulerContainer(queue->records, queue->count, queue->capacity, queue->flags);
}

inline SchedulerEntryIndex::~SchedulerEntryIndex()
{
    DestroySchedulerContainer(records, count, capacity, flags);
}

inline SchedulerLiveEntryList::~SchedulerLiveEntryList()
{
    DestroySchedulerContainer(records, count, capacity, flags);
}

// simscheduler.dll 0x20cc15d0.
extern "C" __declspec(noinline) double* __stdcall InitializeContextTimeValue(double* destination, double value)
{
    __declspec(align(8)) const double seconds = value;
    *destination = seconds;
    return destination;
}

#include <new>
// simscheduler.dll 0x20cc27a0. The standard pilot inlines this into its wrapper.
#if defined(MATCH_LIFECYCLE_FACTORY)
__forceinline
#endif
SchedulerContext::SchedulerContext(const char* contextName)
#if defined(MATCH_NATIVE_CONTEXT)
    : referenceCount(0), time(0.0), remainder(0.0)
#else
    : table(&schedulerContextTable), referenceCount(0), time(0.0), remainder(0.0)
#endif
{
    memset(name, 0, sizeof(unsigned int));
    CopySchedulerContextName(name, contextName, sizeof(name));
    minimumStep = 1.0;
    modeBits.active = 0;
    maximumStep = 1.0;
    updateBits.active = 0;
    cursor = -1;
    substep = 0.0;
    substepCount = 0;
    double timeBuffer;
    const double* baseline = schedulerObject.table->readSimTime(&schedulerObject, &timeBuffer);
    memcpy(&time, baseline, sizeof(time));
    double remainderBuffer;
    const double* initialRemainder = InitializeContextTimeValue(&remainderBuffer, 0.0);
    memcpy(&remainder, initialRemainder, sizeof(remainder));
}

// simscheduler.dll 0x20cc27a0 (FUN_20cc27a0).
extern "C" SchedulerContext* __stdcall ConstructSchedulerContext(
    SchedulerContext* storage, const char* contextName)
{
    __assume(storage != 0);
    return ::new (storage) SchedulerContext(contextName);
}

__declspec(noinline) SchedulerContext::~SchedulerContext()
{
#if !defined(MATCH_NATIVE_CONTEXT)
    table = &schedulerContextTable;
#endif
    if (currentSchedulerContext == this) {
        UpdateSimConnectFallback(0, simConnectFallbackSetting);
        currentSchedulerContext = 0;
    }
}

#if !defined(MATCH_NATIVE_CONTEXT)
SchedulerContext* SchedulerContext::destroy(unsigned int deleteFlags)
{
    this->~SchedulerContext();
    if (deleteFlags & 1) {
        ReleaseSchedulerContextStorage(this);
    }
    return this;
}
#endif
#endif

#if defined(MATCH_NATIVE_CONTEXT)
int SchedulerContext::RegisterEntry(void* key, int flags)
{
    SchedulerContext* const context = this;
#else
extern "C" int __stdcall RegisterSchedulerEntry(SchedulerContext* context, void* key, int flags)
{
#endif
    const int classification = reinterpret_cast<unsigned long>(key) == schedulerDispatchKey;
    if (context->flags & 2) {
        return QueueSchedulerRegistration(context, key, classification, flags, 1);
    }

    SchedulerEntryIndexRecord* existing;
    if (context->index.count && FindSchedulerEntry(&context->index, &key, &existing) &&
        reinterpret_cast<unsigned long>(existing) != 0xfffffffcUL) {
        return 0;
    }

    SchedulerTimedEntry* entry = AcquireSchedulerEntry();
    if (entry) {
        entry->time = context->time;
        entry->callback = static_cast<SchedulerEntryCallback*>(key);
        entry->flags = (entry->flags & ~0x0eu) | ((classification * 2 + (flags & 1)) << 2) | 1;
    }

    UpdateSimConnectFallback(context, simConnectFallbackSetting);
    currentSchedulerContext = context;
    if (!context->liveEntries.count) {
        AppendSchedulerEntry(&context->liveEntries, &entry);
        context->cursor = 0;
    } else if (flags && (context->mode & 1)) {
        AppendSchedulerEntry(&context->liveEntries, &entry);
    } else {
        InsertSchedulerEntry(&context->liveEntries, context->cursor, &entry);
        ++context->cursor;
    }
    InsertSchedulerEntryIndex(&context->index, &key, &entry);
    return 1;
}

#if defined(MATCH_NATIVE_CONTEXT)
int SchedulerContext::UnregisterEntry(void* key)
{
    SchedulerContext* const context = this;
#else
extern "C" int __stdcall UnregisterSchedulerEntry(SchedulerContext* context, void* key)
{
#endif
    SchedulerEntryIndexRecord* found;
    if (!context->index.count || !FindSchedulerEntry(&context->index, &key, &found)) {
        SchedulerPendingQueue& queue = context->pending;
        for (int i = 0; i < queue.count; ++i) {
            if (queue.records[i].entry == key && queue.records[i].add) {
                MovePendingStorage(queue.records + i, queue.records + i + 1,
                    (queue.count - i - 1) * sizeof(*queue.records));
                --queue.count;
                return 1;
            }
        }
        return 0;
    }

    SchedulerTimedEntry* entry = found->entry;
    if (context->flags & 2) {
        QueueSchedulerRegistration(context, key, 0, 0, 0);
        entry->flags |= 2;
        return 1;
    }
    if (!RemoveSchedulerEntryReferences(&context->liveEntries, &entry)) {
        return 0;
    }
    if (!context->liveEntries.count) {
        context->cursor = -1;
    } else if (context->cursor >= context->liveEntries.count) {
        context->cursor = 0;
    }
    RemoveSchedulerIndexRecord(&context->index, &key);
    ReleaseSchedulerEntry(entry);
    return 1;
}

struct SchedulerPoolLock;
struct SchedulerPoolFreeList;
struct SchedulerPoolRegistration;
struct SchedulerPoolTable
{
    void* deletingDestructor;
    void* drain;
    void* reserved;
    unsigned int (__fastcall *countUsed)(SchedulerPoolRegistration*);
    unsigned int (__fastcall *countAvailable)(SchedulerPoolRegistration*);
    unsigned int (__fastcall *countCapacity)(SchedulerPoolRegistration*);
};

struct SchedulerPoolRegistration
{
    SchedulerPoolRegistration();
    ~SchedulerPoolRegistration();
    SchedulerPoolRegistration* destroy(unsigned int flags);

    const SchedulerPoolTable* table;
    SchedulerPoolRegistration* next;
};

template <class Entry, int Capacity>
struct SchedulerPoolBlock
{
    typedef Entry Value;
    enum { entryCount = Capacity };
    Entry entries[Capacity];
    long freeCount;
    SchedulerPoolFreeList* freeList;
    SchedulerPoolBlock* next;
};

template <class Block>
struct SchedulerPool
{
    explicit SchedulerPool(const SchedulerPoolTable*);
    SchedulerPool* destroy(unsigned int flags);

    SchedulerPoolRegistration registration;
    Block* blocks;
    SchedulerPoolLock* lock;
    unsigned int blockCount;
    unsigned int peakBlockCount;
};

union SchedulerContextStorage
{
    unsigned char bytes[0x90];
    unsigned int alignment;
};

typedef SchedulerPoolBlock<SchedulerTimedEntry, 128> SchedulerEntryPoolBlock;
typedef SchedulerPoolBlock<SchedulerContextStorage, 16> SchedulerContextPoolBlock;
typedef SchedulerPool<SchedulerEntryPoolBlock> SchedulerEntryPool;
typedef SchedulerPool<SchedulerContextPoolBlock> SchedulerContextPool;
typedef char SchedulerContextSizeCheck[sizeof(SchedulerContext) == 0x90 ? 1 : -1];
typedef char SchedulerContextBlockSizeCheck[sizeof(SchedulerContextPoolBlock) == 0x90c ? 1 : -1];

extern "C" SchedulerEntryPool* schedulerEntryPool;
extern "C" SchedulerContextPool* schedulerContextPool;
extern "C" __declspec(dllimport) void __stdcall AllocateSchedulerPoolStorage(
    int mode, unsigned int bytes, void** result, void* allocator);
extern "C" __declspec(dllimport) void __stdcall ReleaseSchedulerPoolStorage(void** block);
extern "C" __declspec(dllimport) long __stdcall DecrementSchedulerPoolCount(long* count);
extern "C" __declspec(dllimport) long __stdcall IncrementSchedulerPoolCount(long* count);
extern "C" __declspec(dllimport) void __stdcall BreakOnSchedulerPoolFailure();

// The rebuilt declarations make the lock and free-list register arguments
// explicit. Their original entry points use internal compiler-assigned ABIs.
extern "C" unsigned int __stdcall AcquireSchedulerPoolLock(
    SchedulerPoolLock*, int, int, void*, int, unsigned int, int);
extern "C" unsigned int __stdcall ReleaseSchedulerPoolLock(SchedulerPoolLock*, int);
extern "C" SchedulerPoolFreeList* __stdcall CreateSchedulerPoolFreeList(int, int);
extern "C" int DestroySchedulerPoolFreeList(SchedulerPoolFreeList*);
extern "C" void* PopSchedulerPoolEntry(SchedulerPoolFreeList*);
extern "C" int __stdcall PushSchedulerPoolEntry(SchedulerPoolFreeList*, void*);

template <class Block>
static __forceinline int InitializeSchedulerStorageBlock(Block* block)
{
    const int capacity = sizeof(block->entries) / sizeof(block->entries[0]);
    if (!block->freeList) {
        block->freeList = CreateSchedulerPoolFreeList(capacity, capacity);
        if (!block->freeList) {
            BreakOnSchedulerPoolFailure();
            return 0;
        }
    } else {
        while (PopSchedulerPoolEntry(block->freeList)) {
        }
    }
    for (int i = 0; i < capacity; ++i) {
        if (!PushSchedulerPoolEntry(block->freeList, block->entries + i)) {
            return 0;
        }
    }
    block->freeCount = capacity;
    return 1;
}

extern "C" int __stdcall InitializeSchedulerPoolBlock(SchedulerEntryPoolBlock* block)
{
    return InitializeSchedulerStorageBlock(block);
}

extern "C" int __stdcall InitializeSchedulerContextBlock(SchedulerContextPoolBlock* block)
{
    return InitializeSchedulerStorageBlock(block);
}

template <class Block, int (__stdcall *Initialize)(Block*)>
static __forceinline void* AcquireSchedulerPoolStorage(SchedulerPool<Block>* pool)
{
    int exclusive = 0;
    void* entry = 0;
    for (;;) {
        if (!pool->lock || AcquireSchedulerPoolLock(pool->lock, exclusive, 0, 0, 0, ~0u, 0)) {
            return 0;
        }
        for (Block* block = pool->blocks; block; block = block->next) {
            if (block->freeList) {
                entry = PopSchedulerPoolEntry(block->freeList);
                if (entry) {
                    DecrementSchedulerPoolCount(&block->freeCount);
                    break;
                }
            }
        }
        if (entry) {
            break;
        }

        void* storage;
        AllocateSchedulerPoolStorage(1, sizeof(Block), &storage, schedulerPendingAllocator);
        Block* block = static_cast<Block*>(storage);
        if (!block) {
            break;
        }
        block->freeCount = sizeof(block->entries) / sizeof(block->entries[0]);
        block->freeList = 0;
        block->next = 0;
        if (!Initialize(block)) {
            break;
        }

        if (!exclusive) {
            exclusive = 1;
            if (!pool->lock || AcquireSchedulerPoolLock(pool->lock, 3, 0, 0, 0, ~0u, 0)) {
                if (pool->lock) {
                    ReleaseSchedulerPoolLock(pool->lock, 0);
                }
                if (block->freeList) {
                    DestroySchedulerPoolFreeList(block->freeList);
                    block->freeList = 0;
                }
                ReleaseSchedulerPoolStorage(&storage);
                continue;
            }
        }

        block->next = pool->blocks;
        ++pool->blockCount;
        pool->blocks = block;
        if (pool->blockCount > pool->peakBlockCount) {
            pool->peakBlockCount = pool->blockCount;
        }
        entry = PopSchedulerPoolEntry(block->freeList);
        if (entry) {
            --block->freeCount;
        }
        break;
    }
    if (pool->lock) {
        ReleaseSchedulerPoolLock(pool->lock, exclusive);
    }
    return entry;
}

template <class Block>
static __forceinline void ReturnSchedulerPoolStorage(SchedulerPool<Block>* pool, void* entry)
{
    if (!pool->lock || AcquireSchedulerPoolLock(pool->lock, 0, 0, 0, 0, ~0u, 0)) {
        return;
    }
    const unsigned long address = reinterpret_cast<unsigned long>(entry);
    for (Block* block = pool->blocks; block; block = block->next) {
        const unsigned long start = reinterpret_cast<unsigned long>(block);
        if (start <= address && address < start + sizeof(block->entries)) {
            if (block->freeList && PushSchedulerPoolEntry(block->freeList, entry)) {
                IncrementSchedulerPoolCount(&block->freeCount);
            }
            break;
        }
    }
    if (pool->lock) {
        ReleaseSchedulerPoolLock(pool->lock, 0);
    }
}

extern "C" SchedulerTimedEntry* AcquireSchedulerEntry()
{
    return static_cast<SchedulerTimedEntry*>(
        AcquireSchedulerPoolStorage<SchedulerEntryPoolBlock, InitializeSchedulerPoolBlock>(schedulerEntryPool));
}

extern "C" void ReleaseSchedulerEntry(SchedulerTimedEntry* entry)
{
    ReturnSchedulerPoolStorage(schedulerEntryPool, entry);
}

extern "C" SchedulerContext* AcquireSchedulerContextStorage()
{
    return static_cast<SchedulerContext*>(
        AcquireSchedulerPoolStorage<SchedulerContextPoolBlock, InitializeSchedulerContextBlock>(schedulerContextPool));
}

extern "C" void ReleaseSchedulerContextStorage(SchedulerContext* context)
{
    ReturnSchedulerPoolStorage(schedulerContextPool, context);
}

extern "C" long __stdcall AddSchedulerContextReference(SchedulerContext* context)
{
    return IncrementSchedulerPoolCount(&context->referenceCount);
}

extern "C" long __stdcall ReleaseSchedulerContextReference(SchedulerContext* context)
{
    const long count = DecrementSchedulerPoolCount(&context->referenceCount);
    if (!count && context) {
#if defined(MATCH_NATIVE_CONTEXT)
        delete context;
#else
        context->table->deleteContext(context, 1);
#endif
    }
    return count;
}

struct SchedulerNameRecord
{
    char key[32];
    SchedulerContext* context;
};

struct SchedulerNameIndex
{
    int count;
    int capacity;
    short growth;
    unsigned short flags;
    SchedulerNameRecord* records;
};

struct SchedulerContextRegistry;
struct SchedulerContextRegistryTable
{
    void* firstSlot;
    SchedulerContext** (__stdcall *find)(SchedulerContextRegistry*, SchedulerContext**, const char*);
};

struct SchedulerContextRegistry
{
    const SchedulerContextRegistryTable* table;
    unsigned char unknown04[0x2c];
    SchedulerNameIndex nameIndex;
};

extern "C" __declspec(dllimport) unsigned int __stdcall GetSchedulerContextNameLength(const char*);
extern "C" __declspec(dllimport) int __stdcall CompareSchedulerContextNames(const char*, const char*);

extern "C" bool __stdcall FindSchedulerName(
    const SchedulerNameIndex* index, const char* key, SchedulerNameRecord** position)
{
    unsigned int first = 0;
    unsigned int last = index->count;
    while (first < last) {
        const unsigned int middle = first + (last - first) / 2;
        const char* candidate = index->records[middle].key;
        if (CompareSchedulerContextNames(key, candidate) >= 0
            && CompareSchedulerContextNames(key, candidate) > 0) {
            first = middle + 1;
        } else {
            last = middle;
        }
    }
    SchedulerNameRecord* candidate = index->records + first;
    if (position) {
        *position = candidate;
    }
    return first < static_cast<unsigned int>(index->count)
        && CompareSchedulerContextNames(key, candidate->key) >= 0
        && CompareSchedulerContextNames(key, candidate->key) <= 0;
}

static __forceinline void AssignSchedulerContext(SchedulerContext** slot, SchedulerContext* context)
{
    if (context) {
#if defined(MATCH_NATIVE_CONTEXT)
        context->AddReference();
#else
        context->table->addReference(context);
#endif
    }
    if (*slot) {
#if defined(MATCH_NATIVE_CONTEXT)
        (*slot)->ReleaseReference();
#else
        (*slot)->table->releaseReference(*slot);
#endif
    }
    *slot = context;
}

static __forceinline void ReleaseNamedContext(SchedulerContext* context)
{
    if (context) {
#if defined(MATCH_NATIVE_CONTEXT)
        context->ReleaseReference();
#else
        context->table->releaseReference(context);
#endif
    }
}

#if defined(MATCH_UNWIND_HELPERS)
// Original: simscheduler.dll 0x20cc15f0
extern "C" void ReturnUnconstructedContextStorage(SchedulerContext* storage)
{
    ReleaseSchedulerContextStorage(storage);
}

extern "C" void __fastcall ReleaseContextReferenceSlot(SchedulerContext* const* slot)
{
    ReleaseNamedContext(*slot);
}
#endif

struct SchedulerContextReference
{
    explicit SchedulerContextReference(SchedulerContext* value) : context(value) {}
    ~SchedulerContextReference() { ReleaseNamedContext(context); }

    SchedulerContext* context;

private:
    SchedulerContextReference(const SchedulerContextReference&);
    SchedulerContextReference& operator=(const SchedulerContextReference&);
};

struct SchedulerNameKey
{
    char text[32];
};

extern "C" int __stdcall LookupSchedulerName(
    const SchedulerNameIndex* index, SchedulerContext** result, SchedulerNameKey key)
{
    SchedulerNameRecord* record;
    if (!index->count || !FindSchedulerName(index, key.text, &record)) {
        return 0;
    }
    if (result) {
        AssignSchedulerContext(result, record->context);
    }
    return 1;
}

struct PublishedSchedulerContextReference
{
    explicit PublishedSchedulerContextReference(SchedulerContext** slot) : result(slot), published(false) {}
    ~PublishedSchedulerContextReference()
    {
        if (published) {
            published = false;
            ReleaseNamedContext(*result);
        }
    }
    void Arm() { published = true; }
    void Complete() { published = false; }

private:
    SchedulerContext** result;
    bool published;
};

extern "C" SchedulerContext** __stdcall FindNamedSchedulerContext(
    SchedulerContextRegistry* registry, SchedulerContext** result, const char* name)
{
    if (GetSchedulerContextNameLength(name) >= sizeof(SchedulerNameKey)) {
        *result = 0;
        return result;
    }
    PublishedSchedulerContextReference output(result);
    {
        SchedulerContextReference found(0);
        SchedulerNameKey key;
        CopySchedulerContextName(key.text, name, sizeof(key.text));
        LookupSchedulerName(&registry->nameIndex, &found.context, key);
        *result = 0;
        AssignSchedulerContext(result, found.context);
        output.Arm();
    }
    output.Complete();
    return result;
}

static const int schedulerNameLimit = 0x38e38e3;

extern "C" int ResizeSchedulerNameStorage(SchedulerNameIndex* index, int requested)
{
    if (requested > schedulerNameLimit) {
        return 0;
    }
    int capacity = 0;
    if (requested > 0) {
        if (index->growth <= 0) {
            index->growth = 5;
        }
        capacity = index->growth;
        if (index->flags & 4) {
            while (capacity < requested) {
                capacity *= 2;
            }
        } else {
            capacity = ((requested + capacity - 1) / capacity) * capacity;
        }
    }
    if (capacity == index->capacity) {
        return 1;
    }
    void* replacement = 0;
    if (capacity > 0) {
        AllocateSchedulerContainerStorage(1, capacity * sizeof(SchedulerNameRecord),
            &replacement, schedulerPendingAllocator);
        if (!replacement) {
            return 0;
        }
    }
    const int retained = requested < index->count ? requested : index->count;
    if (retained > 0) {
        memcpy(replacement, index->records, retained * sizeof(SchedulerNameRecord));
    }
    if (index->records) {
        for (int i = retained; i < index->count; ++i) {
            ReleaseNamedContext(index->records[i].context);
        }
    }
    void* previous = index->records;
    index->count = retained;
    ReleaseSchedulerContainerStorage(&previous);
    index->capacity = capacity;
    index->flags |= 1;
    index->records = static_cast<SchedulerNameRecord*>(replacement);
    return 1;
}

static __forceinline int ClampSchedulerNameCount(int count)
{
    if (count < 0) {
        return 0;
    }
    return count > schedulerNameLimit ? schedulerNameLimit : count;
}

extern "C" int SetSchedulerNameCount(SchedulerNameIndex* index, int requested)
{
    const int count = ClampSchedulerNameCount(requested);
    if (count > index->capacity && !ResizeSchedulerNameStorage(index, count)) {
        return index->count;
    }
    if (count > index->count) {
        for (int i = index->count; i < count; ++i) {
            memset(index->records[i].key, 0, sizeof(unsigned int));
            index->records[i].context = 0;
        }
    } else {
        for (int i = count; i < index->count; ++i) {
            ReleaseNamedContext(index->records[i].context);
        }
    }
    index->count = count;
    return count;
}

#pragma optimize("y", on)
extern "C" SchedulerNameRecord* InsertSchedulerNameSlot(SchedulerNameIndex* index, int position)
{
    const int count = index->count + 1;
    if (count > schedulerNameLimit
        || (count > index->capacity && !ResizeSchedulerNameStorage(index, count))) {
        return 0;
    }
    SchedulerNameRecord* slot = index->records + position;
    MovePendingStorage(slot + 1, slot, (index->count - position) * sizeof(*slot));
    ++index->count;
    memset(slot->key, 0, sizeof(unsigned int));
    slot->context = 0;
    return slot;
}

extern "C" void RemoveSchedulerNameSlot(SchedulerNameIndex* index, int position)
{
    ReleaseNamedContext(index->records[position].context);
    SchedulerNameRecord* slot = index->records + position;
    MovePendingStorage(slot, slot + 1, (index->count - position - 1) * sizeof(*slot));
    --index->count;
}
#pragma optimize("y", off)

extern "C" int __stdcall InsertNamedSchedulerContext(
    SchedulerNameIndex* index, const char* key, SchedulerContext* const* context)
{
    SchedulerNameRecord* slot;
    if (!index->count) {
        slot = SetSchedulerNameCount(index, 1) == 1 ? index->records : 0;
    } else {
        SchedulerNameRecord* position;
        if (FindSchedulerName(index, key, &position)) {
            return 0;
        }
        slot = InsertSchedulerNameSlot(index, static_cast<int>(position - index->records));
    }
    if (!slot) {
        return 0;
    }
    CopySchedulerContextName(slot->key, key, sizeof(slot->key));
    AssignSchedulerContext(&slot->context, *context);
    return 1;
}

extern "C" int __stdcall RemoveNamedSchedulerContext(
    SchedulerContextRegistry* registry, SchedulerContext* context)
{
    if (!context) {
        return 0;
    }
    SchedulerContextReference ownedContext(context);
    SchedulerNameKey key;
#if defined(MATCH_NATIVE_CONTEXT)
    CopySchedulerContextName(key.text, context->GetName(), sizeof(key.text));
#else
    CopySchedulerContextName(key.text, context->table->getName(context), sizeof(key.text));
#endif
    SchedulerNameIndex* index = &registry->nameIndex;
    SchedulerNameRecord* position;
    const bool found = index->count && FindSchedulerName(index, key.text, &position);
    if (found) {
        RemoveSchedulerNameSlot(index, static_cast<int>(position - index->records));
    }
    return found;
}

struct UnconstructedSchedulerContextStorage
{
    explicit UnconstructedSchedulerContextStorage(SchedulerContext* value) : storage(value) {}
    ~UnconstructedSchedulerContextStorage()
    {
        if (storage) {
            ReleaseSchedulerContextStorage(storage);
        }
    }
    void Complete() { storage = 0; }

private:
    SchedulerContext* storage;
};

// simscheduler.dll 0x20cc1670. Owns raw pool storage during construction.
extern "C" SchedulerContext** __stdcall CreateNamedSchedulerContext(
    SchedulerContextRegistry* registry, SchedulerContext** result, const char* name)
{
    char key[0x20];
    if (GetSchedulerContextNameLength(name) >= sizeof(key)) {
        *result = 0;
        return result;
    }
    CopySchedulerContextName(key, name, sizeof(key));
    SchedulerContext* found = 0;
    const bool exists = *registry->table->find(registry, &found, key) != 0;
    if (found) {
#if defined(MATCH_NATIVE_CONTEXT)
        found->ReleaseReference();
#else
        found->table->releaseReference(found);
#endif
    }
    if (exists) {
        registry->table->find(registry, result, key);
        return result;
    }

    PublishedSchedulerContextReference output(result);
    {
        SchedulerContext* storage = AcquireSchedulerContextStorage();
        SchedulerContext* context = 0;
        if (storage) {
            UnconstructedSchedulerContextStorage unconstructed(storage);
            context = ConstructSchedulerContext(storage, key);
            unconstructed.Complete();
        }
        if (context) {
#if defined(MATCH_NATIVE_CONTEXT)
            context->AddReference();
#else
            context->table->addReference(context);
#endif
        }
        SchedulerContextReference ownedContext(context);
        InsertNamedSchedulerContext(&registry->nameIndex, key, &ownedContext.context);
        *result = 0;
        if (context) {
#if defined(MATCH_NATIVE_CONTEXT)
            context->AddReference();
#else
            context->table->addReference(context);
#endif
            if (*result) {
#if defined(MATCH_NATIVE_CONTEXT)
                (*result)->ReleaseReference();
#else
                (*result)->table->releaseReference(*result);
#endif
            }
        }
        *result = context;
        output.Arm();
    }
    output.Complete();
    return result;
}

extern "C" const char schedulerFallbackName[] = "SimConnectFallback";

extern "C" SchedulerContext* __stdcall GetSchedulerContext(SchedulerContext* context)
{
    return context;
}

extern "C" const char* __stdcall GetSchedulerContextName(SchedulerContext* context)
{
    return context->name;
}

#if defined(MATCH_CONTEXT_ACCESSORS_FASTFP)
#pragma float_control(precise, off, push)
#endif
extern "C" int __stdcall SetSchedulerStepBounds(
    SchedulerContext* context, double minimumStep, double maximumStep)
{
    if (minimumStep > maximumStep) {
        return 0;
    }
    context->minimumStep = minimumStep;
    context->maximumStep = maximumStep;
    return 1;
}

extern "C" int __stdcall SetSchedulerMinimumStep(SchedulerContext* context, double minimumStep)
{
    if (minimumStep > context->maximumStep) {
        return 0;
    }
    context->minimumStep = minimumStep;
    return 1;
}
#if defined(MATCH_CONTEXT_ACCESSORS_FASTFP)
#pragma float_control(pop)
#endif

extern "C" int __stdcall SetSchedulerMaximumStep(SchedulerContext* context, double maximumStep)
{
    if (maximumStep < context->minimumStep) {
        return 0;
    }
    context->maximumStep = maximumStep;
    return 1;
}

extern "C" double __stdcall GetSchedulerMinimumStep(SchedulerContext* context)
{
    return context->minimumStep;
}

extern "C" double __stdcall GetSchedulerMaximumStep(SchedulerContext* context)
{
    return context->maximumStep;
}

extern "C" int __stdcall GetSchedulerPrimaryMode(SchedulerContext* context)
{
    return static_cast<int>(context->mode << 31) >> 31;
}

#if defined(MATCH_NATIVE_CONTEXT)
void SchedulerContext::SetPrimaryMode(unsigned int enabled)
{
    SchedulerContext* const context = this;
#else
extern "C" void __stdcall SetSchedulerPrimaryMode(SchedulerContext* context, unsigned int enabled)
{
#endif
    if (!enabled) {
        if (context->mode & 6) {
            return;
        }
        for (int i = context->liveEntries.count; i > 0; --i) {
            if (context->liveEntries.records[i - 1]->flags & 4) {
                return;
            }
        }
    }
    context->substep = 0.0;
    context->mode ^= (context->mode ^ enabled) & 1;
    context->substepCount = 0;
}

extern "C" int __stdcall GetSchedulerUpdateEnabled(SchedulerContext* context)
{
    return static_cast<int>(context->flags << 31) >> 31;
}

// The first fastcall register is unused by the original helper.
// Reconstructed value type for the stack-passed time.
class SchedulerPropagationTime
{
public:
    explicit SchedulerPropagationTime(double value) : seconds(value) {}
    SchedulerPropagationTime(const SchedulerPropagationTime& other) : seconds(other.seconds) {}

    double seconds;
};

// Original: simscheduler.dll 0x20cc2fa0.
extern "C" __declspec(noinline) void __fastcall PropagateSchedulerTime(
    void*, SchedulerContext* context, SchedulerPropagationTime time)
{
    int i = 0;
    if (i < context->liveEntries.count) {
        const SchedulerPropagationTime delivered(time);
        do {
            SchedulerTimedEntry* entry = context->liveEntries.records[i++];
            memcpy(&entry->time, &delivered.seconds, sizeof(delivered.seconds));
        } while (i < context->liveEntries.count);
    }
    memcpy(&context->time, &time.seconds, sizeof(time.seconds));
}

// Original: simscheduler.dll 0x20cc2c40.
extern "C" void __stdcall SetSchedulerUpdateEnabled(SchedulerContext* context, unsigned int enabled)
{
    context->flags ^= (context->flags ^ enabled) & 1;
    if (context->flags & 1) {
        __declspec(align(8)) const SchedulerPropagationTime time(context->time);
        PropagateSchedulerTime(0, context, time);
    }
}

extern "C" void __stdcall SetSchedulerSecondaryMode(SchedulerContext* context, int enabled)
{
    if (enabled && !(context->mode & 1)) {
        return;
    }
    const unsigned int changes = ((static_cast<unsigned int>(enabled) << 1) ^ context->mode) & 2;
    context->mode ^= changes;
}

extern "C" void __stdcall SetSchedulerTertiaryMode(SchedulerContext* context, int enabled)
{
    const unsigned int mode = context->mode;
    if (!(mode & 4) || (mode & 1)) {
        context->mode = mode ^ ((mode ^ (static_cast<unsigned int>(enabled) << 2)) & 4);
    }
}

// simscheduler.dll 0x20cc2ff0, context VTable slot +0x44.
extern "C" double* __stdcall ReadSchedulerContextTime(SchedulerContext* context, double* result)
{
    *result = context->time;
    return result;
}

extern "C" int __stdcall ReadSchedulerSubsteps(SchedulerContext* context, double* step, int* count)
{
    if (context->mode & 1) {
        if (step) {
            *step = context->substep;
        }
        if (count) {
            *count = context->substepCount;
        }
        return 1;
    }
    return 0;
}

extern "C" void __stdcall UpdateSimConnectFallback(
    SchedulerContext* context, SchedulerTimeObserver* observer)
{
    SchedulerContext* previousContext = currentSchedulerContext;
    SchedulerContextReference fallback(0);
    SchedulerContextRegistry* registry = reinterpret_cast<SchedulerContextRegistry*>(&schedulerObject);
    {
#if defined(MATCH_FALLBACK_ALIGNED)
        __declspec(align(64))
#endif
        SchedulerContext* foundResult = 0;
        SchedulerContextReference found(*FindNamedSchedulerContext(
            registry, &foundResult, schedulerFallbackName));
        AssignSchedulerContext(&fallback.context, found.context);
    }

    if ((fallback.context != 0) != (observer != 0)) {
        if (observer) {
            {
                SchedulerContext* createdResult = 0;
                SchedulerContextReference created(*CreateNamedSchedulerContext(
                    registry, &createdResult, schedulerFallbackName));
                AssignSchedulerContext(&fallback.context, created.context);
            }
#if defined(MATCH_NATIVE_CONTEXT)
            fallback.context->SetPrimaryMode(1);
#else
            fallback.context->table->setPrimaryMode(fallback.context, 1);
#endif
#if defined(MATCH_NATIVE_CONTEXT)
            fallback.context->SetUpdateEnabled(1);
#else
            fallback.context->table->setUpdateEnabled(fallback.context, 1);
#endif
        } else {
#if defined(MATCH_NATIVE_CONTEXT)
            fallback.context->AddReference();
#else
            fallback.context->table->addReference(fallback.context);
#endif
            RemoveNamedSchedulerContext(registry, fallback.context);
        }
    }

    SchedulerContext* target = fallback.context;
    SchedulerTimeObserver* previousObserver = simConnectFallbackSetting;
    if (!target || (previousContext == context && previousObserver == observer)) {
        return;
    }
    if (!previousContext && previousObserver) {
#if defined(MATCH_NATIVE_CONTEXT)
        target->UnregisterEntry(previousObserver);
#else
        target->table->removePendingEntry(target, previousObserver);
#endif
    }
    if (!context) {
        if (observer) {
#if defined(MATCH_NATIVE_CONTEXT)
            target->RegisterEntry(observer, 0);
#else
            target->table->addPendingEntry(target, observer, 0);
#endif
        }
    } else if (previousContext != context) {
#if defined(MATCH_NATIVE_CONTEXT)
        target->SetStepBounds(context->minimumStep, context->maximumStep);
#else
        target->table->setStepBounds(target, context->minimumStep, context->maximumStep);
#endif
#if defined(MATCH_NATIVE_CONTEXT)
        target->SetSecondaryMode(-static_cast<int>((context->mode >> 1) & 1));
#else
        target->table->setSecondaryMode(target, -static_cast<int>((context->mode >> 1) & 1));
#endif
#if defined(MATCH_NATIVE_CONTEXT)
        target->SetTertiaryMode(-static_cast<int>((context->mode >> 2) & 1));
#else
        target->table->setTertiaryMode(target, -static_cast<int>((context->mode >> 2) & 1));
#endif
    }
}
#endif
#pragma optimize("y", on)
#endif
#endif

#if defined(MATCH_PENDING_LTCG)
extern "C" __declspec(dllexport) int __stdcall ProbeSchedulerRegistration(
    SchedulerContext* context, void* entry, int classification, int flags, int add)
{
    return QueueSchedulerRegistration(context, entry, classification, flags, add);
}
#endif
#endif

#if !defined(MATCH_PENDING_LTCG) && !defined(MATCH_ENTRY_HELPERS_LTCG)
#pragma optimize("y", off)
extern "C" MATCH_CONTEXT_EXPORT void __stdcall AdvanceSchedulerTime(SchedulerObject* object, double inputStep)
{
    const double scaledStep = object->table->readFieldMinimum(object) * inputStep * (1.0 / 256.0);
    const int suppressed = schedulerApiState.timeUpdateSuppressed != 0;
    const double nextTime = object->time + scaledStep;
    for (int index = 0; index < object->sourceCount; ++index) {
        SchedulerTimeSourceObject* source = object->sources[index].source;
        SchedulerContext* context = source->table->getContext(source);
        if (ProcessSchedulerContext(context, suppressed, object->timeStateActive, nextTime) &&
            context == currentSchedulerContext && simConnectFallbackSetting &&
#if defined(MATCH_NATIVE_CONTEXT)
            context->GetUpdateEnabled()) {
#else
            context->table->shouldNotifyTimeObserver(context)) {
#endif
            SchedulerTimeObserver* observer = simConnectFallbackSetting;
            observer->table->notify(observer, 0.0);
        }
    }
    if (!suppressed) {
        object->time = nextTime;
    }
    object->lastScaledStep = suppressed ? 0.0 : scaledStep;
    object->lastInputStep = inputStep;
    DispatchSchedulerApiEvent(0x22, 0, 0);
}

#if !defined(MATCH_CONTEXT_LTCG)
#if defined(MATCH_NATIVE_CONTEXT)
int SchedulerContext::Reset()
{
    SchedulerContext* const context = this;
#else
extern "C" int __stdcall ResetSchedulerContext(SchedulerContext* context)
{
#endif
    context->pending.count = 0;
    for (int i = 0; i < context->liveEntries.count; ++i) {
        ReleaseSchedulerEntry(context->liveEntries.records[i]);
    }
    context->liveEntries.count = 0;
    context->index.count = 0;
    context->substep = 0.0;
    context->substepCount = 0;
    context->cursor = -1;
    return 1;
}

#if defined(MATCH_NATIVE_CONTEXT)
void SchedulerContext::operator delete(void* storage)
{
    ReleaseSchedulerContextStorage(static_cast<SchedulerContext*>(storage));
}

long SchedulerContext::AddReference()
{
    return AddSchedulerContextReference(this);
}

long SchedulerContext::ReleaseReference()
{
    return ReleaseSchedulerContextReference(this);
}

const char* SchedulerContext::GetName()
{
    return GetSchedulerContextName(this);
}

int SchedulerContext::SetStepBounds(double minimum, double maximum)
{
    return SetSchedulerStepBounds(this, minimum, maximum);
}

int SchedulerContext::SetMinimumStep(double step)
{
    return SetSchedulerMinimumStep(this, step);
}

int SchedulerContext::SetMaximumStep(double step)
{
    return SetSchedulerMaximumStep(this, step);
}

double SchedulerContext::GetMinimumStep()
{
    return GetSchedulerMinimumStep(this);
}

double SchedulerContext::GetMaximumStep()
{
    return GetSchedulerMaximumStep(this);
}

int SchedulerContext::GetPrimaryMode()
{
    return GetSchedulerPrimaryMode(this);
}

int SchedulerContext::GetUpdateEnabled()
{
    return GetSchedulerUpdateEnabled(this);
}

void SchedulerContext::SetUpdateEnabled(unsigned int enabled)
{
    SetSchedulerUpdateEnabled(this, enabled);
}

void SchedulerContext::SetSecondaryMode(int enabled)
{
    SetSchedulerSecondaryMode(this, enabled);
}

void SchedulerContext::SetTertiaryMode(int enabled)
{
    SetSchedulerTertiaryMode(this, enabled);
}

double* SchedulerContext::ReadTime(double* result)
{
    return ReadSchedulerContextTime(this, result);
}

int SchedulerContext::ReadSubsteps(double* step, int* count)
{
    return ReadSchedulerSubsteps(this, step, count);
}

SchedulerContext* SchedulerContext::GetContext()
{
    return GetSchedulerContext(this);
}
#endif

extern "C" const SchedulerVTable schedulerObjectTable;
extern "C" int __cdecl atexit(void (__cdecl *callback)());

extern "C" __declspec(noinline) SchedulerObject* InitializeSchedulerObject()
{
    SchedulerNameIndex& names = reinterpret_cast<SchedulerContextRegistry*>(&schedulerObject)->nameIndex;
    schedulerObject.time = 0.0;
    names.records = 0;
    schedulerObject.lastScaledStep = 0.0;
    names.count = 0;
    schedulerObject.lastInputStep = 0.0;
    names.capacity = 0;
    schedulerObject.timeStateActive = 0;
    schedulerObject.savedTime = 0.0;
    schedulerObject.table = &schedulerObjectTable;
    names.growth = 5;
    names.flags = 4;
    schedulerObject.simSpeed = 256;
    schedulerObject.maximumSimSpeed = 32768;
    return &schedulerObject;
}

extern "C" __declspec(noinline) void DestroySchedulerNameIndex(SchedulerNameIndex* names)
{
    if (!names->records || !(names->flags & 1) || !names->capacity) {
        return;
    }
    const int retained = names->count > 0 ? 0 : names->count;
    for (int i = retained; i < names->count; ++i) {
        ReleaseNamedContext(names->records[i].context);
    }
    void* storage = names->records;
    names->count = retained;
    ReleaseSchedulerContainerStorage(&storage);
    names->flags |= 1;
    names->records = 0;
    names->capacity = 0;
}

extern "C" void DestroySchedulerObject()
{
    DestroySchedulerNameIndex(&reinterpret_cast<SchedulerContextRegistry*>(&schedulerObject)->nameIndex);
}

extern "C" void InitializeSchedulerObjectAtStartup()
{
    InitializeSchedulerObject();
    atexit(DestroySchedulerObject);
}

extern "C" SchedulerContextPool schedulerContextPoolStorage;
extern "C" SchedulerEntryPool schedulerEntryPoolStorage;
extern "C" unsigned int schedulerContextPoolGuard;
extern "C" unsigned int schedulerEntryPoolGuard;
extern "C" const SchedulerPoolTable schedulerPoolBaseTable;
extern "C" const SchedulerPoolTable schedulerContextPoolTable;
extern "C" const SchedulerPoolTable schedulerEntryPoolTable;
extern "C" SchedulerPoolLock* CreateSchedulerPoolLock();
extern "C" unsigned int DestroySchedulerPoolLock(SchedulerPoolLock*);

static __forceinline void UnlinkPoolRegistration(SchedulerPoolRegistration* registration)
{
    SchedulerPoolRegistration* head = reinterpret_cast<SchedulerPoolRegistration*>(schedulerListHead);
    SchedulerPoolRegistration* node = head;
    SchedulerPoolRegistration* previous = 0;
    registration->table = &schedulerPoolBaseTable;
    if (!node) {
        return;
    }
    do {
        if (node == registration) {
            if (previous) {
                previous->next = registration->next;
            }
            if (head == registration) {
                schedulerListHead = reinterpret_cast<SchedulerListNode*>(registration->next);
            }
            return;
        }
        previous = node;
        node = node->next;
    } while (node);
}

extern "C" void __fastcall UnlinkSchedulerPool(SchedulerPoolRegistration* registration)
{
    UnlinkPoolRegistration(registration);
}

__forceinline SchedulerPoolRegistration::SchedulerPoolRegistration()
    : next(reinterpret_cast<SchedulerPoolRegistration*>(schedulerListHead))
{
    schedulerListHead = reinterpret_cast<SchedulerListNode*>(this);
}

SchedulerPoolRegistration::~SchedulerPoolRegistration()
{
    UnlinkPoolRegistration(this);
}

extern "C" unsigned int __fastcall CountUsedSchedulerPoolSlots(SchedulerPoolRegistration* registration)
{
    const unsigned int capacity = registration->table->countCapacity(registration);
    return capacity - registration->table->countAvailable(registration);
}

template <class Block>
static __forceinline void DrainSchedulerPool(SchedulerPool<Block>* pool)
{
    pool->registration.table->countUsed(&pool->registration);
    if (!pool->lock || AcquireSchedulerPoolLock(pool->lock, 1, 0, 0, 0, ~0u, 0)) {
        return;
    }

    Block* block = pool->blocks;
    pool->blocks = 0;
    while (block) {
        Block* next = block->next;
        if (block->freeList) {
            DestroySchedulerPoolFreeList(block->freeList);
            block->freeList = 0;
        }
        void* storage = block;
        ReleaseSchedulerPoolStorage(&storage);
        block = next;
    }
    pool->blockCount = 0;
    if (pool->lock) {
        ReleaseSchedulerPoolLock(pool->lock, 1);
    }
}

extern "C" __declspec(noinline) void __fastcall DrainSchedulerContextPool(SchedulerContextPool* pool)
{
    DrainSchedulerPool(pool);
}

extern "C" __declspec(noinline) void __fastcall DrainSchedulerEntryPool(SchedulerEntryPool* pool)
{
    DrainSchedulerPool(pool);
}

template <class Block>
__forceinline SchedulerPool<Block>::SchedulerPool(const SchedulerPoolTable* table)
{
    registration.table = table;
    blocks = 0;
    lock = 0;
    lock = CreateSchedulerPoolLock();
    if (!lock) {
        BreakOnSchedulerPoolFailure();
    }
    blockCount = 0;
    peakBlockCount = 0;
}

extern "C" __declspec(noinline) SchedulerContextPool* ConstructSchedulerContextPool()
{
    return new (&schedulerContextPoolStorage) SchedulerContextPool(&schedulerContextPoolTable);
}

extern "C" __declspec(noinline) SchedulerEntryPool* ConstructSchedulerEntryPool()
{
    return new (&schedulerEntryPoolStorage) SchedulerEntryPool(&schedulerEntryPoolTable);
}

struct SchedulerPoolUnlink
{
    explicit SchedulerPoolUnlink(SchedulerPoolRegistration& node) : registration(node) {}
    __forceinline ~SchedulerPoolUnlink() { UnlinkPoolRegistration(&registration); }
    SchedulerPoolRegistration& registration;
};

template <class Pool, void (__fastcall *Drain)(Pool*)>
static __forceinline void DestroySchedulerPool(Pool* pool, const SchedulerPoolTable* table)
{
    pool->registration.table = table;
    SchedulerPoolUnlink unlink(pool->registration);
    Drain(pool);
    if (pool->lock) {
        DestroySchedulerPoolLock(pool->lock);
        pool->lock = 0;
    }
}

extern "C" __declspec(noinline) void __stdcall DestroySchedulerContextPool(SchedulerContextPool* pool)
{
    DestroySchedulerPool<SchedulerContextPool, DrainSchedulerContextPool>(pool, &schedulerContextPoolTable);
}

extern "C" __declspec(noinline) void __stdcall DestroySchedulerEntryPool(SchedulerEntryPool* pool)
{
    DestroySchedulerPool<SchedulerEntryPool, DrainSchedulerEntryPool>(pool, &schedulerEntryPoolTable);
}

extern "C" void DestroySchedulerContextPoolAtExit()
{
    DestroySchedulerContextPool(&schedulerContextPoolStorage);
}

extern "C" void DestroySchedulerEntryPoolAtExit()
{
    DestroySchedulerEntryPool(&schedulerEntryPoolStorage);
}

template <unsigned int& Guard>
class SchedulerPoolInitialization
{
public:
    SchedulerPoolInitialization() : completed(false) { Guard |= 1; }
    ~SchedulerPoolInitialization()
    {
        if (!completed) {
            Guard &= ~1u;
        }
    }
    void Complete() { completed = true; }

private:
    bool completed;
};

extern "C" __declspec(noinline) SchedulerContextPool* GetSchedulerContextPool()
{
    if (!(schedulerContextPoolGuard & 1)) {
        SchedulerPoolInitialization<schedulerContextPoolGuard> initialization;
        ConstructSchedulerContextPool();
        atexit(DestroySchedulerContextPoolAtExit);
        initialization.Complete();
    }
    return &schedulerContextPoolStorage;
}

extern "C" __declspec(noinline) SchedulerEntryPool* GetSchedulerEntryPool()
{
    if (!(schedulerEntryPoolGuard & 1)) {
        SchedulerPoolInitialization<schedulerEntryPoolGuard> initialization;
        ConstructSchedulerEntryPool();
        atexit(DestroySchedulerEntryPoolAtExit);
        initialization.Complete();
    }
    return &schedulerEntryPoolStorage;
}

extern "C" void InitializeSchedulerContextPoolAtStartup()
{
    schedulerContextPool = GetSchedulerContextPool();
}

extern "C" void InitializeSchedulerEntryPoolAtStartup()
{
    schedulerEntryPool = GetSchedulerEntryPool();
}

template <class Block>
static __forceinline void TrimSchedulerPool(SchedulerPool<Block>* pool)
{
    if (!pool->lock || AcquireSchedulerPoolLock(pool->lock, 1, 0, 0, 0, ~0u, 0)) {
        return;
    }
    Block* previous = 0;
    for (Block* block = pool->blocks; block; ) {
        Block* next = block->next;
        if (block->freeCount != Block::entryCount) {
            previous = block;
        } else {
            if (previous) {
                previous->next = next;
            } else {
                pool->blocks = next;
            }
            if (block->freeList) {
                DestroySchedulerPoolFreeList(block->freeList);
                block->freeList = 0;
            }
            void* storage = block;
            ReleaseSchedulerPoolStorage(&storage);
            --pool->blockCount;
        }
        block = next;
    }
    if (pool->lock) {
        ReleaseSchedulerPoolLock(pool->lock, 1);
    }
}

extern "C" void __fastcall TrimSchedulerContextPool(SchedulerContextPool* pool)
{
    TrimSchedulerPool(pool);
}

extern "C" void __fastcall TrimSchedulerEntryPool(SchedulerEntryPool* pool)
{
    TrimSchedulerPool(pool);
}

#pragma optimize("y", on)
template <class Block>
static __forceinline unsigned int CountAvailableSchedulerPoolSlots(SchedulerPool<Block>* pool)
{
    if (pool->lock && AcquireSchedulerPoolLock(pool->lock, 0, 0, 0, 0, ~0u, 0)) {
        return 0;
    }
    unsigned int available = 0;
    for (const Block* block = pool->blocks; block; block = block->next) {
        available += block->freeCount;
    }
    if (pool->lock) {
        ReleaseSchedulerPoolLock(pool->lock, 0);
    }
    return available;
}

extern "C" unsigned int __fastcall CountAvailableSchedulerContextSlots(SchedulerContextPool* pool)
{
    return CountAvailableSchedulerPoolSlots(pool);
}

extern "C" unsigned int __fastcall CountAvailableSchedulerEntrySlots(SchedulerEntryPool* pool)
{
    return CountAvailableSchedulerPoolSlots(pool);
}

extern "C" unsigned int __fastcall ReadSchedulerContextPoolCapacity(const SchedulerContextPool* pool)
{
    return pool->blockCount * SchedulerContextPoolBlock::entryCount;
}

extern "C" unsigned int __fastcall ReadSchedulerEntryPoolCapacity(const SchedulerEntryPool* pool)
{
    return pool->blockCount * SchedulerEntryPoolBlock::entryCount;
}

// Both pool types share these offsets in their registration layout.
extern "C" unsigned int __fastcall ReadSchedulerPoolBlockCount(const SchedulerContextPool* pool)
{
    return pool->blockCount;
}

extern "C" unsigned int __fastcall ReadSchedulerPoolPeakBlockCount(const SchedulerContextPool* pool)
{
    return pool->peakBlockCount;
}

// The same original body also reports sizeof(SchedulerTimedEntry).
extern "C" unsigned int __fastcall ReadSchedulerContextsPerBlock(const SchedulerPoolRegistration*)
{
    return SchedulerContextPoolBlock::entryCount;
}

extern "C" unsigned int __fastcall ReadSchedulerEntriesPerBlock(const SchedulerPoolRegistration*)
{
    return SchedulerEntryPoolBlock::entryCount;
}

extern "C" unsigned int __fastcall ReadSchedulerContextPoolBlockBytes(const SchedulerContextPool* pool)
{
    return pool->blockCount * sizeof(SchedulerContextPoolBlock);
}

extern "C" unsigned int __fastcall ReadSchedulerEntryPoolBlockBytes(const SchedulerEntryPool* pool)
{
    return pool->blockCount * sizeof(SchedulerEntryPoolBlock);
}

extern "C" unsigned int __fastcall ReadSchedulerContextStorageSize(const SchedulerPoolRegistration*)
{
    return sizeof(SchedulerContextStorage);
}

#pragma optimize("y", off)
template <class Pool>
static __forceinline Pool* FinishSchedulerPoolDeletion(Pool* pool, unsigned int flags)
{
    if (flags & 1) {
        void* storage = pool;
        ReleaseSchedulerPoolStorage(&storage);
    }
    return pool;
}

SchedulerPoolRegistration* SchedulerPoolRegistration::destroy(unsigned int flags)
{
    UnlinkPoolRegistration(this);
    return FinishSchedulerPoolDeletion(this, flags);
}

template <>
SchedulerContextPool* SchedulerContextPool::destroy(unsigned int flags)
{
    DestroySchedulerContextPool(this);
    return FinishSchedulerPoolDeletion(this, flags);
}

template <>
SchedulerEntryPool* SchedulerEntryPool::destroy(unsigned int flags)
{
    DestroySchedulerEntryPool(this);
    return FinishSchedulerPoolDeletion(this, flags);
}

#include <windows.h>
#include <intrin.h>

struct SchedulerFreeListAllocator
{
    unsigned int alignment;
    void* context;
    void* (__stdcall *allocate)(void*, unsigned int);
    void (__stdcall *release)(void*, void*);
};

struct SchedulerFreeListNode
{
    SchedulerFreeListNode* volatile next;
    void* value;
};

union SchedulerFreeListHead
{
    volatile __int64 tagged;
    SchedulerFreeListNode* volatile node;
};

struct __declspec(align(8)) SchedulerPoolFreeList
{
    SchedulerFreeListHead spare;
    SchedulerFreeListHead ready;
    volatile LONG spareSignals;
    HANDLE spareSemaphore;
    volatile LONG readySignals;
    HANDLE readySemaphore;
    SchedulerFreeListHead allocations;
    LONG batchSize;
    volatile LONG remainingNodes;
    volatile LONG allocatedNodes;
    volatile LONG allocatedBytes;
    volatile LONG signature;
    void* rawAllocation;
    SchedulerFreeListAllocator* allocator;
};

typedef char SchedulerFreeListSizeCheck[sizeof(SchedulerPoolFreeList) == 0x48 ? 1 : -1];
typedef char SchedulerFreeListNodeSizeCheck[sizeof(SchedulerFreeListNode) == 8 ? 1 : -1];

extern "C" volatile signed char schedulerAtomicMode;
static const LONG schedulerFreeListSignature = 0x4c665374;

extern "C" __declspec(noinline) void InitializeSchedulerAtomicMode()
{
    if (schedulerAtomicMode == 2) {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        const DWORD_PTR active = info.dwActiveProcessorMask;
        schedulerAtomicMode = info.dwNumberOfProcessors <= 1 && !(active & (active - 1)) ? -1 : 1;
    }
}

static __forceinline SchedulerFreeListNode* ReadSchedulerFreeListHead(__int64 head)
{
    return reinterpret_cast<SchedulerFreeListNode*>(static_cast<unsigned int>(head));
}

static __forceinline __int64 MakeSchedulerFreeListHead(SchedulerFreeListNode* node, unsigned int sequence)
{
    return static_cast<__int64>((static_cast<unsigned __int64>(sequence) << 32)
        | reinterpret_cast<unsigned int>(node));
}

static __forceinline unsigned int ReadSchedulerFreeListSequence(__int64 head)
{
    return static_cast<unsigned int>(static_cast<unsigned __int64>(head) >> 32);
}

static __forceinline SchedulerFreeListNode* PopSchedulerFreeListNode(SchedulerFreeListHead* head)
{
    __int64 current = _InterlockedCompareExchange64(&head->tagged, 0, 0);
    while (SchedulerFreeListNode* node = ReadSchedulerFreeListHead(current)) {
        const __int64 replacement = MakeSchedulerFreeListHead(node->next,
            ReadSchedulerFreeListSequence(current) + 1);
        const __int64 observed = _InterlockedCompareExchange64(&head->tagged, replacement, current);
        if (observed == current) {
            return node;
        }
        current = observed;
    }
    return 0;
}

static __forceinline void PushSchedulerFreeListNode(
    SchedulerFreeListHead* head, SchedulerFreeListNode* node)
{
    SchedulerFreeListNode* current = head->node;
    for (;;) {
        node->next = current;
        SchedulerFreeListNode* observed = static_cast<SchedulerFreeListNode*>(
            InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(&head->node), node, current));
        if (observed == current) {
            return;
        }
        current = observed;
    }
}

extern "C" __declspec(noinline) void* AllocateSchedulerFreeListStorage(
    unsigned int bytes, volatile LONG* allocatedBytes, SchedulerFreeListAllocator* allocator)
{
    if (allocator) {
        void* storage = allocator->allocate(allocator->context, bytes);
        if (allocatedBytes) {
            InterlockedExchangeAdd(allocatedBytes, bytes);
        }
        return storage;
    }
    HANDLE heap = GetProcessHeap();
    void* storage = HeapAlloc(heap, 0, bytes);
    if (storage && allocatedBytes) {
        const LONG size = static_cast<LONG>(HeapSize(heap, 0, storage));
        if (size > 0) {
            InterlockedExchangeAdd(allocatedBytes, size);
        }
    }
    return storage;
}

extern "C" __declspec(noinline) void ReleaseSchedulerFreeListStorage(void* storage, volatile LONG* allocatedBytes)
{
    HANDLE heap = GetProcessHeap();
    if (allocatedBytes) {
        const LONG size = static_cast<LONG>(HeapSize(heap, 0, storage));
        if (size > 0) {
            InterlockedExchangeAdd(allocatedBytes, -size);
        }
    }
    HeapFree(heap, 0, storage);
}

extern "C" __declspec(noinline) void* AllocateAlignedSchedulerFreeListStorage(
    unsigned int bytes, SchedulerFreeListAllocator* allocator, volatile LONG* allocatedBytes, void** raw)
{
    unsigned int alignment = bytes <= 32 ? 32 : 64;
    while (bytes && bytes <= alignment / 2) {
        alignment /= 2;
    }
    if (alignment <= (allocator ? allocator->alignment : 8)) {
        alignment = 1;
    }
    *raw = 0;
    if (!bytes || bytes > 0x7fffffffu - alignment) {
        return 0;
    }
    void* storage = AllocateSchedulerFreeListStorage(bytes + alignment - 1, allocatedBytes, allocator);
    *raw = storage;
    const unsigned int address = reinterpret_cast<unsigned int>(storage);
    return reinterpret_cast<void*>((address + alignment - 1) & ~(alignment - 1));
}

extern "C" __declspec(noinline) SchedulerFreeListNode* AllocateSchedulerFreeListBatch(
    unsigned int bytes, SchedulerFreeListAllocator* allocator, SchedulerFreeListHead* allocations,
    volatile LONG* allocatedBytes)
{
    if (!bytes || bytes > 0x7fffffffu - sizeof(SchedulerFreeListNode)) {
        return 0;
    }
    void* storage;
    void* aligned = AllocateAlignedSchedulerFreeListStorage(bytes + sizeof(SchedulerFreeListNode),
        allocator, allocatedBytes, &storage);
    if (!aligned) {
        return 0;
    }
    SchedulerFreeListNode* marker = reinterpret_cast<SchedulerFreeListNode*>(
        static_cast<char*>(aligned) + bytes);
    marker->value = storage;
    PushSchedulerFreeListNode(allocations, marker);
    return static_cast<SchedulerFreeListNode*>(aligned);
}

extern "C" __declspec(noinline) void ReleaseSchedulerFreeListBatches(
    SchedulerFreeListHead* allocations, SchedulerFreeListAllocator* allocator, volatile LONG* allocatedBytes)
{
    while (SchedulerFreeListNode* marker = PopSchedulerFreeListNode(allocations)) {
        void* storage = marker->value;
        if (allocator) {
            allocator->release(allocator->context, storage);
        } else {
            ReleaseSchedulerFreeListStorage(storage, allocatedBytes);
        }
    }
}

extern "C" SchedulerPoolFreeList* __stdcall CreateSchedulerPoolFreeList(int batchSize, int limit)
{
    void* raw;
    LONG bytes = 0;
    SchedulerPoolFreeList* list = static_cast<SchedulerPoolFreeList*>(
        AllocateAlignedSchedulerFreeListStorage(sizeof(SchedulerPoolFreeList), 0, &bytes, &raw));
    if (!list) {
        return 0;
    }
    memset(list, 0, sizeof(*list));
    list->rawAllocation = raw;
    list->allocatedBytes = bytes;
    if (batchSize < 16) {
        batchSize = 16;
    } else if (batchSize > 256) {
        batchSize = 256;
    }
    if (limit < 1) {
        limit = -1;
    } else {
        if (limit >= 0xfffffff) {
            limit = 0xffffffe;
        }
        if (batchSize > limit) {
            batchSize = limit;
        }
    }
    list->batchSize = batchSize;
    list->remainingNodes = limit;
    InitializeSchedulerAtomicMode();
    InterlockedExchange(&list->signature, schedulerFreeListSignature);
    return list;
}

extern "C" __declspec(noinline) int DestroySchedulerPoolFreeList(SchedulerPoolFreeList* list)
{
    if (!list || InterlockedExchange(&list->signature, 0) != schedulerFreeListSignature) {
        return 1;
    }
    InitializeSchedulerAtomicMode();
    if (list->spareSemaphore) {
        CloseHandle(list->spareSemaphore);
    }
    if (list->readySemaphore) {
        CloseHandle(list->readySemaphore);
    }
    LONG bytes = list->allocatedBytes;
    ReleaseSchedulerFreeListBatches(&list->allocations, list->allocator, &bytes);
    if (list->allocator) {
        list->allocator->release(list->allocator->context, list->rawAllocation);
    } else {
        ReleaseSchedulerFreeListStorage(list->rawAllocation, &bytes);
    }
    return 1;
}

extern "C" __declspec(noinline) SchedulerFreeListNode* __stdcall RefillSchedulerPoolFreeList(
    SchedulerPoolFreeList* list)
{
    LONG count = list->batchSize;
    for (;;) {
        const LONG remaining = list->remainingNodes;
        if (remaining < 0) {
            break;
        }
        if (!remaining) {
            return 0;
        }
        if (remaining < count) {
            count = remaining;
        }
        if (InterlockedCompareExchange(&list->remainingNodes, remaining - count, remaining) == remaining) {
            break;
        }
    }
    SchedulerFreeListNode* first = AllocateSchedulerFreeListBatch(count * sizeof(*first),
        list->allocator, &list->allocations, &list->allocatedBytes);
    if (!first) {
        InterlockedExchange(&list->remainingNodes, 0);
        return 0;
    }
    InterlockedExchangeAdd(&list->allocatedNodes, count);
    for (LONG i = count - 1; i > 0; --i) {
        PushSchedulerFreeListNode(&list->spare, first + i);
    }
    if (list->spareSemaphore) {
        const LONG previous = InterlockedExchangeAdd(&list->spareSignals, count);
        if (previous < -1 && count > 1) {
            const LONG waiting = ~previous;
            ReleaseSemaphore(list->spareSemaphore, count - 1 < waiting ? count - 1 : waiting, 0);
        }
    }
    return first;
}

static __forceinline void PublishSchedulerFreeListNode(
    SchedulerFreeListHead* head, SchedulerFreeListNode* node, HANDLE semaphore, volatile LONG* signals)
{
    PushSchedulerFreeListNode(head, node);
    if (semaphore && InterlockedIncrement(signals) <= 0) {
        ReleaseSemaphore(semaphore, 1, 0);
    }
}

extern "C" __declspec(noinline) int __stdcall PushSchedulerPoolEntry(SchedulerPoolFreeList* list, void* entry)
{
    if (!entry || !list || list->signature != schedulerFreeListSignature) {
        return 0;
    }
    InitializeSchedulerAtomicMode();
    const bool reserved = list->spareSemaphore != 0;
    SchedulerFreeListNode* node = 0;
    if (!reserved || InterlockedDecrement(&list->spareSignals) >= 0) {
        node = PopSchedulerFreeListNode(&list->spare);
    }
    if (!node && list->remainingNodes) {
        node = RefillSchedulerPoolFreeList(list);
    }
    if (!node) {
        if (reserved) {
            InterlockedIncrement(&list->spareSignals);
        }
        return 0;
    }
    node->value = entry;
    PublishSchedulerFreeListNode(&list->ready, node, list->readySemaphore, &list->readySignals);
    return 1;
}

extern "C" __declspec(noinline) void* PopSchedulerPoolEntry(SchedulerPoolFreeList* list)
{
    if (!list || list->signature != schedulerFreeListSignature) {
        return 0;
    }
    InitializeSchedulerAtomicMode();
    const bool reserved = list->readySemaphore != 0;
    SchedulerFreeListNode* node = 0;
    if (!reserved || InterlockedDecrement(&list->readySignals) >= 0) {
        node = PopSchedulerFreeListNode(&list->ready);
    }
    if (!node) {
        if (reserved) {
            InterlockedIncrement(&list->readySignals);
        }
        return 0;
    }
    void* value = node->value;
    PublishSchedulerFreeListNode(&list->spare, node, list->spareSemaphore, &list->spareSignals);
    return value;
}

struct __declspec(align(8)) SchedulerPoolLock
{
    volatile __int64 state;
    LONG ownerMode;
    volatile DWORD owner;
    HANDLE readers;
    HANDLE writer;
    volatile LONG signature;
    unsigned int writerDepth;
    void* rawAllocation;
    SchedulerFreeListAllocator* allocator;
};

typedef char SchedulerPoolLockSizeCheck[sizeof(SchedulerPoolLock) == 0x28 ? 1 : -1];

static const LONG schedulerPoolLockSignature = 0x4c664c6b;
static const unsigned int schedulerWaitingWriters = 0xfffff;
static const unsigned int schedulerExclusive = 0x100000;
static const unsigned int schedulerPromoting = 0x200000;
static const unsigned int schedulerSpin = 0x400000;
static const unsigned int schedulerClosing = 0x800000;
static const unsigned __int64 schedulerWaitingReader = 0x1000000;
static const unsigned __int64 schedulerSharedReader = 0x100000000000ULL;
static const DWORD schedulerNotOwned = 0xe0000001;
static const DWORD schedulerPromotionPending = 0xe0000002;
static const DWORD schedulerWriterPending = 0xe0000003;
static const DWORD schedulerRecursionDisabled = 0xe0000004;

struct SchedulerPoolLockState
{
    explicit SchedulerPoolLockState(unsigned __int64 value)
        : bits(value), low(static_cast<unsigned int>(value)) {}

    bool hasReaders() const { return (bits >> 44) != 0; }
    bool hasWriter() const { return (low & schedulerExclusive) != 0; }
    bool hasWaitingWriter() const { return (low & schedulerWaitingWriters) != 0; }
    LONG waitingReaders() const { return static_cast<LONG>((bits >> 24) & 0xfffff); }

    unsigned __int64 bits;
    unsigned int low;
};

static __forceinline SchedulerPoolLockState ReadSchedulerPoolLockState(SchedulerPoolLock* lock)
{
    return SchedulerPoolLockState(_InterlockedCompareExchange64(&lock->state, 0, 0));
}

static __forceinline bool SetSchedulerPoolLockState(
    SchedulerPoolLock* lock, const SchedulerPoolLockState& before, unsigned __int64 after)
{
    return static_cast<unsigned __int64>(_InterlockedCompareExchange64(
        &lock->state, static_cast<__int64>(after), static_cast<__int64>(before.bits))) == before.bits;
}

static __forceinline unsigned __int64 SchedulerPoolSpinState(unsigned __int64 state, bool spin)
{
    return spin ? state | schedulerSpin : state & ~static_cast<unsigned __int64>(schedulerSpin);
}

extern "C" __declspec(noinline) unsigned int PauseSchedulerPoolLock(unsigned int mask)
{
    unsigned int pauses = (static_cast<unsigned int>(__rdtsc()) & mask) + 1;
    do {
        _mm_pause();
    } while (--pauses);
    return mask < 512 ? mask * 2 + 1 : mask;
}

typedef DWORD (__stdcall *SchedulerPoolWaitCallback)(void*, HANDLE, DWORD, BOOL);

struct SchedulerPoolWait
{
    DWORD operator()(HANDLE handle) const
    {
        if (callback) {
            return callback(context, handle, timeout, alertable);
        }
        return timeout ? WaitForSingleObjectEx(handle, timeout, alertable) : WAIT_TIMEOUT;
    }

    SchedulerPoolWaitCallback callback;
    void* context;
    DWORD timeout;
    BOOL alertable;
    bool spin;
    bool tryOnly;
};

static __forceinline bool CannotWaitForSchedulerPool(const SchedulerPoolLockState& state,
    const SchedulerPoolWait& wait)
{
    return wait.tryOnly || (schedulerAtomicMode > 0 && !(state.low & schedulerSpin) && !wait.timeout);
}

static __forceinline void WakeSchedulerPoolReaders(SchedulerPoolLock* lock, const SchedulerPoolLockState& state)
{
    const LONG count = state.waitingReaders();
    if (!state.hasWriter() && !state.hasWaitingWriter() && count) {
        ReleaseSemaphore(lock->readers, count, 0);
    }
}

static __forceinline DWORD AcquireSchedulerPoolReadLock(SchedulerPoolLock* lock, bool ownWriter,
    const SchedulerPoolWait& wait)
{
    if (ownWriter && !lock->ownerMode) {
        return schedulerRecursionDisabled;
    }
    for (;;) {
        const SchedulerPoolLockState state = ReadSchedulerPoolLockState(lock);
        if (ownWriter || (!state.hasWriter() && !state.hasWaitingWriter())) {
            const unsigned __int64 bits = SchedulerPoolSpinState(state.bits, wait.spin &&
                (!state.hasReaders() || (state.low & schedulerSpin)));
            if (SetSchedulerPoolLockState(lock, state, bits + schedulerSharedReader)) {
                return 0;
            }
        } else {
            if (CannotWaitForSchedulerPool(state, wait)) {
                return WAIT_TIMEOUT;
            }
            if (SetSchedulerPoolLockState(lock, state, state.bits + schedulerWaitingReader)) {
                break;
            }
        }
    }

    unsigned int spins = schedulerAtomicMode > 0 ? 1000 : 0;
    unsigned int mask = 0;
    bool signaled = false;
    for (;;) {
        const SchedulerPoolLockState state = ReadSchedulerPoolLockState(lock);
        const bool spinning = spins && (state.low & schedulerSpin);
        if ((signaled || spinning) && !state.hasWriter() && !state.hasWaitingWriter()) {
            const unsigned __int64 bits = SchedulerPoolSpinState(state.bits, wait.spin &&
                (!state.hasReaders() || (state.low & schedulerSpin)));
            if (SetSchedulerPoolLockState(lock, state, bits - schedulerWaitingReader + schedulerSharedReader)) {
                return 0;
            }
            continue;
        }
        if (spinning) {
            --spins;
            mask = PauseSchedulerPoolLock(mask);
            continue;
        }
        spins = 0;
        const DWORD result = wait(lock->readers);
        signaled = result == WAIT_OBJECT_0;
        if (!signaled) {
            for (;;) {
                const SchedulerPoolLockState pending = ReadSchedulerPoolLockState(lock);
                if (SetSchedulerPoolLockState(lock, pending, pending.bits - schedulerWaitingReader)) {
                    return result;
                }
            }
        }
    }
}

static __forceinline DWORD WaitForSchedulerPoolWriter(SchedulerPoolLock* lock, bool promoting, DWORD owner,
    const SchedulerPoolWait& wait)
{
    unsigned int spins = schedulerAtomicMode > 0 ? 1000 : 0;
    unsigned int mask = 0;
    bool signaled = false;
    for (;;) {
        const SchedulerPoolLockState state = ReadSchedulerPoolLockState(lock);
        const bool spinning = spins && (state.low & schedulerSpin);
        if ((signaled || spinning) && !state.hasReaders() && !state.hasWriter() &&
            (promoting || !(state.low & schedulerPromoting))) {
            unsigned __int64 bits = (state.bits | schedulerExclusive) - 1;
            if (promoting) {
                bits &= ~static_cast<unsigned __int64>(schedulerPromoting);
            }
            if (SetSchedulerPoolLockState(lock, state, SchedulerPoolSpinState(bits, wait.spin))) {
                lock->owner = owner;
                return 0;
            }
            continue;
        }
        if (spinning) {
            --spins;
            mask = PauseSchedulerPoolLock(mask);
            continue;
        }
        spins = 0;
        const DWORD result = wait(lock->writer);
        signaled = result == WAIT_OBJECT_0;
        if (signaled) {
            if (!promoting && (ReadSchedulerPoolLockState(lock).low & schedulerPromoting)) {
                SetEvent(lock->writer);
            }
            continue;
        }
        for (;;) {
            const SchedulerPoolLockState pending = ReadSchedulerPoolLockState(lock);
            unsigned __int64 bits = pending.bits - 1;
            if (promoting) {
                bits = (bits & ~static_cast<unsigned __int64>(schedulerPromoting)) + schedulerSharedReader;
            }
            if (SetSchedulerPoolLockState(lock, pending, bits)) {
                WakeSchedulerPoolReaders(lock, SchedulerPoolLockState(bits));
                return result;
            }
        }
    }
}

static __forceinline DWORD AcquireSchedulerPoolWriteLock(SchedulerPoolLock* lock, DWORD owner, bool ownWriter,
    const SchedulerPoolWait& wait)
{
    if (ownWriter) {
        if (!lock->ownerMode) {
            return schedulerRecursionDisabled;
        }
        ++lock->writerDepth;
        if (!wait.spin) {
            for (;;) {
                const SchedulerPoolLockState state = ReadSchedulerPoolLockState(lock);
                if (SetSchedulerPoolLockState(lock, state, SchedulerPoolSpinState(state.bits, false))) {
                    break;
                }
            }
        }
        return 0;
    }
    for (;;) {
        const SchedulerPoolLockState state = ReadSchedulerPoolLockState(lock);
        if (!state.hasReaders() && !state.hasWriter() && !(state.low & schedulerPromoting) &&
            (!state.hasWaitingWriter() || (state.low & schedulerSpin))) {
            if (SetSchedulerPoolLockState(lock, state,
                SchedulerPoolSpinState(state.bits | schedulerExclusive, wait.spin))) {
                lock->owner = owner;
                return 0;
            }
        } else if (CannotWaitForSchedulerPool(state, wait)) {
            return WAIT_TIMEOUT;
        } else if (SetSchedulerPoolLockState(lock, state, state.bits + 1)) {
            return WaitForSchedulerPoolWriter(lock, false, owner, wait);
        }
    }
}

static __forceinline DWORD DowngradeSchedulerPoolLock(SchedulerPoolLock* lock, bool ownWriter, bool spin)
{
    if (lock->ownerMode >= 0 && !ownWriter) {
        return schedulerNotOwned;
    }
    const bool recursive = lock->writerDepth != 0;
    if (!recursive && lock->ownerMode >= 0) {
        lock->owner = 0;
    }
    for (;;) {
        const SchedulerPoolLockState state = ReadSchedulerPoolLockState(lock);
        if (!recursive && !state.hasWriter()) {
            return schedulerNotOwned;
        }
        unsigned __int64 bits = state.bits + schedulerSharedReader;
        if (!recursive) {
            bits &= ~static_cast<unsigned __int64>(schedulerExclusive);
        }
        bits = SchedulerPoolSpinState(bits, spin);
        if (SetSchedulerPoolLockState(lock, state, bits)) {
            if (recursive) {
                --lock->writerDepth;
            } else {
                WakeSchedulerPoolReaders(lock, SchedulerPoolLockState(bits));
            }
            return 0;
        }
    }
}

static __forceinline DWORD PromoteSchedulerPoolLock(SchedulerPoolLock* lock, DWORD owner, bool ownWriter,
    bool respectQueuedWriter, const SchedulerPoolWait& wait)
{
    for (;;) {
        const SchedulerPoolLockState state = ReadSchedulerPoolLockState(lock);
        if (!state.hasReaders()) {
            return ERROR_INVALID_PARAMETER;
        }
        const unsigned __int64 fewerReaders = state.bits - schedulerSharedReader;
        if (ownWriter) {
            const unsigned __int64 bits = wait.spin ? fewerReaders : SchedulerPoolSpinState(fewerReaders, false);
            if (SetSchedulerPoolLockState(lock, state, bits)) {
                ++lock->writerDepth;
                return 0;
            }
            continue;
        }
        if (state.low & schedulerPromoting) {
            return schedulerPromotionPending;
        }
        if (respectQueuedWriter && state.hasWaitingWriter()) {
            return schedulerWriterPending;
        }
        if (!state.hasWriter() && !SchedulerPoolLockState(fewerReaders).hasReaders()) {
            if (SetSchedulerPoolLockState(lock, state,
                SchedulerPoolSpinState(fewerReaders | schedulerExclusive, wait.spin))) {
                lock->owner = owner;
                return 0;
            }
        } else if (CannotWaitForSchedulerPool(state, wait)) {
            return WAIT_TIMEOUT;
        } else if (SetSchedulerPoolLockState(lock, state, (fewerReaders | schedulerPromoting) + 1)) {
            return WaitForSchedulerPoolWriter(lock, true, owner, wait);
        }
    }
}

extern "C" unsigned int __stdcall AcquireSchedulerPoolLock(SchedulerPoolLock* lock, int mode,
    int waitContext, void* waitCallback, int spin, unsigned int timeout, int alertable)
{
    if (!lock || lock->signature != schedulerPoolLockSignature) {
        return ERROR_INVALID_PARAMETER;
    }
    InitializeSchedulerAtomicMode();
    const SchedulerPoolWait wait = {reinterpret_cast<SchedulerPoolWaitCallback>(waitCallback),
        reinterpret_cast<void*>(waitContext), timeout, alertable, spin != 0, (mode & 8) != 0};
    const DWORD owner = lock->ownerMode >= 0 ? GetCurrentThreadId() : 0;
    const bool ownWriter = owner != 0 && lock->owner == owner;
    switch (mode & ~8) {
    case 0:
        return AcquireSchedulerPoolReadLock(lock, ownWriter, wait);
    case 1:
        return AcquireSchedulerPoolWriteLock(lock, owner, ownWriter, wait);
    case 2:
        return DowngradeSchedulerPoolLock(lock, ownWriter, wait.spin);
    case 3:
    case 4:
        return PromoteSchedulerPoolLock(lock, owner, ownWriter, (mode & ~8) == 3, wait);
    default:
        return ERROR_INVALID_PARAMETER;
    }
}

extern "C" unsigned int __stdcall ReleaseSchedulerPoolLock(SchedulerPoolLock* lock, int exclusive)
{
    if (!lock || lock->signature != schedulerPoolLockSignature) {
        return ERROR_INVALID_PARAMETER;
    }
    InitializeSchedulerAtomicMode();
    if (exclusive && lock->ownerMode >= 0) {
        if (lock->owner != GetCurrentThreadId()) {
            return schedulerNotOwned;
        }
        if (lock->writerDepth) {
            --lock->writerDepth;
            return 0;
        }
        lock->owner = 0;
    }
    for (;;) {
        const SchedulerPoolLockState state = ReadSchedulerPoolLockState(lock);
        if (exclusive ? !state.hasWriter() : !state.hasReaders()) {
            return schedulerNotOwned;
        }
        const unsigned __int64 bits = exclusive ? state.bits & ~static_cast<unsigned __int64>(schedulerExclusive)
            : state.bits - schedulerSharedReader;
        if (SetSchedulerPoolLockState(lock, state, bits)) {
            const SchedulerPoolLockState after(bits);
            if (!after.hasReaders() && !after.hasWriter() && after.hasWaitingWriter()) {
                SetEvent(lock->writer);
            } else if (exclusive) {
                WakeSchedulerPoolReaders(lock, after);
            }
            return 0;
        }
    }
}

extern "C" SchedulerPoolLock* CreateSchedulerPoolLock()
{
    void* raw;
    SchedulerPoolLock* lock = static_cast<SchedulerPoolLock*>(
        AllocateAlignedSchedulerFreeListStorage(sizeof(SchedulerPoolLock), 0, 0, &raw));
    if (!lock) {
        return 0;
    }
    memset(lock, 0, sizeof(*lock));
    lock->rawAllocation = raw;
    lock->readers = CreateSemaphoreA(0, 0, 0x7fffffff, 0);
    if (lock->readers) {
        lock->writer = CreateEventA(0, FALSE, FALSE, 0);
        if (lock->writer) {
            InitializeSchedulerAtomicMode();
            InterlockedExchange(&lock->signature, schedulerPoolLockSignature);
            return lock;
        }
        CloseHandle(lock->readers);
    }
    ReleaseSchedulerFreeListStorage(raw, 0);
    return 0;
}

extern "C" unsigned int DestroySchedulerPoolLock(SchedulerPoolLock* lock)
{
    InitializeSchedulerAtomicMode();
    if (!lock || InterlockedExchange(&lock->signature, 0) != schedulerPoolLockSignature) {
        return ERROR_INVALID_PARAMETER;
    }
    for (;;) {
        const SchedulerPoolLockState state = ReadSchedulerPoolLockState(lock);
        if (state.hasWriter() || state.hasReaders() || state.hasWaitingWriter() || state.waitingReaders()) {
            return 0;
        }
        if (SetSchedulerPoolLockState(lock, state, state.bits | schedulerClosing)) {
            break;
        }
    }
    const BOOL writerClosed = CloseHandle(lock->writer);
    const BOOL readersClosed = CloseHandle(lock->readers);
    if (lock->allocator) {
        lock->allocator->release(lock->allocator->context, lock->rawAllocation);
    } else {
        ReleaseSchedulerFreeListStorage(lock->rawAllocation, 0);
    }
    return writerClosed & readersClosed;
}
#endif
#pragma optimize("y", on)
#endif
