#include "flight_dynamics.h"
#include <math.h>

namespace sim1 {

typedef char CheckAirplaneConfiguration[offsetof(AirplaneDynamics, airframeConfiguration) == 0x330 ? 1 : -1];
typedef char CheckPropellerInertia[offsetof(AirplaneConfiguration, settings.propellerMomentOfInertia) == 0x218 ? 1 : -1];
typedef char CheckPitchGyroFactor[offsetof(AirplaneConfiguration, settings.gyroPrecessionOnPitch) == 0x11b0 ? 1 : -1];
typedef char CheckPropellerCount[offsetof(PropulsionConfiguration, propellerCount) == 0x458 ? 1 : -1];
typedef char CheckPropellerRpm[offsetof(PropellerForceRecord, signedRpm) == 0 ? 1 : -1];
typedef char CheckPropellerRecordSize[sizeof(PropellerForceRecord) == 0x54 ? 1 : -1];
typedef char CheckPropellerGyroGain[offsetof(PropulsionMomentControl, propellerGyroGain) == 0x10 ? 1 : -1];
typedef char CheckPropellerGyroGate[offsetof(PropulsionMomentControl, suppressPropellerGyro) == 0x20 ? 1 : -1];
typedef char CheckFirstMomentReference[offsetof(FlightPositionDispatch, firstMomentReferenceZ) == 0x11c ? 1 : -1];
typedef char CheckSecondMomentReference[offsetof(FlightPositionDispatch, secondMomentReferenceZ) == 0x120 ? 1 : -1];

// Reconstructed layout of the AIR record read by the airplane moment model.
#pragma pack(push, 4)
struct AerodynamicSample {
    double angle;
    double coefficient;
};

template <unsigned Count>
struct AerodynamicCurveStorage {
    AerodynamicCurve curve;
    AerodynamicSample samples[Count];
};

struct AirplaneControlSurfaces {
    char unknown00[0xc];
    const PitchMomentModifier* pitchControlModifier;
    char unknown10[0xbc];
    double elevator;
    double firstAileron;
    double secondAileron;
    double rudder;
    char unknownec[0x10];
    double elevatorTrim;
    float aileronTrim;
    float rudderTrim;
    double firstSpoilerAngle;
    double secondSpoilerAngle;
};

struct AirplaneGeometry {
    double wingArea;
    double wingSpan;
    double referenceChord;
    char unknown18[0xb4];
    double horizontalTailIncidence;
};

struct AirplaneAerodynamicTables {
    char unknown00[0x54];
    double elevatorMoment;
    double elevatorTrimMoment;
    double pitchRateMoment;
    double zeroAlphaMoment;
    double angleOfAttackRateMoment;
    double horizontalTailIncidenceMoment;
    double pitchCoefficientSlot70;
    double flapPitchMoment;
    double spoilerPitchMoment;
    double slipstreamElevatorMoment;
    double slipstreamPitchRateMoment;
    double elevatorTrimCoupling;
    char unknownb4[0x28];
    double spoilerRoll;
    float aileronTrimRoll;
    char unknowne8[0x28];
    double slipstreamRudderYaw;
    double slipstreamRollRateYaw;
    float rudderTrimYaw;
    char unknown124[0x40c];
    short machElevatorMoment[17];
    short machAngleOfAttackRateMoment[17];
    short machPitchRateMoment[17];
    short machHorizontalTailIncidenceMoment[17];
    short machPitchMoment[17];
    char unknown5da[0x536];
    AerodynamicCurveStorage<5> elevatorAlphaCurve;
    AerodynamicCurveStorage<5> horizontalTailIncidenceAlphaCurve;
    AerodynamicCurveStorage<5> angleOfAttackRateAlphaCurve;
    AerodynamicCurveStorage<5> pitchRateAlphaCurve;
    char unknownc60[0x54];
    AerodynamicCurveStorage<7> elevatorDeflectionCurve;
    char unknownd28[0x3dc];
    AerodynamicCurveStorage<47> angleMoment;
};

struct AirplaneCoefficientModel;

// Minimum ABI of the original coefficient table at 0x20bc2618.
// The unreconstructed members retain their original virtual calls.
struct AirplaneCoefficientDispatch {
    const void* slots00To80[33];
    double (__thiscall* sideslipRoll)(const AirplaneCoefficientModel*);
    double (__thiscall* rudderRoll)(const AirplaneCoefficientModel*);
    double (__thiscall* aileronRoll)(const AirplaneCoefficientModel*, int surface);
    double (__thiscall* rollRate)(const AirplaneCoefficientModel*);
    double (__thiscall* yawRateRoll)(const AirplaneCoefficientModel*);
    double (__thiscall* spoilerRoll)(const AirplaneCoefficientModel*);
    double (__thiscall* aileronTrimRoll)(const AirplaneCoefficientModel*);
    double (__thiscall* additionalRoll)(const AirplaneCoefficientModel*);
    double (__thiscall* rollRateYaw)(const AirplaneCoefficientModel*);
    double (__thiscall* yawRate)(const AirplaneCoefficientModel*);
    double (__thiscall* rudderYaw)(const AirplaneCoefficientModel*);
    double (__thiscall* sideslipYaw)(const AirplaneCoefficientModel*);
    double (__thiscall* aileronYaw)(const AirplaneCoefficientModel*);
    double (__thiscall* slipstreamRudderYaw)(const AirplaneCoefficientModel*);
    double (__thiscall* slipstreamRollRateYaw)(const AirplaneCoefficientModel*);
    double (__thiscall* rudderTrimYaw)(const AirplaneCoefficientModel*);
    double (__thiscall* additionalYaw)(const AirplaneCoefficientModel*);
};

struct AirplaneCoefficientModel {
    double CalculateBaselineMoment() const;
    double CalculateElevatorCoefficient() const;
    double CalculatePitchRateCoefficient() const;
    double CalculateAngleOfAttackRateCoefficient() const;
    double CalculateHorizontalTailIncidenceCoefficient() const;
    double CalculateElevatorTrimCoefficient() const;
    double GetPitchCoefficientSlot70() const;
    double GetFlapPitchCoefficient() const;
    double GetSpoilerPitchCoefficient() const;
    double GetSlipstreamElevatorCoefficient() const;
    double GetSlipstreamPitchRateCoefficient() const;
    double GetSpoilerRollCoefficient() const;
    double GetAileronTrimRollCoefficient() const;
    double GetSlipstreamRudderYawCoefficient() const;
    double GetSlipstreamRollRateYawCoefficient() const;
    double GetRudderTrimYawCoefficient() const;

