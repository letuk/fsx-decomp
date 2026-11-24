#include "cab_atomic_state.h"
#include <stddef.h>

struct CabPoolLockObserved40Pilot {
    volatile DWORD stateLow, stateHigh;
    LONG ownerTracking;
    DWORD ownerThread;
    HANDLE readerSemaphore, writerEvent;
    volatile LONG signature;
    DWORD recursion;
    void* rawAllocation;
    CabSlotAllocatorObservedPilot* allocator;
};
C_ASSERT(sizeof(CabPoolLockObserved40Pilot) == 0x28);
C_ASSERT(offsetof(CabPoolLockObserved40Pilot, signature) == 0x18);

static const DWORD CabLockSignature = 0x4c664c6b;
static const DWORD CabLockInvalidRelease = 0xe0000001;
static const DWORD CabLockWriterCount = 0x000fffff;
static const DWORD CabLockExclusive = 0x00100000;
static const DWORD CabLockReaderCount = 0xfffff000;
static const DWORD CabLockUpgrade = 0x00200000;
static const DWORD CabLockSpin = 0x00400000;
static const DWORD CabLockReaderBlockers = CabLockWriterCount | CabLockExclusive;
static const DWORD CabLockReaderUnit = 0x1000;
static const DWORD CabLockQueuedReaderUnit = 0x01000000;

struct CabLockWordsPilot { DWORD low, high; };

static CabLockWordsPilot ReadCabLockWords(CabPoolLockObserved40Pilot* state)
{
    CabLockWordsPilot words;
    words.low = state->stateLow;
    words.high = state->stateHigh;
    return words;
}

static bool ChangeCabLockWords(CabPoolLockObserved40Pilot* state, CabLockWordsPilot expected,
    DWORD nextLow, DWORD nextHigh)
{
    return CompareExchangeCabPair(state, expected.low, expected.high, nextLow, nextHigh,
        g_cabSlotAtomicModePilot > 0);
}

// The original backoff at 20145ca9 takes its mask in ESI.
static DWORD SpinCabPoolLock(DWORD mask)
{
    DWORD timestamp;
    __asm {
        rdtsc
        mov timestamp, eax
    }
    DWORD remaining = timestamp & mask;
    do {
        __asm pause
        --remaining;
    } while (static_cast<LONG>(remaining) >= 0);
    return static_cast<LONG>(mask) <= 0x1ff ? mask * 2 + 1 : mask;
}

static bool GrantCabQueuedReader(CabPoolLockObserved40Pilot* state, CabLockWordsPilot words)
{
    DWORD low = words.low & ~CabLockSpin;
    DWORD high = words.high + CabLockReaderUnit - (low < CabLockQueuedReaderUnit ? 1 : 0);
    return ChangeCabLockWords(state, words, low - CabLockQueuedReaderUnit, high);
}

static DWORD AcquireCabPoolReader(CabPoolLockObserved40Pilot* state, bool alreadyOwner)
{
    if (alreadyOwner && state->ownerTracking == 0) return 0xe0000004;
    CabLockWordsPilot words;
    for (;;) {
        words = ReadCabLockWords(state);
        if (alreadyOwner || (words.low & CabLockReaderBlockers) == 0) {
            if (ChangeCabLockWords(state, words, words.low & ~CabLockSpin, words.high + CabLockReaderUnit))
                return 0;
        } else {
            DWORD high = words.high + (words.low >= 0xff000000 ? 1 : 0);
            if (ChangeCabLockWords(state, words, words.low + CabLockQueuedReaderUnit, high)) break;
        }
    }

    if (g_cabSlotAtomicModePilot > 0 && (words.low & CabLockSpin) != 0) {
        DWORD mask = 0;
        for (int attempt = 0; attempt < 1000; ++attempt) {
            mask = SpinCabPoolLock(mask);
            words = ReadCabLockWords(state);
            if ((words.low & CabLockReaderBlockers) == 0 && GrantCabQueuedReader(state, words)) return 0;
            if ((words.low & CabLockSpin) == 0) break;
        }
    }
    for (;;) {
        DWORD status = WaitForSingleObjectEx(state->readerSemaphore, INFINITE, FALSE);
        if (status != WAIT_OBJECT_0) {
            do {
                words = ReadCabLockWords(state);
            } while (!ChangeCabLockWords(state, words, words.low - CabLockQueuedReaderUnit,
                words.high - (words.low < CabLockQueuedReaderUnit ? 1 : 0)));
            return status;
        }
        for (;;) {
            words = ReadCabLockWords(state);
            if ((words.low & CabLockReaderBlockers) != 0) break;
            if (GrantCabQueuedReader(state, words)) return 0;
        }
    }
}

