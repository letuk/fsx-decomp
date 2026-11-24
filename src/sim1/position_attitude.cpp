#include "flight_dynamics.h"
#include <float.h>
#include <math.h>

namespace sim1 {

#pragma pack(push, 4)
// Reconstructed data prefix reached through api.dll ordinal 304.
struct PublishedPosition {
    char unknown00[0x160];
    unsigned __int64 longitude;
    unsigned __int64 altitude;
};
#pragma pack(pop)

extern "C" __declspec(dllimport) PublishedPosition SimPublishedPosition;

#pragma float_control(precise, on, push)
// Reconstruction-only equivalent of storing a rounded x87 integer and shifting it.
static unsigned __int64 PackCoordinate(double scaled)
{
    const double signedLimit = 9223372036854775808.0;
    if (!(scaled >= -signedLimit && scaled < signedLimit)) {
        // The masked x87 integer-indefinite value becomes zero after the shift.
        return 0;
    }

    const unsigned rounding = _controlfp(0, 0) & _MCW_RC;
    if (rounding == _RC_CHOP) {
        return static_cast<unsigned __int64>(static_cast<__int64>(scaled)) << 16;
    }
    double rounded = rounding == _RC_UP ? ceil(scaled) : floor(scaled);
    if (rounding == _RC_NEAR) {
        const double fraction = scaled - rounded;
        if (fraction > 0.5 ||
            (fraction == 0.5 && (static_cast<__int64>(rounded) & 1) != 0)) {
            rounded += 1.0;
        }
    }
    return static_cast<unsigned __int64>(static_cast<__int64>(rounded)) << 16;
}
#pragma float_control(pop)

// Original: sim1.dll 0x20bd1d90
__declspec(noinline) void FlightDynamics::StoreWorldVelocity(Vector3 velocity)
{
    worldVelocity = velocity;
    horizontalSpeed = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
}

// Original: sim1.dll 0x20bd1bd0
__declspec(noinline) void FlightDynamics::SetLongitude(double radians)
{
    const double pi = 3.141592653589793;
    const double turn = 2.0 * pi;
    if (radians > pi) {
        radians -= turn;
    }
    if (radians < -pi) {
        radians += turn;
    }

    longitude = radians;
    longitudeCosine = cos(radians);
    longitudeSine = sin(radians);
    fixedLongitude = PackCoordinate(radians * 44798133900177.02);
    if (positionDispatch->publishesPosition(this)) {
        SimPublishedPosition.longitude = fixedLongitude;
    }
}

// Original: sim1.dll 0x20bd1c90
__declspec(noinline) void FlightDynamics::SetAltitude(double feet)
{
    if (!(feet < 100000000.0)) {
        feet = 100000000.0;
    }

    altitude = feet;
    fixedAltitude = PackCoordinate(feet * 0.3048 * 65536.0);
    if (positionDispatch->publishesPosition(this)) {
        SimPublishedPosition.altitude = fixedAltitude;
    }

    const double currentAltitude = altitude;
    altitudeAboveReference = currentAltitude - groundReference->dispatch->height(groundReference);
}

}