    const AirplaneCoefficientDispatch* dispatch;
    unsigned machIndex;
    double machFraction;
    AirplaneDynamics* airplane;
    const IntegrationHistory* state;
    const AirplaneConfiguration* configuration;
    const AirplaneAerodynamicTables* tables;
};

// Minimum ABI shared by the runtime and control-surface modifier callbacks.
struct PitchMomentModifierDispatch {
    void* unreviewedSlots[6];
    double (__stdcall* factor)(const PitchMomentModifier*);
};

struct PitchMomentModifier {
    const PitchMomentModifierDispatch* dispatch;
};

struct ZeroAlphaMomentTuning {
    double scale;
    double bias;
};

// Minimum virtual interface verified at the two supplemental aerodynamic tables.
class SupplementalAerodynamics {
public:
    virtual double PitchMoment() const = 0;
    virtual double RollMoment() const = 0;
};

// Minimum ABI for the configuration table at 0x20bc213c.
struct AirplaneHighAngleDispatch {
    const void* slots00To0c[4];
    float (__thiscall* referenceAngle)(const AirplaneHighAngleModel*);
};

class AirplaneHighAngleModel {
public:
    const AirplaneHighAngleDispatch* dispatch;
};
#pragma pack(pop)

typedef char CheckAerodynamicSample[offsetof(AerodynamicCurveStorage<47>, samples) == 4 && sizeof(AerodynamicSample) == 16 ? 1 : -1];
typedef char CheckAngleCurveExtent[sizeof(AerodynamicCurveStorage<47>) == 0x2f4 ? 1 : -1];
typedef char CheckAngleCurveOffset[offsetof(AirplaneAerodynamicTables, angleMoment) == 0x1104 ? 1 : -1];
typedef char CheckCoefficientTables[offsetof(AirplaneCoefficientModel, tables) == 0x1c ? 1 : -1];
typedef char CheckAerodynamicAngle[offsetof(IntegrationHistory, aerodynamicAngleDegrees) == 0x19c ? 1 : -1];
typedef char CheckAerodynamicMomentScale[offsetof(IntegrationHistory, aerodynamicMomentScale) == 0x1f4 ? 1 : -1];
typedef char CheckZeroAlphaMoment[offsetof(AirplaneAerodynamicTables, zeroAlphaMoment) == 0x6c ? 1 : -1];
typedef char CheckMachPitchMoment[offsetof(AirplaneAerodynamicTables, machPitchMoment) == 0x5b8 ? 1 : -1];
typedef char CheckMachState[offsetof(AirplaneCoefficientModel, machFraction) == 8 ? 1 : -1];
typedef char CheckModifierSlot[offsetof(PitchMomentModifierDispatch, factor) == 0x18 ? 1 : -1];
typedef char CheckModifierOffset[offsetof(IntegrationHistory, pitchMomentModifier) == 0x250 ? 1 : -1];
typedef char CheckZeroAlphaTuning[offsetof(AirplaneConfiguration, zeroAlphaMomentTuning) == 0x19c4 ? 1 : -1];
typedef char CheckRealismWeight[offsetof(FlightDynamics, realismWeight) == 0x38 ? 1 : -1];

// Both reference corrections are published by the same airplane model update.
extern double highAlphaReferenceXShift;
extern double highAlphaReferenceZShift;
// Published by the slipstream update at sim1.dll 0x20c00840.
extern double slipstreamPitchCoefficient;
extern double slipstreamYawCoefficient;
// Roll inputs published before the moment callback at sim1.dll 0x20c45510.
extern double slipstreamRollCoefficient;
extern double wingLiftMultiplierSum;
extern double secondOuterWingLift;
extern double secondInnerWingLift;
extern double firstInnerWingLift;
extern double firstOuterWingLift;

class AirplaneMomentModel {
public:
    double CalculatePropellerGyroscopicMoment(const AirplaneDynamics& airplane) const;
    double CalculateForceMomentX(const AirplaneDynamics& airplane, const Vector3& force) const;
    double CalculateForceMomentY(const AirplaneDynamics& airplane, const Vector3& force) const;
    double CalculateForceMomentZ(const ModelPoint& center, const Vector3& force) const;
    double CalculateAngleCurveMoment(const AirplaneDynamics& airplane) const;
    double CalculateScaledBaselineMoment(const AirplaneDynamics& airplane) const;
    double CalculateElevatorMoment(const AirplaneDynamics& airplane) const;
    double CalculatePitchRateMoment(const AirplaneDynamics& airplane) const;
    double CalculateAngleOfAttackRateMoment(const AirplaneDynamics& airplane) const;
    double CalculateHorizontalTailIncidenceMoment(const AirplaneDynamics& airplane) const;
    double CalculateElevatorTrimMoment(const AirplaneDynamics& airplane) const;
    double CalculatePitchMoment(const AirplaneDynamics& airplane, const Vector3& force) const;
    double CalculateSlipstreamTailPitchMoment(const AirplaneGeometry& geometry, const IntegrationHistory& state) const;
    double CalculateSimplePitchMoment(double pitch, double elevatorMoment, double pitchRateMoment) const;
    double CalculateRollMoment(AirplaneDynamics& airplane, const Vector3& force) const;
    double CalculatePropellerRollTorque(const AirplaneDynamics& airplane) const;
    double CalculateSimpleRollMoment(double firstAileron, double secondAileron,
        double spoilers, double yawRate) const;
    double CalculateYawMoment(AirplaneDynamics& airplane, const Vector3& force) const;
    double CalculatePropellerYawGyro(const AirplaneDynamics& airplane) const;
    double CalculatePropellerPFactorYaw(const AirplaneDynamics& airplane) const;
    double CalculateExternalTankYaw(AirplaneDynamics& airplane) const;
    double CalculateSimpleYawMoment(double rudder, double yawRate, double rollRate, double sideslip) const;

