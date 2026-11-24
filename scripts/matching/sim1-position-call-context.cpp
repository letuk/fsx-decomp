#include "flight_dynamics.h"

// Reconstruction-only linked comparison caller. The original transform slot
// computes the geographic arguments before invoking the two position writers.
extern "C" __declspec(dllexport) void __stdcall ComparePositionUpdates(
    sim1::FlightDynamics* flight, double altitude, double longitude)
{
    sim1::Vector3 worldVelocity;
    flight->heading->RotateForward(&worldVelocity, &flight->linearVelocity);
    flight->StoreWorldVelocity(worldVelocity);
    flight->SetAltitude(altitude);
    flight->SetLongitude(longitude);
}
