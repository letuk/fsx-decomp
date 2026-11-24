// Matching-only program for compiler-assigned handler calling conventions.
#if defined(MATCH_TIME_STATE_WORDS) || defined(MATCH_TIME_STATE_COPY) || defined(MATCH_PROPAGATION_COPY)
#include <string.h>
#endif

#if defined(MATCH_TIME_STATE_WORDS)
struct SimTimeValue
{
    unsigned long low;
    unsigned long high;
};

static double SimTimeAsDouble(const SimTimeValue& value)
{
    double time;
    memcpy(&time, &value, sizeof(time));
    return time;
}
#else
typedef double SimTimeValue;

static double SimTimeAsDouble(double value)
{
    return value;
}
#endif

struct SchedulerState;
struct TimeGroup;

struct TimeSource;
struct TimeSourceTable
{
    unsigned char precedingSlots[0x4c];
    TimeGroup* (__stdcall *getTimeGroup)(TimeSource*);
};

struct TimeSource
{
    const TimeSourceTable* table;
};

#pragma pack(push, 4)
struct TimedEntry
{
    unsigned long unknown;
    double time;
};

struct TimeGroup
{
    unsigned char unknown00[0x44];
    double time;
    unsigned char unknown4c[0x14];
    int entryCount;
    unsigned char unknown64[8];
    TimedEntry** entries;
};

struct TimeSourceEntry
{
    unsigned char unknown[0x20];
    TimeSource* source;
};

struct SchedulerStateTable
{
    unsigned char precedingSlots[0x14];
    void (__stdcall *writeSimSpeed)(SchedulerState*, int);
    int (__stdcall *readSimSpeed)(SchedulerState*);
    int (__stdcall *readFieldMinimum)(SchedulerState*);
    SimTimeValue* (__stdcall *readSimTime)(SchedulerState*, SimTimeValue*);
};

struct SchedulerState
{
    const SchedulerStateTable* table;
    double time;
    unsigned char unknown0c[0x18];
    int timeStateActive;
    SimTimeValue savedTime;
    int sourceCount;
    unsigned char unknown34[8];
    TimeSourceEntry* sources;
};
#pragma pack(pop)

struct FlightField
{
    const char* name;
    void* value;
    int count;
    int type;
    int flags;
    int reserved;
};

typedef int (__stdcall *FlightTransfer)(const char*, FlightField*, int, void*);

// Exported storage keeps the probe's unresolved runtime dependencies visible.
__declspec(dllexport) SchedulerState schedulerObject;
#if !defined(MATCH_FLIGHT_LOAD_IMPORT)
__declspec(dllexport) FlightTransfer readFlightState;
#endif
#if !defined(MATCH_FLIGHT_SAVE_IMPORT)
__declspec(dllexport) FlightTransfer writeFlightState;
#endif
#if defined(MATCH_FLIGHT_LOAD_IMPORT)
extern "C" __declspec(dllimport) int __stdcall readFlightState(
    const char*, FlightField*, int, void*);
#endif
#if defined(MATCH_FLIGHT_SAVE_IMPORT)
extern "C" __declspec(dllimport) int __stdcall WriteFlightState(
    const char*, FlightField*, int, void*);
#endif

#if defined(MATCH_PROPAGATION_FRAME)
#pragma optimize("y", off)
static __declspec(noinline) void __stdcall ApplySchedulerTime(SchedulerState* object, double time)
#else
static __declspec(noinline) void ApplySchedulerTime(SchedulerState* object, double time)
#endif
{
#if defined(MATCH_PROPAGATION_COPY)
    memcpy(&object->time, &time, sizeof(time));
#else
    object->time = time;
#endif
    for (int sourceIndex = 0; sourceIndex < object->sourceCount; ++sourceIndex) {
        TimeSource* source = object->sources[sourceIndex].source;
        TimeGroup* group = source->table->getTimeGroup(source);
        for (int entryIndex = 0; entryIndex < group->entryCount; ++entryIndex) {
#if defined(MATCH_PROPAGATION_COPY)
            memcpy(&group->entries[entryIndex]->time, &time, sizeof(time));
#else
            group->entries[entryIndex]->time = time;
#endif
        }
#if defined(MATCH_PROPAGATION_COPY)
        memcpy(&group->time, &time, sizeof(time));
#else
        group->time = time;
#endif
    }
}
#if defined(MATCH_PROPAGATION_FRAME)
#pragma optimize("y", on)
#endif