    char unknown00[4];
    const AirplaneDynamics* controlOwner;
    char unknown08[4];
    const AirplaneCoefficientModel* coefficients;
};

// Original: sim1.dll 0x20c013a0, propeller moment block 0x20c0155d–0x20c016ac.
// Extracted reconstruction scope. The original calculates this inside the airplane model.
double AirplaneMomentModel::CalculatePropellerGyroscopicMoment(const AirplaneDynamics& airplane) const
{
    const IntegrationHistory& state = *airplane.history;
    const unsigned count = state.propulsionConfiguration->propellerCount;
    const PropulsionMomentControl* control = airplane.propulsionMomentControl;
    double moment = 0.0;
    if (count != 0 && control->suppressPropellerGyro == 0 && control->propellerGyroGain > 0.0f) {
        const AirplaneConfiguration& configuration = *airplane.airframeConfiguration;
        const PropellerForceRecord* propellers = state.propellerForces;
        const double rpmToRadiansPerSecond = 0.1047197551;
        for (unsigned index = 0; index < count; ++index) {
            moment += propellers[index].signedRpm * rpmToRadiansPerSecond *
                configuration.settings.propellerMomentOfInertia * airplane.angularVelocity.y *
                configuration.settings.gyroPrecessionOnPitch * control->propellerGyroGain;
        }
    }
    // The original stores the term as a double before adding it to the other moments.
    volatile double roundedMoment = moment;
    return roundedMoment;
}

// Original: sim1.dll 0x20c013a0, force moment at 0x20c01733–0x20c01774.
// Extracted contribution for the full moment sum.
double AirplaneMomentModel::CalculateForceMomentX(const AirplaneDynamics& airplane, const Vector3& force) const
{
    const ModelPoint& center = airplane.massProperties->centerOfGravity;
    const double centerZ = center.z;
    const double referenceZ = airplane.positionDispatch->firstMomentReferenceZ(&airplane);
    const double verticalArm = centerZ - referenceZ - highAlphaReferenceZShift;
    return verticalArm * force.y - center.y * force.z;
}

// Original: sim1.dll 0x20c020a0, force moment at 0x20c02645–0x20c0267f.
// Extracted contribution for the full moment sum.
double AirplaneMomentModel::CalculateForceMomentY(const AirplaneDynamics& airplane, const Vector3& force) const
{
    const ModelPoint& center = airplane.massProperties->centerOfGravity;
    // Preserve the original double store before the reference query.
    volatile double longitudinalTerm = (center.x - highAlphaReferenceXShift) * force.z;
    const double centerZ = center.z;
    const double referenceZ = airplane.positionDispatch->secondMomentReferenceZ(&airplane);
    const double verticalArm = centerZ - referenceZ - highAlphaReferenceZShift;
    volatile double moment = longitudinalTerm - verticalArm * force.x;
    return moment;
}

// Original: sim1.dll 0x20c01890, force moment at 0x20c02025–0x20c0203c.
// Extracted contribution for the full moment sum.
double AirplaneMomentModel::CalculateForceMomentZ(const ModelPoint& center, const Vector3& force) const
{
    return center.y * force.x - (center.x - highAlphaReferenceXShift) * force.y;
}

// Original: sim1.dll 0x20bcbf60. The original lookup receives the curve in ESI.
__declspec(noinline) double __fastcall SampleAerodynamicCurve(const AerodynamicCurve* curve, double angle)
{
    if (curve == 0 || curve->count == 0) {
        return 0.0;
    }
    // Every loader stores the samples immediately after the common count.
    const AerodynamicSample* samples =
        reinterpret_cast<const AerodynamicSample*>(reinterpret_cast<const char*>(curve) + sizeof(*curve));
    if (angle <= samples[0].angle) {
        return samples[0].coefficient;
    }
    unsigned last = curve->count - 1;
    if (angle > samples[last].angle) {
        return samples[last].coefficient;
    }

    unsigned first = 0;
    for (;;) {
        const unsigned middle = (first + last) / 2;
        if (middle == first || middle == last) {
            const AerodynamicSample& lower = samples[middle];
            const AerodynamicSample& upper = samples[middle + 1];
            const double lowerAngle = lower.angle;
            const double lowerCoefficient = lower.coefficient;
            const double fraction = (angle - lowerAngle) / (upper.angle - lowerAngle);
            return (upper.coefficient - lowerCoefficient) * fraction + lowerCoefficient;
        }
        if (angle > samples[middle].angle) {
            first = middle;
        } else if (angle < samples[middle].angle) {
            last = middle;
        } else {
            return samples[middle].coefficient;
        }
    }
}

// Reconstruction-only helper for the angle term inside sim1.dll 0x20c013a0.
static inline double ScaleAngleCurveMoment(const AirplaneCoefficientModel& coefficients,
    const IntegrationHistory& state, double angleDegrees)
{
    const double degreesToRadians = 0.017453292519943295;
    const double angle = angleDegrees * degreesToRadians;
    // The original stores the scaled curve contribution before adding it to the sum.
    volatile double moment = SampleAerodynamicCurve(&coefficients.tables->angleMoment.curve, angle) *
        state.aerodynamicMomentScale;
    return moment;
}

// Original: sim1.dll 0x20c013a0, scaled angle term through 0x20c01437.
// Extracted calculation of the angle-dependent contribution from slot +0x54.
double AirplaneMomentModel::CalculateAngleCurveMoment(const AirplaneDynamics& airplane) const
{
    if (coefficients == 0) {
        return 0.0;
    }
    const IntegrationHistory& state = *airplane.history;
    return ScaleAngleCurveMoment(*coefficients, state, state.aerodynamicAngleDegrees);
}

// Original: sim1.dll 0x20c4ff30, concrete coefficient-model slot +0x58.
__declspec(noinline) double AirplaneCoefficientModel::CalculateBaselineMoment() const
{
    if (tables == 0) {
        return 0.0;
    }
    double coefficient = tables->zeroAlphaMoment;
    if (airplane->positionDispatch->publishesPosition(airplane) != 0) {
        const int lower = tables->machPitchMoment[machIndex];
        const int upper = tables->machPitchMoment[machIndex + 1];
        const double sampleScale = 0.0004885197851;
        // The Mach-adjusted coefficient is stored as a double before the modifier query.
        volatile double machAdjusted = coefficient + ((upper - lower) * machFraction + lower) * sampleScale;
        const PitchMomentModifier* modifier = state->pitchMomentModifier;
        const double loss = modifier ? 1.0 - modifier->dispatch->factor(modifier) : 0.0;
        coefficient = (1.0 - loss * 0.3) * machAdjusted;
    }
    const double realism = airplane->propulsionMomentControl->owner->realismWeight;
    const ZeroAlphaMomentTuning& tuning = *configuration->zeroAlphaMomentTuning;
    return ((1.0 - tuning.scale) * realism + tuning.scale) * coefficient + (1.0 - realism) * tuning.bias;
}

// Original: sim1.dll 0x20c013a0, baseline moment contribution at 0x20c013e6–0x20c0140a.
// Extracted calculation for the concrete coefficient model.
double AirplaneMomentModel::CalculateScaledBaselineMoment(const AirplaneDynamics& airplane) const
{
    if (coefficients == 0) {
        return 0.0;
    }
    const IntegrationHistory& state = *airplane.history;
    volatile double moment = coefficients->CalculateBaselineMoment() * state.aerodynamicMomentScale;
    return moment;
}

typedef char CheckElevator[offsetof(AirplaneControlSurfaces, elevator) == 0xcc ? 1 : -1];
typedef char CheckElevatorTrim[offsetof(AirplaneControlSurfaces, elevatorTrim) == 0xfc ? 1 : -1];
typedef char CheckElevatorMoment[offsetof(AirplaneAerodynamicTables, elevatorMoment) == 0x54 ? 1 : -1];
typedef char CheckElevatorTrimCoupling[offsetof(AirplaneAerodynamicTables, elevatorTrimCoupling) == 0xac ? 1 : -1];
typedef char CheckMachElevatorMoment[offsetof(AirplaneAerodynamicTables, machElevatorMoment) == 0x530 ? 1 : -1];
typedef char CheckElevatorAlphaCurve[offsetof(AirplaneAerodynamicTables, elevatorAlphaCurve) == 0xb10 ? 1 : -1];
typedef char CheckElevatorDeflectionCurve[offsetof(AirplaneAerodynamicTables, elevatorDeflectionCurve) == 0xcb4 ? 1 : -1];
typedef char CheckElevatorEffectiveness[offsetof(AirplaneConfiguration, settings.elevatorEffectiveness) == 0x114c ? 1 : -1];
typedef char CheckControlSurfaceSlot[offsetof(FlightPositionDispatch, controlSurfaces) == 0x118 ? 1 : -1];
typedef char CheckRawAlpha[offsetof(IntegrationHistory, rawAerodynamicAngleDegrees) == 0x194 ? 1 : -1];

// Original: sim1.dll 0x20c4fce0, concrete coefficient-model slot +0x5c.
__declspec(noinline) double AirplaneCoefficientModel::CalculateElevatorCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    double coefficient = tables->elevatorMoment;
    if (airplane->positionDispatch->publishesPosition(airplane) != 0) {
        const int lower = tables->machElevatorMoment[machIndex];
        const int upper = tables->machElevatorMoment[machIndex + 1];
        coefficient += ((upper - lower) * machFraction + lower) * 0.0004885197851;
    }
    const double trim = airplane->positionDispatch->controlSurfaces(airplane)->elevatorTrim;
    // Preserve the original rounding before each curve contributes to the product.
    volatile double trimmed = coefficient - fabs(trim) * tables->elevatorTrimCoupling;
    const double deflection = airplane->positionDispatch->controlSurfaces(airplane)->elevator;
    const AirplaneAerodynamicTables& data = *tables;
    volatile double deflectionAdjusted = SampleAerodynamicCurve(&data.elevatorDeflectionCurve.curve, deflection) * trimmed;
    const double alphaAdjusted = SampleAerodynamicCurve(&data.elevatorAlphaCurve.curve, state->rawAerodynamicAngleDegrees) *
        deflectionAdjusted;
    const double realism = airplane->propulsionMomentControl->owner->realismWeight;
    return (realism * 0.09999999999999998 + 0.9) * alphaAdjusted * configuration->settings.elevatorEffectiveness;
}

// Original: sim1.dll 0x20c013a0, elevator contribution at 0x20c01438–0x20c01461.
// Extracted calculation for the concrete coefficient model.
double AirplaneMomentModel::CalculateElevatorMoment(const AirplaneDynamics& airplane) const
{
    if (coefficients == 0) {
        return 0.0;
    }
    const IntegrationHistory& state = *airplane.history;
    const double deflection = controlOwner->positionDispatch->controlSurfaces(controlOwner)->elevator;
    volatile double moment = coefficients->CalculateElevatorCoefficient() * deflection * state.aerodynamicMomentScale;
    return moment;
}

