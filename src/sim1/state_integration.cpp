#include "flight_dynamics.h"

namespace sim1 {

// Loader-owned value at sim1.dll 0x20c7c180.
extern double gravityAcceleration;

// Reconstruction-only helper. The original emits the clamp in each axis update.
static __forceinline double ClampComponent(double value, double limit)
{
    if (value <= -limit) {
        return -limit;
    }
    return value < limit ? value : limit;
}

// Original: sim1.dll 0x20c42d90
void FlightDynamics::PrepareAccelerations()
{
    IntegrationHistory& state = *history;
    state.previousLinearAcceleration = linearAcceleration;

    const double mass = massProperties->mass;
    const double gravity = longitudinalGravityFactor * gravityAcceleration;
    const float gravityZ = static_cast<float>(verticalGravityFactor * gravityAcceleration);
    const float gravityY = static_cast<float>(-gravityFactorY * gravity);
    const float gravityX = static_cast<float>(-gravityFactorX * gravity);
    Vector3 acceleration;
    acceleration.x = ClampComponent((angularVelocity.z * linearVelocity.y + force.x / mass) -
                                    linearVelocity.z * angularVelocity.y + gravityX, 216.7);
    acceleration.y = ClampComponent((linearVelocity.z * angularVelocity.x + force.y / mass) -
                                    angularVelocity.z * linearVelocity.x + gravityY, 216.7);
    acceleration.z = ClampComponent((angularVelocity.y * linearVelocity.x + force.z / mass) -
                                    angularVelocity.x * linearVelocity.y + gravityZ, 216.7);
    linearAcceleration = acceleration;
    heading->RotateForward(&state.transformedAcceleration, &acceleration);

    state.previousAngularAcceleration = angularAcceleration;
    const MassProperties& massData = *massProperties;
    const double ix = massData.inertiaX;
    const double iy = massData.inertiaY;
    const double iz = massData.inertiaZ;
    const double cross = massData.crossInertia;
    const double wx = angularVelocity.x;
    const double wy = angularVelocity.y;
    const double wz = angularVelocity.z;

    const double ax = ((wz * wz - wy * wy) * cross + (iz - iy) * wy * wz + moment.x) /
                      (ix * state.inertiaScale);
    const double az = (((ix - iz) * wx * wy + moment.z) -
                       (wz * wx + state.previousAngularAcceleration.y) * cross) /
                      (iy * state.inertiaScale);
    const double ay = ((wy * wx - state.previousAngularAcceleration.z) * cross +
                       (iy - ix) * wz * wx + moment.y) / (iz * state.inertiaScale);
    angularAcceleration.x = ClampComponent(ax, 5.23);
    angularAcceleration.y = ClampComponent(ay, 5.23);
    angularAcceleration.z = ClampComponent(az, 5.23);
}

// Original: sim1.dll 0x20c43110
void FlightDynamics::IntegrateVelocities(double step)
{
    IntegrationHistory& state = *history;
    int limited = 0;
    if (resetAccelerationHistory != 0) {
        state.previousLinearAcceleration = linearAcceleration;
        state.previousAngularAcceleration = angularAcceleration;
    }

    const double vz = linearVelocity.z + (linearAcceleration.z + state.previousLinearAcceleration.z) * 0.5 * step;
    const double vx = linearVelocity.x + (linearAcceleration.x + state.previousLinearAcceleration.x) * 0.5 * step;
    const double vy = linearVelocity.y + (linearAcceleration.y + state.previousLinearAcceleration.y) * 0.5 * step;
    linearVelocity.x = ClampComponent(vx, 4500.0);
    linearVelocity.y = ClampComponent(vy, 4500.0);
    linearVelocity.z = ClampComponent(vz, 4500.0);

    double wx = angularVelocity.x + (angularAcceleration.x + state.previousAngularAcceleration.x) * 0.5 * step;
    double wz = angularVelocity.z + (angularAcceleration.z + state.previousAngularAcceleration.z) * 0.5 * step;
    double wy = angularVelocity.y + (angularAcceleration.y + state.previousAngularAcceleration.y) * 0.5 * step;
    if (wx > 4.363) {
        wx = 4.363;
        limited = 1;
    } else if (wx < -4.363) {
        wx = -4.363;
        limited = 1;
    }
    if (wz > 7.33) {
        wz = 7.33;
        limited = 1;
    } else if (wz < -7.33) {
        wz = -7.33;
        limited = 1;
    }
    if (wy > 3.491) {
        wy = 3.491;
        limited = 1;
    } else if (wy < -3.491) {
        wy = -3.491;
        limited = 1;
    }
    angularVelocity.x = wx;
    angularVelocity.y = wy;
    angularVelocity.z = wz;

    double scale = state.inertiaScale;
    if (limited) {
        scale *= 1.01;
    } else {
        scale *= 0.98;
        state.inertiaScale = scale;
        if (!(scale > 1.0)) {
            scale = 1.0;
        }
    }
    state.inertiaScale = scale;

    const double halfPi = 1.5707963267948966;
    double cosine = state.rateCosine;
    double sine = state.rateSine;
    if (!(state.attitudeAngle >= -halfPi) || state.attitudeAngle > halfPi) {
        cosine = -cosine;
        sine = -sine;
    }
    state.transformedAngularVelocity.x = wx;
    state.transformedAngularVelocity.y = cosine * wz - sine * wy;
    state.transformedAngularVelocity.z = cosine * wy + wz * sine;
}

// Original: sim1.dll 0x20bdeec0, heading transform slot +0x04
void HeadingTransform::RotateForward(Vector3* output, const Vector3* input)
{
    Vector3 snapshot;
    if (output == input) {
        snapshot = *input;
        input = &snapshot;
    }
    output->y = input->y;
    output->x = sineHeading * input->z + cosineHeading * input->x;
    output->z = cosineHeading * input->z - sineHeading * input->x;
}

// Original: sim1.dll 0x20bdef30, heading transform slot +0x08
void HeadingTransform::RotateInverse(Vector3* output, const Vector3* input)
{
    Vector3 snapshot;
    if (input == output) {
        snapshot = *input;
        input = &snapshot;
    }
    output->y = input->y;
    output->x = cosineHeading * input->x - sineHeading * input->z;
    output->z = cosineHeading * input->z + sineHeading * input->x;
}

// Original: sim1.dll 0x20bdefa0, heading transform slot +0x0c
void HeadingTransform::Refresh(const FlightDynamics* dynamics)
{
    sineHeading = static_cast<float>(dynamics->headingSine);
    cosineHeading = static_cast<float>(dynamics->headingCosine);
}

}