static bool CabQueuedWriterBlocked(CabLockWordsPilot words, bool upgrade)
{
    DWORD mask = CabLockExclusive | (upgrade ? 0 : CabLockUpgrade);
    return (words.low & mask) != 0 || (words.high & CabLockReaderCount) != 0;
}

static DWORD GrantCabWriterLow(DWORD low, bool upgrade)
{
    DWORD clear = CabLockSpin | (upgrade ? CabLockUpgrade : 0);
    return ((low | CabLockExclusive) - 1) & ~clear;
}

static void RecordCabWriter(CabPoolLockObserved40Pilot* state, DWORD thread, bool upgrade)
{
    if (!upgrade || state->ownerTracking >= 0) state->ownerThread = thread;
}

static void RollBackCabQueuedWriter(CabPoolLockObserved40Pilot* state, bool upgrade)
{
    for (;;) {
        CabLockWordsPilot words = ReadCabLockWords(state);
        DWORD low = words.low - 1;
        DWORD high = words.high;
        if (upgrade) {
            low &= ~CabLockUpgrade;
            high += CabLockReaderUnit;
        }
        if (!ChangeCabLockWords(state, words, low, high)) continue;
        DWORD readers = ((low >> 24) + (high << 8)) & 0xfffff;
        if ((low & CabLockReaderBlockers) == 0 && readers != 0)
            ReleaseSemaphore(state->readerSemaphore, readers, NULL);
        return;
    }
}

static DWORD WaitForCabQueuedWriter(CabPoolLockObserved40Pilot* state, DWORD thread,
    DWORD queuedLow, bool upgrade)
{
    if (g_cabSlotAtomicModePilot > 0 && (queuedLow & CabLockSpin) != 0) {
        DWORD mask = 0;
        for (int attempt = 0; attempt < 1000; ++attempt) {
            mask = SpinCabPoolLock(mask);
            CabLockWordsPilot words = ReadCabLockWords(state);
            if (!CabQueuedWriterBlocked(words, upgrade) &&
                    ChangeCabLockWords(state, words, GrantCabWriterLow(words.low, upgrade), words.high)) {
                RecordCabWriter(state, thread, upgrade);
                return 0;
            }
            if ((words.low & CabLockSpin) == 0) break;
        }
    }
    for (;;) {
        DWORD status = WaitForSingleObjectEx(state->writerEvent, INFINITE, FALSE);
        if (status != WAIT_OBJECT_0) {
            RollBackCabQueuedWriter(state, upgrade);
            return status;
        }
        for (;;) {
            CabLockWordsPilot words = ReadCabLockWords(state);
            if (CabQueuedWriterBlocked(words, upgrade) && ChangeCabLockWords(state, words, words.low, words.high)) {
                if (!upgrade && (words.low & CabLockUpgrade) != 0) SetEvent(state->writerEvent);
                break;
            }
            // The original grant attempt follows even a failed no-change comparison.
            if (ChangeCabLockWords(state, words, GrantCabWriterLow(words.low, upgrade), words.high)) {
                RecordCabWriter(state, thread, upgrade);
                return 0;
            }
        }
    }
}