typedef char CheckPitchRateMoment[offsetof(AirplaneAerodynamicTables, pitchRateMoment) == 0x64 ? 1 : -1];
typedef char CheckMachPitchRateMoment[offsetof(AirplaneAerodynamicTables, machPitchRateMoment) == 0x574 ? 1 : -1];
typedef char CheckPitchRateAlphaCurve[offsetof(AirplaneAerodynamicTables, pitchRateAlphaCurve) == 0xc0c ? 1 : -1];
typedef char CheckPitchStability[offsetof(AirplaneConfiguration, settings.pitchStability) == 0x117c ? 1 : -1];
typedef char CheckPitchControlModifier[offsetof(AirplaneControlSurfaces, pitchControlModifier) == 0xc ? 1 : -1];
typedef char CheckPitchRate[offsetof(IntegrationHistory, transformedAngularVelocity.x) == 0x134 ? 1 : -1];
typedef char CheckPitchRateScale[offsetof(IntegrationHistory, pitchRateMomentScale) == 0x760 ? 1 : -1];

// Original: sim1.dll 0x20c4fe00, concrete coefficient-model slot +0x60.
__declspec(noinline) double AirplaneCoefficientModel::CalculatePitchRateCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    const AirplaneAerodynamicTables& data = *tables;
    const short* samples = data.machPitchRateMoment + machIndex;
    const int lower = samples[0];
    const double alphaFactor = SampleAerodynamicCurve(&data.pitchRateAlphaCurve.curve, state->rawAerodynamicAngleDegrees);
    const int upper = samples[1];
    const double coefficient = ((upper - lower) * machFraction + lower) * 0.0004885197851 + data.pitchRateMoment;
    const double realism = airplane->propulsionMomentControl->owner->realismWeight;
    const double modifierWeight = 0.15;
    // The unmodified coefficient is stored as a double before the control-surface query.
    volatile double scaled = alphaFactor * coefficient * (1.0 + (1.0 - realism) * modifierWeight) *
        configuration->settings.pitchStability;

    const PitchMomentModifier* controlModifier = airplane->positionDispatch->controlSurfaces(airplane)->pitchControlModifier;
    const float controlLoss = controlModifier ? static_cast<float>(1.0 - controlModifier->dispatch->factor(controlModifier)) : 1.0f;
    const PitchMomentModifier* runtimeModifier = state->pitchMomentModifier;
    volatile double adjusted = (1.0 - controlLoss * modifierWeight) * scaled;
    const double runtimeLoss = runtimeModifier ? 1.0 - runtimeModifier->dispatch->factor(runtimeModifier) : 0.0;
    return (1.0 - runtimeLoss) * adjusted;
}

// Original: sim1.dll 0x20c013a0, pitch-rate term at 0x20c01462–0x20c0147a.
// Extracted calculation for the concrete coefficient model.
double AirplaneMomentModel::CalculatePitchRateMoment(const AirplaneDynamics& airplane) const
{
    if (coefficients == 0) {
        return 0.0;
    }
    const IntegrationHistory& state = *airplane.history;
    volatile double moment = coefficients->CalculatePitchRateCoefficient() * state.transformedAngularVelocity.x *
        state.pitchRateMomentScale;
    return moment;
}

typedef char CheckAngleOfAttackRateBase[offsetof(AirplaneAerodynamicTables, angleOfAttackRateMoment) == 0x74 ? 1 : -1];
typedef char CheckAngleOfAttackRateMach[offsetof(AirplaneAerodynamicTables, machAngleOfAttackRateMoment) == 0x552 ? 1 : -1];
typedef char CheckAngleOfAttackRateCurve[offsetof(AirplaneAerodynamicTables, angleOfAttackRateAlphaCurve) == 0xbb8 ? 1 : -1];
typedef char CheckAngleOfAttackRate[offsetof(IntegrationHistory, angleOfAttackRate) == 0x1cc ? 1 : -1];
typedef char CheckAirRelativeVelocity[offsetof(IntegrationHistory, airRelativeVelocity) == 0x104 ? 1 : -1];

// Original: sim1.dll 0x20c50020, concrete coefficient-model slot +0x68.
__declspec(noinline) double AirplaneCoefficientModel::CalculateAngleOfAttackRateCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    double coefficient = tables->angleOfAttackRateMoment;
    if (airplane->positionDispatch->publishesPosition(airplane) != 0) {
        const short* samples = tables->machAngleOfAttackRateMoment + machIndex;
        const int lower = samples[0];
        const int upper = samples[1];
        coefficient += ((upper - lower) * machFraction + lower) * 0.0004885197851;
    }
    return SampleAerodynamicCurve(&tables->angleOfAttackRateAlphaCurve.curve, state->rawAerodynamicAngleDegrees) *
        coefficient;
}

// Original: sim1.dll 0x20bfb840, angle-of-attack rate calculation through 0x20bfb8db.
// Extracted calculation for the first output of the original two-rate update.
void __fastcall IntegrationHistory::UpdateAngleOfAttackRate(const FlightDynamics& airplane)
{
    const double speedSquared = airRelativeVelocity.x * airRelativeVelocity.x +
        airRelativeVelocity.z * airRelativeVelocity.z;
    if (speedSquared > 0.0) {
        const double rate = (airplane.linearAcceleration.z * airRelativeVelocity.z -
            airplane.linearAcceleration.y * airRelativeVelocity.x) / speedSquared;
        angleOfAttackRate = rate;
        const double rateLimit = 0.4363323129985824;
        if (fabs(rate) > rateLimit) {
            angleOfAttackRate = rate < 0.0 ? -rateLimit : rateLimit;
        }
    } else {
        angleOfAttackRate = 0.0;
    }
}

// Original: sim1.dll 0x20c013a0, angle-of-attack rate contribution at 0x20c0147b–0x20c01493.
// Extracted calculation for the concrete coefficient model.
double AirplaneMomentModel::CalculateAngleOfAttackRateMoment(const AirplaneDynamics& airplane) const
{
    if (coefficients == 0) {
        return 0.0;
    }
    const IntegrationHistory& state = *airplane.history;
    volatile double moment = coefficients->CalculateAngleOfAttackRateCoefficient() * state.angleOfAttackRate *
        state.pitchRateMomentScale;
    return moment;
}

typedef char CheckAirplaneGeometry[offsetof(AirplaneConfiguration, geometry) == 0x1980 ? 1 : -1];
typedef char CheckTailIncidence[offsetof(AirplaneGeometry, horizontalTailIncidence) == 0xcc ? 1 : -1];
typedef char CheckTailIncidenceBase[offsetof(AirplaneAerodynamicTables, horizontalTailIncidenceMoment) == 0x7c ? 1 : -1];
typedef char CheckTailIncidenceMach[offsetof(AirplaneAerodynamicTables, machHorizontalTailIncidenceMoment) == 0x596 ? 1 : -1];
typedef char CheckTailIncidenceCurve[offsetof(AirplaneAerodynamicTables, horizontalTailIncidenceAlphaCurve) == 0xb64 ? 1 : -1];

// Original: sim1.dll 0x20c500c0, concrete coefficient-model slot +0x64.
__declspec(noinline) double AirplaneCoefficientModel::CalculateHorizontalTailIncidenceCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    double coefficient = tables->horizontalTailIncidenceMoment;
    if (airplane->positionDispatch->publishesPosition(airplane) != 0) {
        const short* samples = tables->machHorizontalTailIncidenceMoment + machIndex;
        const int lower = samples[0];
        const int upper = samples[1];
        coefficient += ((upper - lower) * machFraction + lower) * 0.0004885197851;
    }
    // The alpha-adjusted coefficient is rounded before querying the control surfaces.
    const double alphaAdjusted = SampleAerodynamicCurve(&tables->horizontalTailIncidenceAlphaCurve.curve,
        state->rawAerodynamicAngleDegrees) * coefficient;
    const PitchMomentModifier* modifier = airplane->positionDispatch->controlSurfaces(airplane)->pitchControlModifier;
    const float loss = modifier ? static_cast<float>(1.0 - modifier->dispatch->factor(modifier)) : 1.0f;
    return (1.0 - loss * 0.15) * alphaAdjusted;
}

