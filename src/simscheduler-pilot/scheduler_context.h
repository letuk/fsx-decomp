#ifndef SIMSCHEDULER_SCHEDULER_CONTEXT_H
#define SIMSCHEDULER_SCHEDULER_CONTEXT_H

#if defined(MATCH_NATIVE_CONTEXT)
class SchedulerContext;
#else
struct SchedulerContext;
#endif

struct SchedulerContextTable
{
    long (__stdcall *addReference)(SchedulerContext*);
    long (__stdcall *releaseReference)(SchedulerContext*);
    const char* (__stdcall *getName)(SchedulerContext*);
    int (__stdcall *setStepBounds)(SchedulerContext*, double, double);
    unsigned char precedingSlots[0x14];
    void (__stdcall *setPrimaryMode)(SchedulerContext*, unsigned int);
    int (__stdcall *shouldNotifyTimeObserver)(SchedulerContext*);
    void (__stdcall *setUpdateEnabled)(SchedulerContext*, unsigned int);
    void (__stdcall *setSecondaryMode)(SchedulerContext*, int);
    void (__stdcall *setTertiaryMode)(SchedulerContext*, int);
    int (__stdcall *addPendingEntry)(SchedulerContext*, void*, int);
    int (__stdcall *removePendingEntry)(SchedulerContext*, void*);
    unsigned char beforeReadTime[4];
    double* (__stdcall *readTime)(SchedulerContext*, double*);
    unsigned char beforeDeletingWrapper[8];
    SchedulerContext* (__thiscall *deleteContext)(SchedulerContext*, unsigned int);
};

struct SchedulerEntryCallback;

struct SchedulerEntryCallbackTable
{
    void (__stdcall *advance)(SchedulerEntryCallback*, double);
};

struct SchedulerEntryCallback
{
    const SchedulerEntryCallbackTable* table;
};

#pragma pack(push, 4)
struct SchedulerTimedEntry
{
    unsigned int flags;
    double time;
    SchedulerEntryCallback* callback;
};

struct SchedulerPendingEntry
{
    int add;
    void* entry;
    int flags;
    int classification;
};

struct SchedulerPendingQueue
{
    SchedulerPendingQueue() : count(0), capacity(0), growth(5), flags(4), records(0) {}
    ~SchedulerPendingQueue();

    int count;
    int capacity;
    short growth;
    unsigned short flags;
    SchedulerPendingEntry* records;
};

struct SchedulerEntryIndexRecord
{
    void* key;
    SchedulerTimedEntry* entry;
};

struct SchedulerEntryIndex
{
    SchedulerEntryIndex() : count(0), capacity(0), growth(5), flags(4), records(0) {}
    ~SchedulerEntryIndex();

    unsigned int count;
    int capacity;
    short growth;
    unsigned short flags;
    SchedulerEntryIndexRecord* records;
};

struct SchedulerLiveEntryList
{
    SchedulerLiveEntryList() : count(0), capacity(0), growth(5), flags(4), records(0) {}
    ~SchedulerLiveEntryList();

    int count;
    int capacity;
    short growth;
    unsigned short flags;
    SchedulerTimedEntry** records;
};

#if defined(MATCH_NATIVE_CONTEXT)
class SchedulerContext
{
public:
#else
struct SchedulerContext
{
#endif
    struct ModeBits
    {
        unsigned int active : 3;
    };

    struct UpdateBits
    {
        unsigned int active : 2;
    };

    SchedulerContext(const char* contextName);
#if defined(MATCH_NATIVE_CONTEXT)
    virtual long __stdcall AddReference();
    virtual long __stdcall ReleaseReference();
    virtual const char* __stdcall GetName();
    virtual int __stdcall SetStepBounds(double minimum, double maximum);
    virtual int __stdcall SetMinimumStep(double step);
    virtual int __stdcall SetMaximumStep(double step);
    virtual double __stdcall GetMinimumStep();
    virtual double __stdcall GetMaximumStep();
    virtual int __stdcall GetPrimaryMode();
    virtual void __stdcall SetPrimaryMode(unsigned int enabled);
    virtual int __stdcall GetUpdateEnabled();
    virtual void __stdcall SetUpdateEnabled(unsigned int enabled);
    virtual void __stdcall SetSecondaryMode(int enabled);
    virtual void __stdcall SetTertiaryMode(int enabled);
    virtual int __stdcall RegisterEntry(void* key, int flags);
    virtual int __stdcall UnregisterEntry(void* key);
    virtual int __stdcall Reset();
    virtual double* __stdcall ReadTime(double* result);
    virtual int __stdcall ReadSubsteps(double* step, int* count);
    virtual SchedulerContext* __stdcall GetContext();
    virtual ~SchedulerContext();
    static void operator delete(void* storage);
#else
    ~SchedulerContext();
    SchedulerContext* destroy(unsigned int flags);
    const SchedulerContextTable* table;
#endif
    long referenceCount;
    char name[0x20];
    double minimumStep;
    double maximumStep;
    union {
        unsigned int mode;
        ModeBits modeBits;
    };
    union {
        unsigned int flags;
        UpdateBits updateBits;
    };
    int cursor;
    double time;
    double remainder;
    double substep;
    int substepCount;
    SchedulerLiveEntryList liveEntries;
    SchedulerEntryIndex index;
    SchedulerPendingQueue pending;
};
#pragma pack(pop)

#endif
