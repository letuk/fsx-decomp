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
struct EngineForceRecord;
struct IntegrationHistory;
struct GroundReference;
struct ElectricalRuntime;
struct ElectricalOwner;
struct ForceContributors;
struct TurbineEngineState;
struct PropellerStepInputs;
struct PitchMomentModifier;
struct ZeroAlphaMomentTuning;
struct AirplaneControlSurfaces;
struct AirplaneGeometry;
class SupplementalAerodynamics;
class AirplaneDynamics;

// Reconstructed engine mount record in the configuration array at +0x199c.
struct ThrustOrientation {
    float elevation;
    float lateralAngle;
};

struct EngineMount {
    Vector3 position;
    ThrustOrientation orientation;
};

// Minimum environment layout for the air-density input.
struct FlightEnvironment {
    char unknown00[0x40];
    double ambientTemperatureCelsius;
    char unknown48[8];
    int icingEnvironmentActive;
    unsigned icingSeverity;
    char unknown58[0xdc];
    double airDensity;
};

// Minimum propulsion layouts shared by the thrust and aerodynamic calculations.
struct PropellerForceRecord {
    void UpdateFixedPitch(const FlightDynamics& airplane);
    int UpdateGovernorFeatherState(const IntegrationHistory& state, unsigned engineIndex,
                                   const PropellerStepInputs& inputs, double step);
    int UpdateFeatherState(const IntegrationHistory& state, unsigned engineIndex, double step);
    void ApplyGovernorPitch(const FlightDynamics& airplane, const EngineForceRecord& engine,
                            const PropellerStepInputs& inputs, double step, bool featherActive);
    double __fastcall CalculateAerodynamicTorque(const FlightDynamics& airplane, const PropellerStepInputs& inputs) const;
    void UpdateThrust(const IntegrationHistory& state, const PropellerStepInputs& inputs, double engineTorque);
    void __fastcall IntegrateSpeed(const IntegrationHistory& state, PropellerStepInputs& inputs,
                                    double engineTorque, double step);
    void __fastcall UpdateSynchronization(const IntegrationHistory& state, const EngineForceRecord& engine, double step);

    double signedRpm;
    double normalizedSpeed;
    float rotationPhase;
    double thrust;
    double bladePitch;
    int governorLowSpeedLatch;
    char unknown28[4];
    double synchronizationCorrection;
    int automaticFeatherArmed;
    int featherActive;
    double aerodynamicLoadFactor;
    double signedRotationFactor;
    const Vector3* mountOffset;
    float featherRequestTime;
};

struct EngineForceRecord {
    void UpdateTurbineSpeedOutput(TurbineEngineState* turbine) const;
    void UpdateInducedAirSpeed(const FlightDynamics& airplane, const PropellerForceRecord& propeller,
                               const PropellerStepInputs& step);

    const EngineMount* mount;
    char unknown04[8];
    double reverseControlInput;
    char unknown14[8];
    double governorCommand;
    char unknown24[0x10];
    double thrust;
    double inducedAirSpeed;
    double propellerTorque;
    double propellerDerivedRpm;
    char unknown54[8];
    double speedFraction;
    char unknown64[0x88];
};

// Reconstructed layout of AIR record 0x512 with axis headers in the first row and column.
struct PropellerCoefficientTable {
    double Evaluate(double advanceRatio, double bladePitchDegrees) const;

    unsigned rows;
    unsigned columns;
    double values[168];
};

// Optional fixed-pitch cruise setting allocated by the propeller parser.
struct FixedPitchCruiseSetting {
    double bladePitch;
    float activationSpeed;
};

struct PropulsionConfiguration {
    char unknown00[0xc8];
    double rollTorqueReferenceSpeed;
    char unknownd0[0x138];
    int propellerType;
    double propellerDiameter;
    double propellerMomentOfInertia;
    char unknown21c[4];
    double maximumBladePitch;
    double minimumBladePitch;
    double minimumGovernorRpm;
    double governorGain;
    double gearReductionRatio;
    char unknown248[0x10];
    double fixedBladePitch;
    double lowSpeedTheoryLimit;
    int propellerSyncAvailable;
    char unknown26c[0x10];
    double minimumRpmForFeather;
    double featherBladePitch;
    double featherPowerCoefficient;
    int defeatheringAccumulatorAvailable;
    int featheringSwitches;
    float reverseThrustFactor;
    double minimumGroundBladePitch;
    double minimumReverseBladePitch;
    char unknown2b0[0x1a4];
    unsigned engineCount;
    unsigned propellerCount;
    unsigned engineForPropeller[4];
    unsigned char engineType;
    char unknown46d[7];
    float reverseControlScale;
    char unknown478[0x100];
    AerodynamicCurve elevatorTrimPressureCurve;
    char unknown57c[0xe8];
    PropellerCoefficientTable thrustEfficiencyTable;
    PropellerCoefficientTable powerCoefficientTable;
    char unknown10f4[0x34];
    double cruiseLiftScalar;
    char unknown1130[0x18];
    double elevatorEffectiveness;
    char unknown1150[0x10];
    double elevatorTrimEffectiveness;
    double aileronTrimEffectiveness;
    double rudderTrimEffectiveness;
    double pitchStability;
    char unknown1180[0x20];
    float pFactorOnYaw;
    float torqueOnRoll;
    float gyroPrecessionOnYaw;
    float gyroPrecessionOnPitch;
    char unknown11b0[0x18];
    double propellerThrustScalar;
    char unknown11d0[0x274];
    int suppressFirstMoment;
    char unknown1448[0x534];
    const AirplaneGeometry* geometry;
    char unknown1980[4];
    const FixedPitchCruiseSetting* fixedPitchCruise;
};