// Original: sim1.dll 0x20c013a0, horizontal-tail incidence contribution at 0x20c01494–0x20c014bb.
// Extracted calculation for the concrete coefficient model.
double AirplaneMomentModel::CalculateHorizontalTailIncidenceMoment(const AirplaneDynamics& airplane) const
{
    if (coefficients == 0) {
        return 0.0;
    }
    const AirplaneGeometry& geometry = *airplane.airframeConfiguration->geometry;
    const IntegrationHistory& state = *airplane.history;
    volatile double moment = coefficients->CalculateHorizontalTailIncidenceCoefficient() * geometry.horizontalTailIncidence *
        state.aerodynamicMomentScale;
    return moment;
}

typedef char CheckElevatorTrimBase[offsetof(AirplaneAerodynamicTables, elevatorTrimMoment) == 0x5c ? 1 : -1];
typedef char CheckElevatorTrimEffectiveness[offsetof(AirplaneConfiguration, settings.elevatorTrimEffectiveness) == 0x1164 ? 1 : -1];
typedef char CheckElevatorTrimPressureCurve[offsetof(AirplaneConfiguration, settings.elevatorTrimPressureCurve) == 0x57c ? 1 : -1];
typedef char CheckDynamicPressure[offsetof(IntegrationHistory, dynamicPressure) == 0x1dc ? 1 : -1];

// Original: sim1.dll 0x20c501a0, concrete coefficient-model slot +0x6c.
__declspec(noinline) double AirplaneCoefficientModel::CalculateElevatorTrimCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    const IntegrationHistory& runtime = *state;
    const double alphaFactor = SampleAerodynamicCurve(&tables->elevatorAlphaCurve.curve, runtime.rawAerodynamicAngleDegrees);
    const double alphaAdjusted = configuration->settings.elevatorTrimEffectiveness * tables->elevatorTrimMoment * alphaFactor;
    const double pressureAdjusted = SampleAerodynamicCurve(&configuration->settings.elevatorTrimPressureCurve,
        runtime.dynamicPressure) * alphaAdjusted;
    const PitchMomentModifier* modifier = airplane->positionDispatch->controlSurfaces(airplane)->pitchControlModifier;
    const float loss = modifier ? static_cast<float>(1.0 - modifier->dispatch->factor(modifier)) : 1.0f;
    return (1.0 - loss) * pressureAdjusted;
}

// Original: sim1.dll 0x20c013a0, elevator trim contribution at 0x20c014bc–0x20c014e5.
// Extracted calculation for the concrete coefficient model.
double AirplaneMomentModel::CalculateElevatorTrimMoment(const AirplaneDynamics& airplane) const
{
    if (coefficients == 0) {
        return 0.0;
    }
    const IntegrationHistory& state = *airplane.history;
    const double trim = controlOwner->positionDispatch->controlSurfaces(controlOwner)->elevatorTrim;
    volatile double moment = coefficients->CalculateElevatorTrimCoefficient() * trim * state.aerodynamicMomentScale;
    return moment;
}

typedef char CheckPitchCoefficientSlot70[offsetof(AirplaneAerodynamicTables, pitchCoefficientSlot70) == 0x84 ? 1 : -1];
typedef char CheckFlapPitchCoefficient[offsetof(AirplaneAerodynamicTables, flapPitchMoment) == 0x8c ? 1 : -1];
typedef char CheckSpoilerPitchCoefficient[offsetof(AirplaneAerodynamicTables, spoilerPitchMoment) == 0x94 ? 1 : -1];
typedef char CheckSlipstreamElevatorCoefficient[offsetof(AirplaneAerodynamicTables, slipstreamElevatorMoment) == 0x9c ? 1 : -1];
typedef char CheckSlipstreamPitchRateCoefficient[offsetof(AirplaneAerodynamicTables, slipstreamPitchRateMoment) == 0xa4 ? 1 : -1];
typedef char CheckSpoilerAngles[offsetof(AirplaneControlSurfaces, firstSpoilerAngle) == 0x10c &&
    offsetof(AirplaneControlSurfaces, secondSpoilerAngle) == 0x114 ? 1 : -1];
typedef char CheckFlapDeflection[offsetof(IntegrationHistory, flapDeflectionFactor) == 0x6e4 ? 1 : -1];
typedef char CheckSlipstreamPressure[offsetof(IntegrationHistory, slipstreamDynamicPressure) == 0x1fc ? 1 : -1];
typedef char CheckSlipstreamRateScale[offsetof(IntegrationHistory, slipstreamPitchRateMomentScale) == 0x768 ? 1 : -1];
typedef char CheckSupplementalAerodynamics[offsetof(FlightDynamics, supplementalAerodynamics) == 0x2f0 ? 1 : -1];
typedef char CheckSimpleAerodynamics[offsetof(PropulsionMomentControl, simpleAerodynamics) == 4 ? 1 : -1];
typedef char CheckPitchRadians[offsetof(FlightDynamics, pitchRadians) == 0xc0 ? 1 : -1];

// Original: sim1.dll 0x20c50270, concrete coefficient-model slot +0x70.
__declspec(noinline) double AirplaneCoefficientModel::GetPitchCoefficientSlot70() const
{
    if (tables == 0) {
        return 0.0;
    }
    return tables->pitchCoefficientSlot70;
}

// Original: sim1.dll 0x20c50290, concrete coefficient-model slot +0x74.
__declspec(noinline) double AirplaneCoefficientModel::GetFlapPitchCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    return tables->flapPitchMoment;
}

// Original: sim1.dll 0x20c502b0, concrete coefficient-model slot +0x78.
__declspec(noinline) double AirplaneCoefficientModel::GetSpoilerPitchCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    return tables->spoilerPitchMoment;
}

// Original: sim1.dll 0x20c502d0, concrete coefficient-model slot +0x7c.
__declspec(noinline) double AirplaneCoefficientModel::GetSlipstreamElevatorCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    return tables->slipstreamElevatorMoment;
}

// Original: sim1.dll 0x20c502f0, concrete coefficient-model slot +0x80.
__declspec(noinline) double AirplaneCoefficientModel::GetSlipstreamPitchRateCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    return tables->slipstreamPitchRateMoment;
}

// Original: sim1.dll 0x20c50870, concrete coefficient-model slot +0x98.
__declspec(noinline) double AirplaneCoefficientModel::GetSpoilerRollCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    return tables->spoilerRoll;
}

// Original: sim1.dll 0x20c50890, concrete coefficient-model slot +0x9c.
__declspec(noinline) double AirplaneCoefficientModel::GetAileronTrimRollCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    return tables->aileronTrimRoll;
}

// Original: sim1.dll 0x20c013a0, slipstream contribution at 0x20c016ad–0x20c0172a.
// Extracted calculation of the two tail terms in the slipstream.
double AirplaneMomentModel::CalculateSlipstreamTailPitchMoment(const AirplaneGeometry& geometry,
    const IntegrationHistory& state) const
{
    const double deflection = controlOwner->positionDispatch->controlSurfaces(controlOwner)->elevator;
    const double elevatorCoefficient = coefficients->GetSlipstreamElevatorCoefficient();
    volatile double elevatorMoment = (deflection - geometry.horizontalTailIncidence) * elevatorCoefficient *
        (geometry.referenceChord * geometry.wingArea) * state.slipstreamDynamicPressure;
    const double pitchRateMoment = coefficients->GetSlipstreamPitchRateCoefficient() *
        state.transformedAngularVelocity.x * state.slipstreamPitchRateMomentScale;
    volatile double moment = (pitchRateMoment + elevatorMoment) * state.rateCosine * state.rateCosine;
    return moment;
}

