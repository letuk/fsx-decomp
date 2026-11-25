#ifndef SIM1_FLIGHT_DYNAMICS_H
#define SIM1_FLIGHT_DYNAMICS_H

#include <stddef.h>

namespace sim1 {

#pragma pack(push, 4)

struct Vector3 {
    double x;
    double y;
    double z;
};

// Common header of the original variable-length aerodynamic curves.
struct AerodynamicCurve {
    unsigned count;
};

class FlightDynamics;
struct GroundReference;
struct ForceContributors;
struct EngineForceRecord;
struct PitchMomentModifier;
struct ZeroAlphaMomentTuning;
struct AirplaneControlSurfaces;
struct AirplaneGeometry;
class SupplementalAerodynamics;
class AirplaneDynamics;

// Minimum propulsion layouts shared by the thrust and aerodynamic calculations.
struct PropellerForceRecord {
    double signedRpm;
    char unknown08[0x0c];
    double thrust;
    char unknown1c[0x30];
    const Vector3* mountOffset;
    char unknown50[4];
};

struct PropulsionConfiguration {
    char unknown00[0x214];
    double propellerMomentOfInertia;
    char unknown21c[0x238];
    unsigned engineCount;
    unsigned propellerCount;
    unsigned engineForPropeller[4];
    char unknown46c[0x10c];
    AerodynamicCurve elevatorTrimPressureCurve;
    char unknown57c[0xbcc];
    double elevatorEffectiveness;
    char unknown1150[0x10];
    double elevatorTrimEffectiveness;
    char unknown1168[0x10];
    double pitchStability;
    char unknown1180[0x2c];
    float gyroPrecessionOnPitch;
    char unknown11b0[0x294];
    int suppressFirstMoment;
};

struct PropulsionMomentControl {
    const FlightDynamics* owner;
    int simpleAerodynamics;
    char unknown08[8];
    float propellerGyroGain;
    char unknown14[0x0c];
    int suppressPropellerGyro;
};

// Minimum machine layout for verified position, mass and moment calls.
// The identities and signatures of the intervening slots are unresolved.
struct FlightPositionDispatch {
    void* interveningSlots[20];
    int (__thiscall* publishesPosition)(FlightDynamics*);
    void* slots54ToE0[36];
    void (__thiscall* massPropertiesChanged)(FlightDynamics*);
    void* slotsE8To114[12];
    const AirplaneControlSurfaces* (__thiscall* controlSurfaces)(const AirplaneDynamics*);
    float (__thiscall* firstMomentReferenceZ)(const FlightDynamics*);
    float (__thiscall* secondMomentReferenceZ)(const FlightDynamics*);
};

struct GroundReferenceDispatch {
    void* interveningSlots[2];
    double (__thiscall* height)(GroundReference*);
};

struct GroundReference {
    const GroundReferenceDispatch* dispatch;
};

// Reconstructed prefix of the concrete table at sim1.dll 0x20bbdb1c.
class HeadingTransform {
public:
    virtual void AdvancePositionAndAttitude(FlightDynamics* dynamics, double step);
    virtual void RotateForward(Vector3* output, const Vector3* input);
    virtual void RotateInverse(Vector3* output, const Vector3* input);
    virtual void Refresh(const FlightDynamics* dynamics);

private:
    float sineHeading;
    float cosineHeading;
};

// Reconstructed minimum layout of the state reached through FlightDynamics+0x270.
struct IntegrationHistory {
    void __fastcall UpdateAngleOfAttackRate(const FlightDynamics& airplane);

    PropulsionConfiguration* propulsionConfiguration;
    char unknown04[0x10];
    double thrustToWeight;
    char unknown1c[0x50];
    Vector3 propulsionForce;
    char unknown84[0x20];
    Vector3 propulsionMoment;
    Vector3 previousLinearAcceleration;
    Vector3 transformedAcceleration;
    Vector3 previousAngularAcceleration;
    Vector3 airRelativeVelocity;
    char unknown11c[0x18];
    Vector3 transformedAngularVelocity;
    char unknown14c[0x40];
    double attitudeAngle;
    double rawAerodynamicAngleDegrees;
    double aerodynamicAngleDegrees;
    char unknown1a4[8];
    double rateCosine;
    double rateSine;
    char unknown1bc[0x10];
    double angleOfAttackRate;
    char unknown1d4[8];
    double dynamicPressure;
    char unknown1e4[0x10];
    double aerodynamicMomentScale;
    double slipstreamDynamicPressure;
    char unknown204[0x0c];
    int propulsionMomentBlendMode;
    char unknown214[0x3c];
    const PitchMomentModifier* pitchMomentModifier;
    char unknown254[0x1e4];
    PropellerForceRecord* propellerForces;
    EngineForceRecord* engineForces;
    char unknown440[0xb8];
    double inertiaScale;
    char unknown500[0x1e4];
    double flapDeflectionFactor;
    char unknown6ec[0x74];
    double pitchRateMomentScale;
    double slipstreamPitchRateMomentScale;
};

struct ModelPoint {
    float x;
    float y;
    float z;
};

struct WeightEntry {
    const float* weight;
    const ModelPoint* position;
};

// Reconstructed minimum layout of the accumulator at FlightDynamics+0x2f8.
struct MassProperties {
    void Recalculate(int forceUpdate);

