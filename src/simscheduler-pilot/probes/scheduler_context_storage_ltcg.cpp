// Linked projection of the context factory's allocation and cleanup paths.
#include "scheduler_context_storage_projection.h"

extern "C" SchedulerContext* AcquireSchedulerContextStorage();

// Original: simscheduler.dll 0x20cc20b0
static __declspec(noinline) void ReleaseContextPoolStorage(SchedulerContext* storage)
{
    ReturnSchedulerPoolStorage(schedulerContextPool, storage);
}

void* SchedulerContext::operator new(size_t) throw()
{
    return AcquireSchedulerContextStorage();
}

// Original: simscheduler.dll 0x20cc15f0, operator delete mapping inferred.
void SchedulerContext::operator delete(void* storage)
{
    ReleaseContextPoolStorage(static_cast<SchedulerContext*>(storage));
}

// Reconstruction-only projection of the construction branch at 0x20cc1670.
__declspec(dllexport) SchedulerContext* __stdcall ProbeConstructPooledContext(const char* name)
{
    return new SchedulerContext(name);
}

// Original: simscheduler.dll 0x20cc28c0
__declspec(dllexport) SchedulerContext* SchedulerContext::destroy(unsigned int flags)
{
    this->~SchedulerContext();
    if (flags & 1) {
        ReleaseContextPoolStorage(this);
    }
    return this;
}