// Original: sim1.dll 0x20c013a0, SimpleAero return path at 0x20c0177b–0x20c01847.
// Extracted calculation using the original local two-sample curve.
double AirplaneMomentModel::CalculateSimplePitchMoment(double pitch, double elevatorMoment, double pitchRateMoment) const
{
    const double pitchMagnitude = fabs(pitch);
    const double lowPitch = 0.3490658503988659;
    if (pitchMagnitude > lowPitch) {
        const double elevator = controlOwner->positionDispatch->controlSurfaces(controlOwner)->elevator;
        if (elevator * pitch < 1.0) {
            const AerodynamicCurveStorage<2> pitchScale = {{2}, {{lowPitch, 1.0}, {0.7853981633974483, 0.2}}};
            elevatorMoment *= SampleAerodynamicCurve(&pitchScale.curve, pitchMagnitude);
            pitchRateMoment += pitchRateMoment;
        }
    }
    return pitchRateMoment * 3.0 + elevatorMoment * 0.4;
}

// Original: sim1.dll 0x20c013a0, pitch component of the normal airplane aerodynamic model.
double AirplaneMomentModel::CalculatePitchMoment(const AirplaneDynamics& airplane, const Vector3& force) const
{
    if (coefficients == 0) {
        return 0.0;
    }
    const IntegrationHistory& state = *airplane.history;
    const AirplaneGeometry& geometry = *airplane.airframeConfiguration->geometry;
    const double angleDegrees = state.aerodynamicAngleDegrees;
    const double baseline = CalculateScaledBaselineMoment(airplane);
    const double angle = ScaleAngleCurveMoment(*coefficients, state, angleDegrees);
    const double elevator = CalculateElevatorMoment(airplane);
    const double pitchRate = CalculatePitchRateMoment(airplane);
    const double alphaRate = CalculateAngleOfAttackRateMoment(airplane);
    const double incidence = CalculateHorizontalTailIncidenceMoment(airplane);
    const double trim = CalculateElevatorTrimMoment(airplane);
    volatile double slipstream = state.aerodynamicMomentScale * slipstreamPitchCoefficient;
    volatile double flaps = coefficients->GetFlapPitchCoefficient() * state.flapDeflectionFactor * state.aerodynamicMomentScale;

    const AirplaneControlSurfaces& surfaces = *controlOwner->positionDispatch->controlSurfaces(controlOwner);
    volatile double meanSpoilerAngle = (surfaces.secondSpoilerAngle + surfaces.firstSpoilerAngle) * 0.5;
    volatile double spoilers = coefficients->GetSpoilerPitchCoefficient() * meanSpoilerAngle * state.aerodynamicMomentScale;
    const double gyro = CalculatePropellerGyroscopicMoment(airplane);
    const double slipstreamTail = CalculateSlipstreamTailPitchMoment(geometry, state);
    volatile double supplemental = airplane.supplementalAerodynamics->PitchMoment() * state.aerodynamicMomentScale;
    const double forceTransfer = CalculateForceMomentX(airplane, force);
    if (airplane.propulsionMomentControl->simpleAerodynamics != 0) {
        return CalculateSimplePitchMoment(airplane.pitchRadians, elevator, pitchRate);
    }
    return angle + baseline + elevator + pitchRate + alphaRate + incidence + trim + slipstream + flaps + spoilers +
        gyro + forceTransfer + supplemental + slipstreamTail;
}

typedef char CheckRollCoefficientSlots[offsetof(AirplaneCoefficientDispatch, sideslipRoll) == 0x84 &&
    offsetof(AirplaneCoefficientDispatch, additionalRoll) == 0xa0 ? 1 : -1];
typedef char CheckRollCoefficientFields[offsetof(AirplaneAerodynamicTables, spoilerRoll) == 0xdc &&
    offsetof(AirplaneAerodynamicTables, aileronTrimRoll) == 0xe4 ? 1 : -1];
typedef char CheckRollControlFields[offsetof(AirplaneControlSurfaces, firstAileron) == 0xd4 &&
    offsetof(AirplaneControlSurfaces, rudder) == 0xe4 && offsetof(AirplaneControlSurfaces, aileronTrim) == 0x104 ? 1 : -1];
typedef char CheckRollStateFields[offsetof(IntegrationHistory, rollMomentScale) == 0x1ec &&
    offsetof(IntegrationHistory, firstWingAttenuationInput) == 0x62c &&
    offsetof(IntegrationHistory, rollRateMomentScale) == 0x750 ? 1 : -1];
typedef char CheckRollTorqueFields[offsetof(AirplaneConfiguration, settings.torqueOnRoll) == 0x11a8 &&
    offsetof(EngineForceRecord, propellerTorque) == 0x44 && sizeof(EngineForceRecord) == 0xec &&
    offsetof(PropellerForceRecord, signedRotationFactor) == 0x44 ? 1 : -1];
typedef char CheckRollReferenceFields[offsetof(AirplaneConfiguration, highAngleModel) == 0x19fc &&
    offsetof(AirplaneHighAngleDispatch, referenceAngle) == 0x10 ? 1 : -1];

// Original: sim1.dll 0x20c01890, propeller reaction torque at 0x20c01c55–0x20c01f43.
// Extracted calculation using the original four-engine and four-propeller record layout.
__declspec(noinline) double AirplaneMomentModel::CalculatePropellerRollTorque(const AirplaneDynamics& airplane) const
{
    const IntegrationHistory& state = *airplane.history;
    const PropulsionConfiguration& topology = *state.propulsionConfiguration;
    const PropulsionConfiguration& settings = airplane.airframeConfiguration->settings;
    const PropulsionMomentControl& control = *airplane.propulsionMomentControl;
    const unsigned propellerCount = topology.propellerCount;
    if (propellerCount == 0 || control.suppressPropellerGyro != 0) {
        return 0.0;
    }
    const double gain = (1.0 - state.airRelativeVelocity.x / (settings.rollTorqueReferenceSpeed * 1.5)) *
        control.propellerTorqueGain * settings.torqueOnRoll;
    if (!(gain > 0.0)) {
        return 0.0;
    }

    unsigned propellersOnEngine[4] = {0};
    double propellerTorque[4];
    for (unsigned index = 0; index < propellerCount; ++index) {
        const unsigned engine = topology.engineForPropeller[index];
        ++propellersOnEngine[engine];
        const double torque = state.engineForces[engine].propellerTorque * gain;
        propellerTorque[index] = state.propellerForces[index].signedRotationFactor < 0.0 ? -torque : torque;
    }

    double total = 0.0;
    for (unsigned index = 0; index < propellerCount; ++index) {
        total += propellerTorque[index] / propellersOnEngine[topology.engineForPropeller[index]];
    }
    return total;
}

// Original: sim1.dll 0x20c01890, SimpleAero arithmetic at 0x20c01fde–0x20c02002.
// Extracted from the return path of the normal roll callback.
__declspec(noinline) double AirplaneMomentModel::CalculateSimpleRollMoment(double firstAileron, double secondAileron,
    double spoilers, double yawRate) const
{
    return firstAileron * 0.75 + secondAileron * 0.75 + spoilers + yawRate * 1.3;
}

