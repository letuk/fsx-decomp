#include "scheduler_object.h"

void SchedulerObject::SetMaximumSimSpeed(int speed)
{
    maximumSimSpeed = speed;
}

int SchedulerObject::GetMaximumSimSpeed()
{
    return maximumSimSpeed;
}

void SchedulerObject::SetSimSpeed(int speed)
{
    simSpeed = speed;
}

int SchedulerObject::GetSimSpeed()
{
    return simSpeed;
}

int SchedulerObject::GetEffectiveSimSpeed()
{
    return simSpeed < maximumSimSpeed ? simSpeed : maximumSimSpeed;
}

double* SchedulerObject::ReadSimTime(double* result)
{
    *result = time;
    return result;
}

double* SchedulerObject::ReadLastScaledStep(double* result)
{
    *result = lastScaledStep;
    return result;
}

double SchedulerObject::GetLastInputStep()
{
    return lastInputStep;
}