static DWORD AcquireCabPoolWriter(CabPoolLockObserved40Pilot* state, DWORD thread, bool alreadyOwner)
{
    if (alreadyOwner) {
        if (state->ownerTracking == 0) return 0xe0000004;
        ++state->recursion;
        if ((state->stateLow & CabLockSpin) != 0) {
            // 201471ba masks the expected word. The desired word is the original snapshot.
            for (;;) {
                CabLockWordsPilot words = ReadCabLockWords(state);
                DWORD desiredLow = words.low;
                words.low &= ~CabLockSpin;
                if (ChangeCabLockWords(state, words, desiredLow, words.high)) break;
            }
        }
        return 0;
    }
    for (;;) {
        CabLockWordsPilot words = ReadCabLockWords(state);
        bool canGrant = (words.high & CabLockReaderCount) == 0 &&
            ((words.low & CabLockReaderBlockers) == 0 ||
             ((words.low & CabLockSpin) != 0 && (words.low & (CabLockExclusive | CabLockUpgrade)) == 0));
        if (canGrant) {
            if (ChangeCabLockWords(state, words, (words.low & ~CabLockSpin) | CabLockExclusive, words.high)) {
                RecordCabWriter(state, thread, false);
                return 0;
            }
        } else {
            DWORD queuedLow = words.low + 1;
            if (ChangeCabLockWords(state, words, queuedLow, words.high))
                return WaitForCabQueuedWriter(state, thread, queuedLow, false);
        }
    }
}

static DWORD UpgradeCabPoolReader(CabPoolLockObserved40Pilot* state, DWORD thread, bool alreadyOwner)
{
    for (;;) {
        CabLockWordsPilot words = ReadCabLockWords(state);
        if ((words.high & CabLockReaderCount) == 0) return ERROR_INVALID_PARAMETER;
        DWORD high = words.high - CabLockReaderUnit;
        if (alreadyOwner) {
            if (ChangeCabLockWords(state, words, words.low & ~CabLockSpin, high)) {
                ++state->recursion;
                return 0;
            }
            continue;
        }
        if ((words.low & CabLockUpgrade) != 0) return 0xe0000002;
        if ((words.low & CabLockWriterCount) != 0) return 0xe0000003;
        if ((words.low & CabLockExclusive) == 0 && (high & CabLockReaderCount) == 0) {
            if (ChangeCabLockWords(state, words, (words.low & ~CabLockSpin) | CabLockExclusive, high)) {
                RecordCabWriter(state, thread, true);
                return 0;
            }
        } else {
            DWORD queuedLow = (words.low | CabLockUpgrade) + 1;
            if (ChangeCabLockWords(state, words, queuedLow, high))
                return WaitForCabQueuedWriter(state, thread, queuedLow, true);
        }
    }
}

// Pool-used subset of 20146e07. These callers use an infinite, nonalertable
// wait, no callback and zero policy flags. The original receiver is in EDI.
extern "C" DWORD __stdcall AcquireCabPoolLockForStreamPoolsPilot(void* lock, DWORD mode)
{
    CabPoolLockObserved40Pilot* state = static_cast<CabPoolLockObserved40Pilot*>(lock);
    if (state == NULL || state->signature != CabLockSignature) return ERROR_INVALID_PARAMETER;
    InitializeCabSlotAtomicMode();
    DWORD thread = state->ownerTracking >= 0 ? GetCurrentThreadId() : 0;
    bool alreadyOwner = state->ownerTracking >= 0 && state->ownerThread == thread;
    switch (mode) {
    case 0: return AcquireCabPoolReader(state, alreadyOwner);
    case 1: return AcquireCabPoolWriter(state, thread, alreadyOwner);
    case 3: return UpgradeCabPoolReader(state, thread, alreadyOwner);
    default: return ERROR_INVALID_PARAMETER;
    }
}

