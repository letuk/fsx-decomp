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

struct AerodynamicCurve {
    unsigned count;
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
    char unknownd4[0x28];
    double elevatorTrim;
};

struct AirplaneAerodynamicTables {
    char unknown00[0x54];
    double elevatorMoment;
    char unknown5c[8];
    double pitchRateMoment;
    double zeroAlphaMoment;
    double angleOfAttackRateMoment;
    char unknown7c[0x30];
    double elevatorTrimCoupling;
    char unknownb4[0x47c];
    short machElevatorMoment[17];
    short machAngleOfAttackRateMoment[17];
    short machPitchRateMoment[17];
    char unknown596[0x22];
    short machPitchMoment[17];
    char unknown5da[0x536];
    AerodynamicCurveStorage<5> elevatorAlphaCurve;
    char unknownb64[0x54];
    AerodynamicCurveStorage<5> angleOfAttackRateAlphaCurve;
    AerodynamicCurveStorage<5> pitchRateAlphaCurve;
    char unknownc60[0x54];
    AerodynamicCurveStorage<7> elevatorDeflectionCurve;
    char unknownd28[0x3dc];
    AerodynamicCurveStorage<47> angleMoment;
};

struct AirplaneCoefficientModel {
    double CalculateBaselineMoment() const;
    double CalculateElevatorCoefficient() const;
    double CalculatePitchRateCoefficient() const;
    double CalculateAngleOfAttackRateCoefficient() const;

    const void* dispatch;
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

// Original: sim1.dll 0x20c013a0, scaled angle term through 0x20c01437.
// Extracted calculation of the angle-dependent contribution from slot +0x54.
double AirplaneMomentModel::CalculateAngleCurveMoment(const AirplaneDynamics& airplane) const
{
    if (coefficients == 0) {
        return 0.0;
    }
    const IntegrationHistory& state = *airplane.history;
    const double degreesToRadians = 0.017453292519943295;
    const double angle = state.aerodynamicAngleDegrees * degreesToRadians;
    // The original stores the scaled curve contribution before adding it to the sum.
    volatile double moment = SampleAerodynamicCurve(&coefficients->tables->angleMoment.curve, angle) *
        state.aerodynamicMomentScale;
    return moment;
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

}