// Original: sim1.dll 0x20c01890, roll component of the normal airplane aerodynamic model.
double AirplaneMomentModel::CalculateRollMoment(AirplaneDynamics& airplane, const Vector3& force) const
{
    if (coefficients == 0) {
        return 0.0;
    }
    const IntegrationHistory& state = *airplane.history;
    const AirplaneConfiguration& configuration = *airplane.airframeConfiguration;
    const AirplaneCoefficientDispatch& coefficient = *coefficients->dispatch;
    const FlightPositionDispatch& controls = *controlOwner->positionDispatch;
    volatile double slipstream = state.wingPressureForce * slipstreamRollCoefficient;
    volatile double sideslip = coefficient.sideslipRoll(coefficients) * state.sideslipRadians * state.rollMomentScale;
    const double rudderAngle = controls.controlSurfaces(controlOwner)->rudder;
    volatile double rudder = coefficient.rudderRoll(coefficients) * rudderAngle * state.rollMomentScale;
    const double firstSpoiler = controls.controlSurfaces(controlOwner)->firstSpoilerAngle;
    const double secondSpoiler = controls.controlSurfaces(controlOwner)->secondSpoilerAngle;
    volatile double spoilers = (firstSpoiler - secondSpoiler) * coefficient.spoilerRoll(coefficients) * state.rollMomentScale;
    const float trimSetting = controls.controlSurfaces(controlOwner)->aileronTrim;
    volatile double trim = coefficient.aileronTrimRoll(coefficients) * configuration.settings.aileronTrimEffectiveness *
        trimSetting * state.rollMomentScale;

    volatile double firstAileron;
    volatile double secondAileron;
    if (airplane.positionDispatch->publishesPosition(&airplane) != 0) {
        const double secondAngle = controls.controlSurfaces(controlOwner)->secondAileron;
        secondAileron = coefficient.aileronRoll(coefficients, 3) * secondAngle * state.rollMomentScale;
        const double firstAngle = controls.controlSurfaces(controlOwner)->firstAileron;
        firstAileron = coefficient.aileronRoll(coefficients, 2) * firstAngle * state.rollMomentScale;
    } else {
        const AirplaneControlSurfaces& surfaces = *controls.controlSurfaces(controlOwner);
        volatile double meanAileron = (surfaces.secondAileron + surfaces.firstAileron) * 0.5;
        firstAileron = secondAileron = coefficient.aileronRoll(coefficients, 1) * meanAileron * state.rollMomentScale;
    }

    volatile double rollRate = coefficient.rollRate(coefficients) * state.transformedAngularVelocity.z * state.rollRateMomentScale;
    if (airplane.positionDispatch->publishesPosition(&airplane) != 0) {
        const AerodynamicCurveStorage<6> highAngleScale = {{6}, {
            {0.0, 1.0}, {0.8, 1.06}, {1.0, 1.07}, {1.4, 1.08}, {1.8, 1.02}, {3.71, 0.45}
        }};
        const AirplaneHighAngleModel* angleModel = configuration.highAngleModel;
        const float referenceAngle = angleModel->dispatch->referenceAngle(angleModel);
        rollRate *= SampleAerodynamicCurve(&highAngleScale.curve, state.attitudeAngle / referenceAngle);
    }
    volatile double yawRate = coefficient.yawRateRoll(coefficients) * state.transformedAngularVelocity.y * state.rollRateMomentScale;
    volatile double additional = coefficient.additionalRoll(coefficients) * state.rollMomentScale;
    volatile double supplemental = airplane.supplementalAerodynamics->RollMoment() * state.rollMomentScale;

    if (state.firstWingAttenuationInput > 0.0 || state.secondWingAttenuationInput > 0.0) {
        const AerodynamicCurveStorage<5> attenuation = {{5}, {
            {0.0, 1.0}, {0.21, 0.15}, {0.4, 0.08}, {0.6, 0.05}, {1.0, 0.03}
        }};
        volatile double firstScale = SampleAerodynamicCurve(&attenuation.curve, state.firstWingAttenuationInput);
        const double secondScale = SampleAerodynamicCurve(&attenuation.curve, state.secondWingAttenuationInput);
        firstAileron *= firstScale;
        secondAileron *= secondScale;
        trim *= (firstScale + secondScale) * 0.5;
    }
    const double propellerTorque = CalculatePropellerRollTorque(airplane);

    double outerWing = 0.0;
    double innerWing = 0.0;
    if (wingLiftMultiplierSum < 4.0) {
        const double span = configuration.geometry->wingSpan;
        volatile double outerDifference = (secondOuterWingLift - firstOuterWingLift) * 0.5 * span;
        volatile double innerDifference = (secondInnerWingLift - firstInnerWingLift) * 0.25 * span;
        outerWing = outerDifference;
        innerWing = innerDifference;
        if (airplane.positionDispatch->publishesPosition(&airplane) == 0) {
            outerWing *= 0.5;
            innerWing *= 0.5;
        }
    }

    if (airplane.propulsionMomentControl->simpleAerodynamics != 0) {
        return CalculateSimpleRollMoment(firstAileron, secondAileron, spoilers, yawRate);
    }
    const double forceTransfer = CalculateForceMomentZ(airplane.massProperties->centerOfGravity, force);
    const double aerodynamic = firstAileron + secondAileron + trim + spoilers + yawRate + rollRate + rudder + sideslip + slipstream;
    return ((aerodynamic + propellerTorque + additional + supplemental) + forceTransfer) + outerWing + innerWing;
}

typedef char CheckYawCoefficientSlots[offsetof(AirplaneCoefficientDispatch, rollRateYaw) == 0xa4 &&
    offsetof(AirplaneCoefficientDispatch, additionalYaw) == 0xc4 ? 1 : -1];
typedef char CheckYawCoefficientFields[offsetof(AirplaneAerodynamicTables, slipstreamRudderYaw) == 0x110 &&
    offsetof(AirplaneAerodynamicTables, slipstreamRollRateYaw) == 0x118 &&
    offsetof(AirplaneAerodynamicTables, rudderTrimYaw) == 0x120 ? 1 : -1];
typedef char CheckYawInputs[offsetof(AirplaneControlSurfaces, rudderTrim) == 0x108 &&
    offsetof(AirplaneConfiguration, settings.rudderTrimEffectiveness) == 0x1174 &&
    offsetof(AirplaneConfiguration, settings.propellerDiameter) == 0x210 &&
    offsetof(AirplaneConfiguration, settings.pFactorOnYaw) == 0x11a4 &&
    offsetof(AirplaneConfiguration, settings.gyroPrecessionOnYaw) == 0x11ac &&
    offsetof(PropulsionMomentControl, pFactorGain) == 8 &&
    offsetof(IntegrationHistory, slipstreamYawRateMomentScale) == 0x770 ? 1 : -1];
typedef char CheckExternalTankState[offsetof(IntegrationHistory, firstExternalTank) == 0x3ec &&
    offsetof(IntegrationHistory, secondExternalTank) == 0x40c && offsetof(ExternalFuelTankState, flags) == 0xc &&
    offsetof(ExternalFuelTank, lateralPosition) == 4 && offsetof(ExternalFuelTank, capacity) == 0x10 ? 1 : -1];

// Original: sim1.dll 0x20c50dc0, concrete coefficient-model slot +0xb8.
__declspec(noinline) double AirplaneCoefficientModel::GetSlipstreamRudderYawCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    return tables->slipstreamRudderYaw;
}

// Original: sim1.dll 0x20c50de0, concrete coefficient-model slot +0xbc.
__declspec(noinline) double AirplaneCoefficientModel::GetSlipstreamRollRateYawCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    return tables->slipstreamRollRateYaw;
}

// Original: sim1.dll 0x20c50e00, concrete coefficient-model slot +0xc0.
__declspec(noinline) double AirplaneCoefficientModel::GetRudderTrimYawCoefficient() const
{
    if (tables == 0) {
        return 0.0;
    }
    return tables->rudderTrimYaw;
}

