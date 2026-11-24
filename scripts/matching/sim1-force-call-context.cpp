#include "flight_forces.h"

namespace sim1 {
// Backing storage for the original loader-owned gravity value in the comparison image.
__declspec(dllexport) double gravityAcceleration;
}

// Reconstruction-only linked comparison caller for the reviewed flight-step calls.
extern "C" __declspec(dllexport) void __stdcall CompareForceStep(
    sim1::FlightDynamics* flight, double step, int forceMassUpdate)
{
    flight->massProperties->Recalculate(forceMassUpdate);
    flight->forceContributors->Accumulate(step, &flight->force, &flight->moment);
    flight->PrepareAccelerations();
    flight->IntegrateVelocities(step);
}