// Original 20147aa6, two stack arguments and RET8.
extern "C" DWORD __stdcall ReleaseCabPoolLockPilot(void* lock, int exclusive)
{
    CabPoolLockObserved40Pilot* state = static_cast<CabPoolLockObserved40Pilot*>(lock);
    if (state == NULL || state->signature != CabLockSignature) return ERROR_INVALID_PARAMETER;
    InitializeCabSlotAtomicMode();
    if (exclusive == 0) {
        for (;;) {
            DWORD low = state->stateLow;
            DWORD high = state->stateHigh;
            if ((high & CabLockReaderCount) == 0) return CabLockInvalidRelease;
            DWORD nextHigh = high - 0x1000;
            if (CompareExchangeCabPair(state, low, high, low, nextHigh, g_cabSlotAtomicModePilot > 0)) {
                if ((((low & CabLockExclusive) | nextHigh) & CabLockReaderCount) == 0 &&
                        (low & CabLockWriterCount) != 0)
                    SetEvent(state->writerEvent);
                return 0;
            }
        }
    }

    if (state->ownerTracking >= 0) {
        if (state->ownerThread != GetCurrentThreadId()) return CabLockInvalidRelease;
        if (state->recursion != 0) {
            --state->recursion;
            return 0;
        }
        state->ownerThread = 0;
    }
    for (;;) {
        DWORD low = state->stateLow;
        DWORD high = state->stateHigh;
        if ((low & CabLockExclusive) == 0) return CabLockInvalidRelease;
        DWORD nextLow = low & ~CabLockExclusive;
        if (!CompareExchangeCabPair(state, low, high, nextLow, high, g_cabSlotAtomicModePilot > 0)) continue;
        if ((low & CabLockWriterCount) != 0) {
            if ((high & CabLockReaderCount) == 0) SetEvent(state->writerEvent);
        } else {
            DWORD waiters = ((low >> 24) + (high << 8)) & 0xfffff;
            if (waiters != 0) ReleaseSemaphore(state->readerSemaphore, waiters, NULL);
        }
        return 0;
    }
}

static void FreeCabPoolLockAllocation(CabSlotAllocatorObservedPilot* allocator, void* raw)
{
    if (allocator != NULL) allocator->release(allocator->context, raw);
    else {
        GetProcessHeap();
        HeapFree(GetProcessHeap(), 0, raw);
    }
}

// Original 20147dff, EAX result and no stack arguments.
extern "C" void* __cdecl CreateCabPoolLockPilot()
{
    void* raw;
    CabPoolLockObserved40Pilot* state = static_cast<CabPoolLockObserved40Pilot*>(
        AllocateCabSlotAlignedSourcePilot(NULL, sizeof(CabPoolLockObserved40Pilot), &raw, NULL));
    if (state == NULL) return NULL;
    state->stateLow = 0;
    state->stateHigh = 0;
    state->ownerTracking = 0;
    state->ownerThread = 0;
    state->readerSemaphore = NULL;
    state->writerEvent = NULL;
    state->signature = 0;
    state->recursion = 0;
    state->rawAllocation = raw;
    state->allocator = NULL;
    state->readerSemaphore = CreateSemaphoreA(NULL, 0, 0x7fffffff, NULL);
    if (state->readerSemaphore != NULL) {
        state->writerEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
        if (state->writerEvent != NULL) {
            state->signature = CabLockSignature;
            InitializeCabSlotAtomicMode();
            FinishCabSlotMutation();
            return state;
        }
        CloseHandle(state->readerSemaphore);
    }
    FreeCabPoolLockAllocation(state->allocator, raw);
    return NULL;
}

// Original 20147c77 takes state in EAX. The pool callers ignore the result.
extern "C" DWORD __stdcall DestroyCabPoolLockSourcePilot(void* lock)
{
    InitializeCabSlotAtomicMode();
    CabPoolLockObserved40Pilot* state = static_cast<CabPoolLockObserved40Pilot*>(lock);
    if (state == NULL || InterlockedExchange(&state->signature, 0) != CabLockSignature)
        return ERROR_INVALID_PARAMETER;
    for (;;) {
        DWORD low = state->stateLow;
        DWORD high = state->stateHigh;
        bool busy = (low & 0xff1fffff) != 0 || high != 0;
        DWORD nextLow = busy ? low : low | 0x800000;
        if (!CompareExchangeCabPair(state, low, high, nextLow, high, g_cabSlotAtomicModePilot > 0)) continue;
        if (busy) return 0;
        BOOL eventClosed = CloseHandle(state->writerEvent);
        BOOL semaphoreClosed = CloseHandle(state->readerSemaphore);
        FreeCabPoolLockAllocation(state->allocator, state->rawAllocation);
        FinishCabSlotMutation();
        return eventClosed & semaphoreClosed;
    }
}
