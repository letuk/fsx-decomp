#ifndef SIMSCHEDULER_SCHEDULER_OBJECT_H
#define SIMSCHEDULER_SCHEDULER_OBJECT_H

class SchedulerObject;
struct SchedulerTimeSource;

struct SchedulerVTable
{
    unsigned char precedingSlots[0x0c];
    void (__stdcall *writeMaximumSimSpeed)(SchedulerObject*, int);
    int (__stdcall *readMaximumSimSpeed)(SchedulerObject*);
    void (__stdcall *writeSimSpeed)(SchedulerObject*, int);
    int (__stdcall *readSimSpeed)(SchedulerObject*);
    int (__stdcall *readFieldMinimum)(SchedulerObject*);
    double* (__stdcall *readSimTime)(SchedulerObject*, double*);
    double* (__stdcall *readLastScaledStep)(SchedulerObject*, double*);
    double (__stdcall *readLastInputStep)(SchedulerObject*);
    void (__stdcall *advanceTime)(SchedulerObject*, double);
};

#pragma pack(push, 4)
class SchedulerObject
{
public:
    // The original table passes this on the stack for these eight slots.
    void __stdcall SetMaximumSimSpeed(int speed);
    int __stdcall GetMaximumSimSpeed();
    void __stdcall SetSimSpeed(int speed);
    int __stdcall GetSimSpeed();
    int __stdcall GetEffectiveSimSpeed();
    double* __stdcall ReadSimTime(double* result);
    double* __stdcall ReadLastScaledStep(double* result);
    double __stdcall GetLastInputStep();

    const SchedulerVTable* table;
    double time;
    double lastScaledStep;
    double lastInputStep;
    int simSpeed;
    int maximumSimSpeed;
    int timeStateActive;
    double savedTime;
    int sourceCount;
    unsigned char unknown34[8];
    SchedulerTimeSource* sources;
};
#pragma pack(pop)

#endif