#if defined(MATCH_TIME_STATE)
#pragma optimize("y", off)
#endif
static __declspec(noinline) void UpdateSchedulerTimeState(SchedulerState* object, int active)
{
    if (active) {
        if (!object->timeStateActive) {
            SimTimeValue time;
            const SimTimeValue* currentTime = object->table->readSimTime(object, &time);
#if defined(MATCH_TIME_STATE_COPY)
            memcpy(&object->savedTime, currentTime, sizeof(object->savedTime));
#else
            object->savedTime = *currentTime;
#endif
        }
    } else if (object->timeStateActive) {
        ApplySchedulerTime(object, SimTimeAsDouble(object->savedTime));
    }
    object->timeStateActive = active;
}
#if defined(MATCH_TIME_STATE)
#pragma optimize("y", on)

// Keep the object input live across the link as a separate ABI experiment.
__declspec(dllexport) void __stdcall ProbeSchedulerTimeState(
    SchedulerState* object, int active)
{
    UpdateSchedulerTimeState(object, active);
}
#endif

#if defined(MATCH_FLIGHT_LOAD_IMPORT)
#pragma optimize("y", off)
#endif
static __declspec(noinline) void LoadSchedulerFlightState(void* payload)
{
    double time = 0.0;
    FlightField timeFields[] = { { "SimTime", &time, 1, 0x13, 0, 0 }, { 0 } };
    if (readFlightState("SimScheduler", timeFields, 0, payload) &&
        readFlightState("DateTimeSeason", timeFields, 0, payload)) {
        time = 0.0;
    }
    ApplySchedulerTime(&schedulerObject, time);

    int speed = 0x100;
    FlightField speedFields[] = { { "SimSpeed", &speed, 1, 0x16, 0x5b, 0 }, { 0 } };
    if (readFlightState("SimScheduler", speedFields, 0, payload) &&
        readFlightState("Options", speedFields, 0, payload)) {
        speed = 0x100;
    }
    schedulerObject.table->writeSimSpeed(&schedulerObject, speed);
}
#if defined(MATCH_FLIGHT_LOAD_IMPORT)
#pragma optimize("y", on)
#endif

#if defined(MATCH_FLIGHT_SAVE_FRAME)
#pragma optimize("y", off)
#endif
static __declspec(noinline) void __stdcall SaveSchedulerFlightState(void* payload)
{
    SimTimeValue buffer;
    double time = SimTimeAsDouble(*schedulerObject.table->readSimTime(&schedulerObject, &buffer));
    int speed = schedulerObject.table->readSimSpeed(&schedulerObject);
    FlightField fields[] = {
        { "SimSpeed", &speed, 1, 0x16, 0x5b, 0 },
        { "SimTime", &time, 1, 0x13, 0, 0 },
        { 0 }
    };
#if defined(MATCH_FLIGHT_SAVE_IMPORT)
    WriteFlightState("SimScheduler", fields, 0, payload);
#else
    writeFlightState("SimScheduler", fields, 0, payload);
#endif
}
#if defined(MATCH_FLIGHT_SAVE_FRAME)
#pragma optimize("y", on)
#endif

#pragma optimize("y", off)
// Original: simscheduler.dll 0x20cc1400
__declspec(dllexport) void __stdcall SchedulerGroup10Callback(unsigned int event, void* payload)
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