struct PropulsionMomentControl {
    const FlightDynamics* owner;
    int simpleAerodynamics;
    float pFactorGain;
    float propellerTorqueGain;
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
    int (__thiscall* surfaceMode)(GroundReference*);
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

// Minimum layouts of the external fuel tank configuration and runtime records.
struct ExternalFuelTank {
    char unknown00[4];
    float lateralPosition;
    char unknown08[8];
    float capacity;
};

struct ExternalFuelTankState {
    const ExternalFuelTank* configuration;
    char unknown04[8];
    unsigned flags;
    char unknown10[0x10];
};

// Reconstructed minimum layout of the state reached through FlightDynamics+0x270.
struct IntegrationHistory {
    void __fastcall UpdateAngleOfAttackRate(const FlightDynamics& airplane);
    void UpdateAerodynamicLoadScales(const AirplaneGeometry& geometry, double airDensity);

    PropulsionConfiguration* propulsionConfiguration;
    char unknown04[0x10];
    double thrustToWeight;
    char unknown1c[0x40];
    double aerodynamicLift;
    double aerodynamicDrag;
    Vector3 propulsionForce;
    char unknown84[0x20];
    Vector3 propulsionMoment;
    Vector3 previousLinearAcceleration;
    Vector3 transformedAcceleration;
    Vector3 previousAngularAcceleration;
    Vector3 airRelativeVelocity;
    char unknown11c[0x18];
    Vector3 transformedAngularVelocity;
    double airRelativeSpeed;
    char unknown154[0x38];
    double attitudeAngle;
    double rawAerodynamicAngleDegrees;
    double aerodynamicAngleDegrees;
    double sideslipRadians;
    double rateCosine;
    double rateSine;
    char unknown1bc[0x10];
    double angleOfAttackRate;
    char unknown1d4[8];
    double dynamicPressure;
    double wingPressureForce;
    double rollMomentScale;
    double aerodynamicMomentScale;
    double slipstreamDynamicPressure;
    double slipstreamSpeedIncrement;
    char unknown20c[4];
    int propulsionMomentBlendMode;
    unsigned qualifiedGroundContactCount;
    char unknown218[0x38];
    const PitchMomentModifier* pitchMomentModifier;
    char unknown254[0x198];
    ExternalFuelTankState firstExternalTank;
    ExternalFuelTankState secondExternalTank;
    char unknown42c[8];
    TurbineEngineState* turbineEngines;
    PropellerForceRecord* propellerForces;
    EngineForceRecord* engineForces;
    char unknown440[0x38];
    double liftAdjustment;
    char unknown480[8];
    int propellerStopRequested;
    char unknown48c[0x6c];
    double inertiaScale;
    char unknown500[0x1c];
    int propellerSynchronizationEnabled;
    int automaticFeatherSwitch;
    char unknown524[0x10];
    const ElectricalRuntime* electrical;
    char unknown538[0x18];
    double propellerDiameterToFifth;
    char unknown558[0xb8];
    int propellerDeiceSwitch;
    char unknown614[8];
    double propellerRpmReference;
    char unknown624[8];
    double firstWingAttenuationInput;
    double secondWingAttenuationInput;
    char unknown63c[0xa8];
    double flapDeflectionFactor;
    char unknown6ec[0x5c];
    double lateralForceRateScale;
    double rollRateMomentScale;
    double liftRateScale;
    double pitchRateMomentScale;
    double slipstreamPitchRateMomentScale;
    double slipstreamYawRateMomentScale;
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
    int UsesGroundContactLimits() const;
    void UpdatePropellerBladePitch(unsigned propellerIndex, unsigned engineIndex, PropellerStepInputs& inputs,
                                   double step) const;
    void UpdateAutomaticFeatherArming() const;
    void UpdateMappedPropellers(double step) const;
    void UpdateMappedPropellersAfterPreparation(double step) const;
    void UpdatePropellerLoadFactor(double step) const;
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
    const FlightEnvironment* environment;
    char unknown1c[8];
    HeadingTransform* heading;
    char unknown28[0x10];
    float realismWeight;
    char unknown3c[0x28];
    ElectricalOwner* electricalOwner;
    char unknown68[0x4c];
    int electricalOwnerBypass;
    char unknownb8[4];
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
    char unknown198c[0x38];
    const ZeroAlphaMomentTuning* zeroAlphaMomentTuning;
    char unknown19c8[0x34];
    const class AirplaneHighAngleModel* highAngleModel;
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
