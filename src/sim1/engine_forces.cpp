#include "flight_dynamics.h"

#include <math.h>

namespace sim1 {

// Reconstructed pair of float orientation angles at engine mount +0x18.
struct ThrustOrientation {
    float elevation;
    float lateralAngle;
};

#pragma pack(push, 4)

struct TurbineEngineState;

// Reconstructed mount record in the configuration array at +0x199c.
struct EngineMount {
    Vector3 position;
    ThrustOrientation orientation;
};

// Reconstructed prefixes and verified record strides of the propulsion arrays.
struct EngineForceRecord {
    void UpdateTurbineSpeedOutput(TurbineEngineState* turbine) const;

    const EngineMount* mount;
    char unknown04[0x30];
    double thrust;
    char unknown3c[0x20];
    double speedFraction;
    char unknown64[0x88];
};

// Reconstructed turbine state with the verified per-engine stride.
struct TurbineEngineState {
    char unknown00[0x50];
    double speedScaledOutput;
    char unknown58[0x28];
};

#pragma pack(pop)

typedef char CheckEngineMountOrientation[offsetof(EngineMount, orientation) == 0x18 ? 1 : -1];
typedef char CheckEngineForceStride[sizeof(EngineForceRecord) == 0xec ? 1 : -1];
typedef char CheckEngineThrust[offsetof(EngineForceRecord, thrust) == 0x34 ? 1 : -1];
typedef char CheckTurbineSpeed[offsetof(EngineForceRecord, speedFraction) == 0x5c ? 1 : -1];
typedef char CheckTurbineOutput[offsetof(TurbineEngineState, speedScaledOutput) == 0x50 ? 1 : -1];
typedef char CheckTurbineStateStride[sizeof(TurbineEngineState) == 0x80 ? 1 : -1];
typedef char CheckPropellerForceStride[sizeof(PropellerForceRecord) == 0x54 ? 1 : -1];
typedef char CheckPropellerMountOffset[offsetof(PropellerForceRecord, mountOffset) == 0x4c ? 1 : -1];
typedef char CheckMomentSuppression[offsetof(PropulsionConfiguration, suppressFirstMoment) == 0x1444 ? 1 : -1];
typedef char CheckPropulsionState[offsetof(IntegrationHistory, engineForces) == 0x43c ? 1 : -1];
typedef char CheckPropulsionForce[offsetof(IntegrationHistory, propulsionForce) == 0x6c ? 1 : -1];
typedef char CheckPropulsionMoment[offsetof(IntegrationHistory, propulsionMoment) == 0xa4 ? 1 : -1];
typedef char CheckMomentMode[offsetof(IntegrationHistory, propulsionMomentBlendMode) == 0x210 ? 1 : -1];
typedef char CheckMomentControl[offsetof(FlightDynamics, propulsionMomentControl) == 0x2d8 ? 1 : -1];
typedef char CheckMomentBlendInput[offsetof(FlightDynamics, propulsionMomentBlendInput) == 0x1f4 ? 1 : -1];

#if defined(SIM1_PROPULSION_COMPARISON) || defined(SIM1_THRUST_COMPARISON)
// Reconstruction-only backing storage for the loader-owned gravity value.
__declspec(dllexport) double gravityAcceleration;
#else
extern double gravityAcceleration;
#endif

// Original: sim1.dll 0x20c01010
static __declspec(noinline) void __stdcall AccumulateEngineThrust(
    const ThrustOrientation* orientation, double thrust,
    Vector3& forceSum, Vector3& momentSum, const Vector3& leverArm)
{
    Vector3 force;
    if (orientation->elevation == 0.0f && orientation->lateralAngle == 0.0f) {
        force.x = 0.0;
        force.y = 0.0;
        force.z = thrust;
    } else {
        const float elevationCosine = static_cast<float>(cos(orientation->elevation));
        const double horizontalThrust = elevationCosine * thrust;
        const float lateralSine = static_cast<float>(sin(orientation->lateralAngle));
        force.x = horizontalThrust * lateralSine;
        const float verticalSine = static_cast<float>(sin(-orientation->elevation));
        force.y = thrust * verticalSine;
        const float lateralCosine = static_cast<float>(cos(orientation->lateralAngle));
        force.z = horizontalThrust * lateralCosine;
    }

    forceSum.x += force.x;
    forceSum.y += force.y;
    forceSum.z += force.z;

    momentSum.x += leverArm.y * force.z - leverArm.z * force.y;
    momentSum.y += leverArm.z * force.x - leverArm.x * force.z;
    momentSum.z += leverArm.x * force.y - leverArm.y * force.x;
}

// Original: sim1.dll 0x20c010f0
void FlightDynamics::CollectPropulsionForces(double, Vector3* forceOutput, Vector3* momentOutput)
{
    IntegrationHistory* state = history;
    const PropulsionConfiguration* initialConfiguration = state->propulsionConfiguration;
    Vector3& force = state->propulsionForce;
    Vector3& moment = state->propulsionMoment;
    const ModelPoint& center = massProperties->centerOfGravity;

    force.x = 0.0;
    force.y = 0.0;
    force.z = 0.0;
    moment.x = 0.0;
    moment.z = 0.0;
    moment.y = 0.0;

    if (initialConfiguration->engineCount != 0) {
        unsigned index = 0;
        do {
            const EngineForceRecord& engine = state->engineForces[index];
            if (engine.thrust != 0.0) {
                const EngineMount& mount = *engine.mount;
                const Vector3 leverArm = {
                    mount.position.x - center.x,
                    mount.position.y - center.y,
                    mount.position.z - center.z
                };
                AccumulateEngineThrust(&mount.orientation, engine.thrust, force, moment, leverArm);
            }
        } while (++index < state->propulsionConfiguration->engineCount);
    }

    for (unsigned index = 0; index < state->propulsionConfiguration->propellerCount; ++index) {
        const unsigned engineIndex = state->propulsionConfiguration->engineForPropeller[index];
        const EngineMount& mount = *state->engineForces[engineIndex].mount;
        const PropellerForceRecord& propeller = state->propellerForces[index];
        Vector3 leverArm = {
            mount.position.x - center.x,
            mount.position.y - center.y,
            mount.position.z - center.z
        };
        if (propeller.mountOffset != 0) {
            leverArm.x += propeller.mountOffset->x;
            leverArm.y += propeller.mountOffset->y;
            leverArm.z += propeller.mountOffset->z;
        }
        AccumulateEngineThrust(&mount.orientation, propeller.thrust, force, moment, leverArm);
    }

    if (state->propulsionConfiguration->suppressFirstMoment || propulsionMomentControl->suppressFirstMoment) {
        moment.x = 0.0;
    }
    state->thrustToWeight = force.z / (massProperties->mass * gravityAcceleration);

    if (state->propulsionMomentBlendMode == 2) {
        const double scale = propulsionMomentBlendInput / 3.0;
        const double weight = scale > 0.0 ? (scale < 1.0 ? scale : 1.0) : 0.0;
        moment.y *= weight;
    }

    *forceOutput = force;
    *momentOutput = moment;
}

// Original: sim1.dll 0x20c2bc20
void EngineForceRecord::UpdateTurbineSpeedOutput(TurbineEngineState* turbine) const
{
    if (speedFraction > 0.5) {
        turbine->speedScaledOutput = 120.0 * speedFraction;
        return;
    }
    if (speedFraction > 0.2) {
        turbine->speedScaledOutput = 80.0 * speedFraction;
        return;
    }
    turbine->speedScaledOutput = 10.0 * speedFraction;
}

#ifdef SIM1_THRUST_COMPARISON
// Reconstruction-only inputs for the observed per-engine accumulation call.
struct ThrustSample {
    Vector3 mountPosition;
    ThrustOrientation orientation;
    double thrust;
};

// Reconstruction-only LTCG call context for the force-collector projection.
extern "C" __declspec(dllexport) void __stdcall CompareThrustCollection(
    const ThrustSample* samples, unsigned count, const ModelPoint& centerOfMass,
    Vector3& forceSum, Vector3& momentSum)
{
    for (unsigned index = 0; index < count; ++index) {
        const ThrustSample& sample = samples[index];
        if (sample.thrust != 0.0) {
            const Vector3 leverArm = {
                sample.mountPosition.x - centerOfMass.x,
                sample.mountPosition.y - centerOfMass.y,
                sample.mountPosition.z - centerOfMass.z
            };
            AccumulateEngineThrust(&sample.orientation, sample.thrust, forceSum, momentSum, leverArm);
        }
    }
}
#endif

}