    char unknown00[4];
    FlightDynamics* owner;
    char unknown08[4];
    int dirty;
    float totalWeight;
    float mass;
    ModelPoint weightedPosition;
    ModelPoint centerOfGravity;
    float inertiaX;
    float inertiaY;
    float inertiaZ;
    float crossInertia;
    float inertiaZX;
    float inertiaXY;
    float baseInertiaX;
    float baseInertiaY;
    float baseInertiaZ;
    float baseInertiaYZ;
    float baseInertiaZX;
    float baseInertiaXY;
    unsigned entryCount;
    char unknown64[8];
    WeightEntry* const* entries;
};

// Reconstructed minimum layout. Axis and frame names reflect the observed equations.
class FlightDynamics {
public:
    void PrepareAccelerations();
    void IntegrateVelocities(double step);
    void StoreWorldVelocity(Vector3 velocity);
    void SetLongitude(double radians);
    void SetAltitude(double feet);
#ifdef SIM1_PROPULSION_COMPARISON
    __declspec(dllexport)
#endif
    void CollectPropulsionForces(double step, Vector3* forceOutput, Vector3* momentOutput);

    const FlightPositionDispatch* positionDispatch;
    char unknown04[0x10];
    GroundReference* groundReference;
    char unknown18[0x0c];
    HeadingTransform* heading;
    char unknown28[0x10];
    float realismWeight;
    char unknown3c[0x80];
    int resetAccelerationHistory;
    double pitchRadians;
    double longitudinalGravityFactor;
    double verticalGravityFactor;
    char unknownd8[0x08];
    double gravityFactorY;
    double gravityFactorX;
    char unknownf0[0x08];
    double headingCosine;
    double headingSine;
    char unknown108[0x18];
    double longitude;
    double longitudeCosine;
    double longitudeSine;
    double altitude;
    Vector3 worldVelocity;
    char unknown158[0x30];
    Vector3 linearVelocity;
    Vector3 angularVelocity;
    Vector3 linearAcceleration;
    Vector3 angularAcceleration;
    char unknown1e8[4];
    double horizontalSpeed;
    double propulsionMomentBlendInput;
    double altitudeAboveReference;
    char unknown204[8];
    unsigned __int64 fixedLongitude;
    unsigned __int64 fixedAltitude;
    char unknown21c[0x54];
    IntegrationHistory* history;
    char unknown274[0x64];
    const PropulsionMomentControl* propulsionMomentControl;
    char unknown2dc[0x14];
    const SupplementalAerodynamics* supplementalAerodynamics;
    ForceContributors* forceContributors;
    MassProperties* massProperties;
    char unknown2fc[4];
    Vector3 force;
    Vector3 moment;
};

// Minimum airplane extension and the four-byte owner prefix before its configuration.
struct AirplaneConfiguration {
    char ownerPrefix[4];
    PropulsionConfiguration settings;
    char unknown144c[0x534];
    const AirplaneGeometry* geometry;
    char unknown1984[0x40];
    const ZeroAlphaMomentTuning* zeroAlphaMomentTuning;
};

class AirplaneDynamics : public FlightDynamics {
public:
    const AirplaneConfiguration* airframeConfiguration;
};

#pragma pack(pop)

typedef char CheckHistoryOffset[offsetof(FlightDynamics, history) == 0x270 ? 1 : -1];
typedef char CheckLinearAccelerationOffset[offsetof(FlightDynamics, linearAcceleration) == 0x1b8 ? 1 : -1];
typedef char CheckInertiaScaleOffset[offsetof(IntegrationHistory, inertiaScale) == 0x4f8 ? 1 : -1];
typedef char CheckForceOffset[offsetof(FlightDynamics, force) == 0x300 ? 1 : -1];
typedef char CheckWorldVelocityOffset[offsetof(FlightDynamics, worldVelocity) == 0x140 ? 1 : -1];
typedef char CheckFixedAltitudeOffset[offsetof(FlightDynamics, fixedAltitude) == 0x214 ? 1 : -1];
typedef char CheckPublisherSlot[offsetof(FlightPositionDispatch, publishesPosition) == 0x50 ? 1 : -1];
typedef char CheckHeightSlot[offsetof(GroundReferenceDispatch, height) == 0x08 ? 1 : -1];
typedef char CheckMassNotificationSlot[offsetof(FlightPositionDispatch, massPropertiesChanged) == 0xe4 ? 1 : -1];
typedef char CheckMassEntriesOffset[offsetof(MassProperties, entries) == 0x6c ? 1 : -1];
typedef char CheckForceContributorsOffset[offsetof(FlightDynamics, forceContributors) == 0x2f4 ? 1 : -1];

}

#endif