// Original: sim1.dll 0x20c020a0, yaw gyroscopic block at 0x20c02265–0x20c023ce.
__declspec(noinline) double AirplaneMomentModel::CalculatePropellerYawGyro(const AirplaneDynamics& airplane) const
{
    const IntegrationHistory& state = *airplane.history;
    const unsigned count = state.propulsionConfiguration->propellerCount;
    const PropulsionMomentControl& control = *airplane.propulsionMomentControl;
    if (count == 0 || control.suppressPropellerGyro != 0 || !(control.propellerGyroGain > 0.0f)) {
        return 0.0;
    }

    const PropulsionConfiguration& settings = airplane.airframeConfiguration->settings;
    const double rpmToRadiansPerSecond = 0.1047197551;
    double moment = 0.0;
    for (unsigned index = 0; index < count; ++index) {
        const double angularMomentum = state.propellerForces[index].signedRpm * rpmToRadiansPerSecond *
            settings.propellerMomentOfInertia;
        moment += -(angularMomentum * airplane.angularVelocity.x) * settings.gyroPrecessionOnYaw * control.propellerGyroGain;
    }
    return moment;
}

// Original: sim1.dll 0x20c020a0, asymmetric propeller thrust at 0x20c023cf–0x20c025dc.
__declspec(noinline) double AirplaneMomentModel::CalculatePropellerPFactorYaw(const AirplaneDynamics& airplane) const
{
    const IntegrationHistory& state = *airplane.history;
    const unsigned count = state.propulsionConfiguration->propellerCount;
    const PropulsionMomentControl& control = *airplane.propulsionMomentControl;
    if (count == 0 || control.suppressPropellerGyro != 0 || !(control.pFactorGain > 0.0f) || !(state.attitudeAngle > 0.0)) {
        return 0.0;
    }

    const AirplaneConfiguration& configuration = *airplane.airframeConfiguration;
    const AirplaneHighAngleModel* angleModel = configuration.highAngleModel;
    const double threeDegrees = 0.05235987755982989;
    const double limit = angleModel->dispatch->referenceAngle(angleModel) - threeDegrees;
    const double boundedBelow = -limit < state.attitudeAngle ? state.attitudeAngle : -limit;
    const double boundedAlpha = boundedBelow < limit ? boundedBelow : limit;
    const double arm = configuration.settings.propellerDiameter * 0.1875 * (boundedAlpha / limit) *
        configuration.settings.pFactorOnYaw * control.pFactorGain;
    double moment = 0.0;
    for (unsigned index = 0; index < count; ++index) {
        const PropellerForceRecord& propeller = state.propellerForces[index];
        const double thrustMoment = -(propeller.thrust * arm);
        moment += propeller.signedRotationFactor < 0.0 ? -thrustMoment : thrustMoment;
    }

    const double forwardVelocity = state.airRelativeVelocity.x;
    if (forwardVelocity < 20.0) {
        return 0.0;
    }
    if (forwardVelocity < 60.0) {
        moment *= (forwardVelocity - 20.0) / 40.0;
    }
    return moment;
}

// Original: sim1.dll 0x20c020a0, external tank contribution at 0x20c02680–0x20c026f3.
__declspec(noinline) double AirplaneMomentModel::CalculateExternalTankYaw(AirplaneDynamics& airplane) const
{
    if (airplane.positionDispatch->publishesPosition(&airplane) == 0) {
        return 0.0;
    }
    const IntegrationHistory& state = *airplane.history;
    const ExternalFuelTankState& first = state.firstExternalTank;
    const ExternalFuelTankState& second = state.secondExternalTank;
    const double dragCoefficient = 0.047;
    double moment = 0.0;
    if (first.configuration->capacity != 0.0f && (first.flags & 2) == 0) {
        moment += state.dynamicPressure * dragCoefficient * first.configuration->lateralPosition;
    }
    if (second.configuration->capacity != 0.0f && (second.flags & 2) == 0) {
        moment += state.dynamicPressure * dragCoefficient * second.configuration->lateralPosition;
    }
    return moment;
}

// Original: sim1.dll 0x20c020a0, SimpleAero return arithmetic at 0x20c02702–0x20c02717.
__declspec(noinline) double AirplaneMomentModel::CalculateSimpleYawMoment(double rudder, double yawRate,
    double rollRate, double sideslip) const
{
    return rudder * 0.75 + yawRate + rollRate + sideslip;
}

// Original: sim1.dll 0x20c020a0, yaw component of the normal airplane aerodynamic model.
double AirplaneMomentModel::CalculateYawMoment(AirplaneDynamics& airplane, const Vector3& force) const
{
    if (coefficients == 0) {
        return 0.0;
    }
    const IntegrationHistory& state = *airplane.history;
    const AirplaneConfiguration& configuration = *airplane.airframeConfiguration;
    const AirplaneGeometry& geometry = *configuration.geometry;
    const AirplaneCoefficientDispatch& coefficient = *coefficients->dispatch;
    const FlightPositionDispatch& controls = *controlOwner->positionDispatch;

    volatile double slipstream = state.wingPressureForce * slipstreamYawCoefficient;
    volatile double additional = coefficient.additionalYaw(coefficients) * state.rollMomentScale;
    volatile double yawRate = coefficient.yawRate(coefficients) * state.transformedAngularVelocity.y * state.rollRateMomentScale;
    if (airplane.positionDispatch->publishesPosition(&airplane) != 0) {
        // The original roll and yaw rate paths read the same six pairs at 0x20c71d10.
        const AerodynamicCurveStorage<6> highAngleScale = {{6}, {
            {0.0, 1.0}, {0.8, 1.06}, {1.0, 1.07}, {1.4, 1.08}, {1.8, 1.02}, {3.71, 0.45}
        }};
        const AirplaneHighAngleModel* angleModel = configuration.highAngleModel;
        const float referenceAngle = angleModel->dispatch->referenceAngle(angleModel);
        yawRate *= SampleAerodynamicCurve(&highAngleScale.curve, state.attitudeAngle / referenceAngle);
    }

    const double rudderDeflection = controls.controlSurfaces(controlOwner)->rudder;
    volatile double rudder = coefficient.rudderYaw(coefficients) * rudderDeflection * state.rollMomentScale;
    const float trimSetting = controls.controlSurfaces(controlOwner)->rudderTrim;
    volatile double trim = coefficient.rudderTrimYaw(coefficients) * configuration.settings.rudderTrimEffectiveness *
        trimSetting * state.rollMomentScale;
    volatile double rollRate = coefficient.rollRateYaw(coefficients) * state.transformedAngularVelocity.z * state.rollRateMomentScale;
    volatile double sideslip = coefficient.sideslipYaw(coefficients) * state.sideslipRadians * state.rollMomentScale;
    const AirplaneControlSurfaces& surfaces = *controls.controlSurfaces(controlOwner);
    volatile double meanAileron = (surfaces.secondAileron + surfaces.firstAileron) * 0.5;
    volatile double ailerons = coefficient.aileronYaw(coefficients) * meanAileron * state.rollMomentScale;

    volatile double gyro = CalculatePropellerYawGyro(airplane);
    volatile double pFactor = CalculatePropellerPFactorYaw(airplane);
    const double slipstreamRudderCoefficient = coefficient.slipstreamRudderYaw(coefficients);
    volatile double slipstreamRudder = (slipstreamRudderCoefficient * rudderDeflection) *
        (geometry.wingSpan * 12.0 * geometry.wingArea) * state.slipstreamDynamicPressure;
    const double slipstreamRate = coefficient.slipstreamRollRateYaw(coefficients) *
        state.transformedAngularVelocity.z * state.slipstreamYawRateMomentScale;
    volatile double slipstreamTail = slipstreamRate * state.rateCosine * state.rateCosine + slipstreamRudder;
    volatile double forceTransfer = CalculateForceMomentY(airplane, force);
    const double externalTanks = CalculateExternalTankYaw(airplane);
    if (airplane.propulsionMomentControl->simpleAerodynamics != 0) {
        return CalculateSimpleYawMoment(rudder, yawRate, rollRate, sideslip);
    }
    return rudder + yawRate + trim + rollRate + sideslip + ailerons + additional + slipstream +
        gyro + pFactor + forceTransfer + externalTanks + slipstreamTail;
}

}
