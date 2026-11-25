#include "flight_dynamics.h"
#include "aerodynamic_curve.h"
#include "electrical.h"

#include <float.h>
#include <math.h>

namespace sim1 {

// Original import: util.dll ordinal 199 (entry 0x20f51990).
extern "C" __declspec(dllimport) double __stdcall AdvanceWrappedAngle(double angle, double increment);

#pragma pack(push, 4)

// Reconstructed turbine state with the verified per-engine stride.
struct TurbineEngineState {
    int HasAutomaticFeatherTorque() const;

    double turbineSpeed;
    char unknown08[0x20];
    double normalizedShaftTorque;
    char unknown30[0x20];
    double speedScaledOutput;
    char unknown58[0x28];
};

// Reconstructed minimum layout of the inputs to one propeller step.
struct PropellerStepInputs {
    void UpdateAdvanceRatio(const IntegrationHistory& state, const PropellerForceRecord& propeller);

    double angularSpeed;
    double aerodynamicTorque;
    double rotationsPerSecond;
    double aerodynamicPower;
    double diskArea;
    double advanceRatio;
};

// Reconstructed scratch record shared by all propellers mapped to one engine.
struct PropellerEngineStep {
    double propellerCount;
    unsigned lastPropellerIndex;
    double torqueShare;
};

#pragma pack(pop)

typedef char CheckEngineMountOrientation[offsetof(EngineMount, orientation) == 0x18 ? 1 : -1];
typedef char CheckEngineForceStride[sizeof(EngineForceRecord) == 0xec ? 1 : -1];
typedef char CheckEngineThrust[offsetof(EngineForceRecord, thrust) == 0x34 ? 1 : -1];
typedef char CheckInducedAirSpeed[offsetof(EngineForceRecord, inducedAirSpeed) == 0x3c ? 1 : -1];
typedef char CheckPropellerDiskArea[offsetof(PropellerStepInputs, diskArea) == 0x20 ? 1 : -1];
typedef char CheckPropellerStepSize[sizeof(PropellerStepInputs) == 0x30 ? 1 : -1];
typedef char CheckPropellerEngineStepSize[sizeof(PropellerEngineStep) == 0x14 ? 1 : -1];
typedef char CheckPropellerEngineRpm[offsetof(EngineForceRecord, propellerDerivedRpm) == 0x4c ? 1 : -1];
typedef char CheckPropellerPhase[offsetof(PropellerForceRecord, rotationPhase) == 0x10 ? 1 : -1];
typedef char CheckPropellerLoadFactor[offsetof(PropellerForceRecord, aerodynamicLoadFactor) == 0x3c ? 1 : -1];
typedef char CheckAmbientTemperature[offsetof(FlightEnvironment, ambientTemperatureCelsius) == 0x40 ? 1 : -1];
typedef char CheckIcingEnvironment[offsetof(FlightEnvironment, icingEnvironmentActive) == 0x50 ? 1 : -1];
typedef char CheckIcingSeverity[offsetof(FlightEnvironment, icingSeverity) == 0x54 ? 1 : -1];
typedef char CheckPropellerDeiceSwitch[offsetof(IntegrationHistory, propellerDeiceSwitch) == 0x610 ? 1 : -1];
typedef char CheckAutomaticFeatherSwitch[offsetof(IntegrationHistory, automaticFeatherSwitch) == 0x520 ? 1 : -1];
typedef char CheckPropellerStopFlag[offsetof(IntegrationHistory, propellerStopRequested) == 0x488 ? 1 : -1];
typedef char CheckPropellerRpmReference[offsetof(IntegrationHistory, propellerRpmReference) == 0x61c ? 1 : -1];
typedef char CheckPropellerGearRatio[offsetof(PropulsionConfiguration, gearReductionRatio) == 0x240 ? 1 : -1];
typedef char CheckPropellerEngineType[offsetof(PropulsionConfiguration, engineType) == 0x46c ? 1 : -1];
typedef char CheckPropellerBladePitch[offsetof(PropellerForceRecord, bladePitch) == 0x1c ? 1 : -1];
typedef char CheckStepRotationsPerSecond[offsetof(PropellerStepInputs, rotationsPerSecond) == 0x10 ? 1 : -1];
typedef char CheckStepAdvanceRatio[offsetof(PropellerStepInputs, advanceRatio) == 0x28 ? 1 : -1];
typedef char CheckFixedBladePitch[offsetof(PropulsionConfiguration, fixedBladePitch) == 0x258 ? 1 : -1];
typedef char CheckFixedPitchCruise[offsetof(PropulsionConfiguration, fixedPitchCruise) == 0x1984 ? 1 : -1];
typedef char CheckFixedPitchCruiseSize[sizeof(FixedPitchCruiseSetting) == 0xc ? 1 : -1];
typedef char CheckPropellerType[offsetof(PropulsionConfiguration, propellerType) == 0x208 ? 1 : -1];
typedef char CheckQualifiedGroundContacts[offsetof(IntegrationHistory, qualifiedGroundContactCount) == 0x214 ? 1 : -1];
typedef char CheckGroundSurfaceModeSlot[offsetof(GroundReferenceDispatch, surfaceMode) == 0xc ? 1 : -1];
typedef char CheckGovernorGain[offsetof(PropulsionConfiguration, governorGain) == 0x238 ? 1 : -1];
typedef char CheckGovernorCommand[offsetof(EngineForceRecord, governorCommand) == 0x1c ? 1 : -1];
typedef char CheckGovernorLowSpeedLatch[offsetof(PropellerForceRecord, governorLowSpeedLatch) == 0x24 ? 1 : -1];
typedef char CheckMinimumRpmForFeather[offsetof(PropulsionConfiguration, minimumRpmForFeather) == 0x27c ? 1 : -1];
typedef char CheckAutomaticFeatherArmed[offsetof(PropellerForceRecord, automaticFeatherArmed) == 0x34 ? 1 : -1];
typedef char CheckFeatherActive[offsetof(PropellerForceRecord, featherActive) == 0x38 ? 1 : -1];
typedef char CheckFeatherRequestTime[offsetof(PropellerForceRecord, featherRequestTime) == 0x50 ? 1 : -1];
typedef char CheckFeatheringSwitches[offsetof(PropulsionConfiguration, featheringSwitches) == 0x298 ? 1 : -1];
typedef char CheckTurbineShaftTorque[offsetof(TurbineEngineState, normalizedShaftTorque) == 0x28 ? 1 : -1];
typedef char CheckTurbineEngines[offsetof(IntegrationHistory, turbineEngines) == 0x434 ? 1 : -1];
typedef char CheckReverseControlScale[offsetof(PropulsionConfiguration, reverseControlScale) == 0x474 ? 1 : -1];
typedef char CheckPropellerFeatherPitch[offsetof(PropulsionConfiguration, featherBladePitch) == 0x284 ? 1 : -1];
typedef char CheckPropellerFeatherPower[offsetof(PropulsionConfiguration, featherPowerCoefficient) == 0x28c ? 1 : -1];
typedef char CheckPropellerPowerTable[offsetof(PropulsionConfiguration, powerCoefficientTable) == 0xbac ? 1 : -1];
typedef char CheckPropellerThrustTable[offsetof(PropulsionConfiguration, thrustEfficiencyTable) == 0x664 ? 1 : -1];
typedef char CheckPropellerThrustScalar[offsetof(PropulsionConfiguration, propellerThrustScalar) == 0x11c8 ? 1 : -1];
typedef char CheckPropellerLowSpeedLimit[offsetof(PropulsionConfiguration, lowSpeedTheoryLimit) == 0x260 ? 1 : -1];
typedef char CheckPropellerReverseFactor[offsetof(PropulsionConfiguration, reverseThrustFactor) == 0x29c ? 1 : -1];
typedef char CheckPropellerPowerTableSize[sizeof(PropellerCoefficientTable) == 0x548 ? 1 : -1];
typedef char CheckPropellerDiameterPower[offsetof(IntegrationHistory, propellerDiameterToFifth) == 0x550 ? 1 : -1];
typedef char CheckTurbineSpeed[offsetof(EngineForceRecord, speedFraction) == 0x5c ? 1 : -1];
typedef char CheckTurbineOutput[offsetof(TurbineEngineState, speedScaledOutput) == 0x50 ? 1 : -1];
typedef char CheckTurbineStateStride[sizeof(TurbineEngineState) == 0x80 ? 1 : -1];
typedef char CheckPropellerForceStride[sizeof(PropellerForceRecord) == 0x54 ? 1 : -1];
typedef char CheckPropellerSyncCorrection[offsetof(PropellerForceRecord, synchronizationCorrection) == 0x2c ? 1 : -1];
typedef char CheckPropellerSyncAvailable[offsetof(PropulsionConfiguration, propellerSyncAvailable) == 0x268 ? 1 : -1];
typedef char CheckPropellerSyncEnabled[offsetof(IntegrationHistory, propellerSynchronizationEnabled) == 0x51c ? 1 : -1];
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

// Original: sim1.dll 0x20c48710.
void FlightDynamics::UpdatePropellerLoadFactor(double step) const
{
    const unsigned propellerDeiceCircuit = 0x11;
    static const double accumulationRates[] = {0.0, 0.0004, 0.0008, 0.0012, 0.0016};
    const unsigned severity = environment->icingSeverity;
    const float temperature = static_cast<float>(environment->ambientTemperatureCelsius);
    volatile double increment;
    if (!(temperature <= 0.0f) || severity == 0 || severity >= sizeof(accumulationRates) / sizeof(accumulationRates[0])) {
        increment = step * -0.016;
    } else if (environment->icingEnvironmentActive == 0) {
        increment = step * -0.0016;
    } else {
        increment = accumulationRates[severity] * step;
    }

    const IntegrationHistory& state = *history;
    const bool deice = state.propellerDeiceSwitch != 0 &&
        state.electrical->CircuitAvailability(*this, propellerDeiceCircuit) != 0;
    for (unsigned i = 0; i < state.propulsionConfiguration->propellerCount; ++i) {
        volatile double& load = state.propellerForces[i].aerodynamicLoadFactor;
        if (deice) {
            load = 0.0;
        } else {
            load += increment;
            const double updated = load;
            load = updated > 0.0 ? (updated < 1.0 ? updated : 1.0) : 0.0;
        }
    }
}

// Original: sim1.dll 0x20c48250.
__declspec(noinline) int TurbineEngineState::HasAutomaticFeatherTorque() const
{
    return normalizedShaftTorque > 0.17;
}

// Original: sim1.dll 0x20c48270.
__declspec(noinline) void FlightDynamics::UpdateAutomaticFeatherArming() const
{
    const IntegrationHistory& state = *history;
    const unsigned propellerCount = state.propulsionConfiguration->propellerCount;
    PropellerForceRecord* propellers = state.propellerForces;
    if (state.propulsionConfiguration->engineType != 5) {
        for (unsigned i = 0; i < propellerCount; ++i) {
            propellers[i].automaticFeatherArmed = 0;
        }
        return;
    }

    const unsigned automaticFeatherCircuit = 0x09;
    const double minimumTurbineSpeed = 88.0;
    const bool powered = state.automaticFeatherSwitch != 0 &&
        state.electrical->CircuitAvailability(*this, automaticFeatherCircuit) != 0;
    const TurbineEngineState* turbines = state.turbineEngines;
    switch (propellerCount) {
    case 1:
        propellers[0].automaticFeatherArmed = powered && turbines[0].turbineSpeed > minimumTurbineSpeed &&
            turbines[0].HasAutomaticFeatherTorque();
        break;
    case 2:
        propellers[0].automaticFeatherArmed = powered && turbines[0].turbineSpeed > minimumTurbineSpeed &&
            turbines[1].HasAutomaticFeatherTorque();
        propellers[1].automaticFeatherArmed = powered && turbines[1].turbineSpeed > minimumTurbineSpeed &&
            turbines[0].HasAutomaticFeatherTorque();
        break;
    case 3:
        propellers[0].automaticFeatherArmed = powered && turbines[0].turbineSpeed > minimumTurbineSpeed &&
            turbines[2].HasAutomaticFeatherTorque();
        propellers[1].automaticFeatherArmed = powered && turbines[1].turbineSpeed > minimumTurbineSpeed &&
            turbines[0].HasAutomaticFeatherTorque() && turbines[2].HasAutomaticFeatherTorque();
        propellers[2].automaticFeatherArmed = powered && turbines[2].turbineSpeed > minimumTurbineSpeed &&
            turbines[0].HasAutomaticFeatherTorque();
        break;
    case 4:
        propellers[0].automaticFeatherArmed = powered && turbines[0].turbineSpeed > minimumTurbineSpeed &&
            turbines[2].HasAutomaticFeatherTorque();
        // The three later results all target the second propeller in the original.
        propellers[1].automaticFeatherArmed = powered && turbines[1].turbineSpeed > minimumTurbineSpeed &&
            turbines[2].HasAutomaticFeatherTorque();
        propellers[1].automaticFeatherArmed = powered && turbines[2].turbineSpeed > minimumTurbineSpeed &&
            turbines[1].HasAutomaticFeatherTorque();
        propellers[1].automaticFeatherArmed = powered && turbines[3].turbineSpeed > minimumTurbineSpeed &&
            turbines[1].HasAutomaticFeatherTorque();
        break;
    }
}

// Original: sim1.dll 0x20c47420, with the mapped passes extracted into a separate method.
void FlightDynamics::UpdateMappedPropellers(double step) const
{
    UpdateAutomaticFeatherArming();
    UpdatePropellerLoadFactor(step);
    UpdateMappedPropellersAfterPreparation(step);
}

// Original: sim1.dll 0x20c47420, extracted mapped passes at 0x20c47456–0x20c47713.
void FlightDynamics::UpdateMappedPropellersAfterPreparation(double step) const
{
    const IntegrationHistory& state = *history;
    const PropulsionConfiguration& settings = *state.propulsionConfiguration;
    const unsigned engineCount = settings.engineCount;
    const unsigned propellerCount = settings.propellerCount;
    if (engineCount > 4 || propellerCount > 4) {
        return;
    }

    PropellerEngineStep engineSteps[4];
    PropellerStepInputs propellerSteps[4];
    for (unsigned i = 0; i < engineCount; ++i) {
        engineSteps[i].propellerCount = 0.0;
    }
    for (unsigned i = 0; i < propellerCount; ++i) {
        const unsigned engineIndex = settings.engineForPropeller[i];
        if (engineIndex >= engineCount) {
            return;
        }
        PropellerEngineStep& engineStep = engineSteps[engineIndex];
        engineStep.propellerCount += 1.0;
        engineStep.lastPropellerIndex = i;
    }

    for (unsigned i = 0; i < propellerCount; ++i) {
        const unsigned engineIndex = settings.engineForPropeller[i];
        PropellerStepInputs& inputs = propellerSteps[i];
        inputs.diskArea = settings.propellerDiameter * 0.7853981633974483 * settings.propellerDiameter;
        inputs.angularSpeed = (state.propellerRpmReference * 0.1047197551 * state.engineForces[engineIndex].speedFraction) /
            settings.gearReductionRatio;
        UpdatePropellerBladePitch(i, engineIndex, inputs, step);
        inputs.aerodynamicTorque = state.propellerForces[i].CalculateAerodynamicTorque(*this, inputs);
    }

    for (unsigned i = 0; i < propellerCount; ++i) {
        const unsigned engineIndex = settings.engineForPropeller[i];
        PropellerEngineStep& engineStep = engineSteps[engineIndex];
        engineStep.torqueShare = state.engineForces[engineIndex].propellerTorque / engineStep.propellerCount;
        state.propellerForces[i].IntegrateSpeed(state, propellerSteps[i], engineStep.torqueShare, step);
    }

    for (unsigned i = 0; i < engineCount; ++i) {
        const PropellerEngineStep& engineStep = engineSteps[i];
        if (engineStep.propellerCount != 0.0) {
            EngineForceRecord& engine = state.engineForces[i];
            const PropellerForceRecord& propeller = state.propellerForces[engineStep.lastPropellerIndex];
            engine.speedFraction = propeller.normalizedSpeed;
            if (propeller.signedRotationFactor < 0.0) {
                engine.speedFraction = -engine.speedFraction;
            }
            engine.propellerDerivedRpm = state.propellerRpmReference * engine.speedFraction;
        }
    }

    for (unsigned i = 0; i < propellerCount; ++i) {
        const unsigned engineIndex = settings.engineForPropeller[i];
        PropellerForceRecord& propeller = state.propellerForces[i];
        EngineForceRecord& engine = state.engineForces[engineIndex];
        const PropellerStepInputs& inputs = propellerSteps[i];
        propeller.UpdateThrust(state, inputs, engineSteps[engineIndex].torqueShare);
        propeller.UpdateSynchronization(state, engine, step);
        engine.UpdateInducedAirSpeed(*this, propeller, inputs);
    }
}

// Original: sim1.dll 0x20c480b0
__declspec(noinline) void EngineForceRecord::UpdateInducedAirSpeed(const FlightDynamics& airplane,
                                                                   const PropellerForceRecord& propeller,
                                                                   const PropellerStepInputs& step)
{
    const IntegrationHistory& state = *airplane.history;
    const double forwardSpeed = state.airRelativeVelocity.x > 0.0 ? state.airRelativeVelocity.x : 0.0;
    const double thrust = propeller.thrust > 0.0 ? propeller.thrust : 0.0;
    const FlightEnvironment& environment = *airplane.environment;
    if (environment.airDensity > 0.0) {
        const double density = environment.airDensity;
        const double loading = thrust / ((density + density) * step.diskArea);
        inducedAirSpeed = sqrt(loading + (forwardSpeed * 0.25) * forwardSpeed) - forwardSpeed * 0.5;
    } else {
        inducedAirSpeed = 0.0;
    }
}

// Original: sim1.dll 0x20c1cbf0
__declspec(noinline) int FlightDynamics::UsesGroundContactLimits() const
{
    if (history->qualifiedGroundContactCount != 0 && history->flapDeflectionFactor >= 1.0) {
        if (groundReference->dispatch->surfaceMode(groundReference) != 2) {
            return 1;
        }
    }
    return 0;
}

// Original: sim1.dll 0x20c47720. The extracted calculations retain their original ranges below.
void FlightDynamics::UpdatePropellerBladePitch(unsigned propellerIndex, unsigned engineIndex,
                                               PropellerStepInputs& inputs, double step) const
{
    const IntegrationHistory& state = *history;
    PropellerForceRecord& propeller = state.propellerForces[propellerIndex];
    inputs.UpdateAdvanceRatio(state, propeller);
    if (state.propulsionConfiguration->propellerType == 0) {
        const EngineForceRecord& engine = state.engineForces[engineIndex];
        const bool feather = propeller.UpdateGovernorFeatherState(state, engineIndex, inputs, step) != 0;
        propeller.ApplyGovernorPitch(*this, engine, inputs, step, feather);
    } else {
        propeller.UpdateFixedPitch(*this);
    }
}

// Original: sim1.dll 0x20c47720, extracted advance-ratio calculation at 0x20c4772e–0x20c477e2.
__declspec(noinline) void PropellerStepInputs::UpdateAdvanceRatio(const IntegrationHistory& state,
                                                                 const PropellerForceRecord& propeller)
{
    const double speed = angularSpeed / 6.283185307179586;
    rotationsPerSecond = speed;
    if (fabs(speed) < 0.0001) {
        advanceRatio = 999.0;
    } else {
        const double axialSpeed = state.airRelativeVelocity.x;
        advanceRatio = (axialSpeed > 0.0 ? axialSpeed : 0.0) / (state.propulsionConfiguration->propellerDiameter * speed);
        const double normalizedSpeed = fabs(propeller.normalizedSpeed);
        if (normalizedSpeed < 0.25) {
            const double gain = (normalizedSpeed - 0.125) * 8.0;
            advanceRatio *= gain > 0.0 ? gain : 0.0;
        }
    }
}

// Original: sim1.dll 0x20c47720, extracted fixed-pitch branch at 0x20c47a2f–0x20c47ad0.
__declspec(noinline) void PropellerForceRecord::UpdateFixedPitch(const FlightDynamics& airplane)
{
    const IntegrationHistory& state = *airplane.history;
    const PropulsionConfiguration& settings = *state.propulsionConfiguration;
    if (settings.propellerType != 1) {
        return;
    }
    const FixedPitchCruiseSetting* cruise = settings.fixedPitchCruise;
    if (cruise == 0) {
        bladePitch = settings.fixedBladePitch;
    } else if (bladePitch == settings.fixedBladePitch) {
        if (state.airRelativeVelocity.x > cruise->activationSpeed) {
            bladePitch = cruise->bladePitch;
        }
    } else if (bladePitch != cruise->bladePitch ||
               (airplane.propulsionMomentBlendInput < 5.0 && normalizedSpeed == 0.0)) {
        bladePitch = settings.fixedBladePitch;
    }
}

// Original: sim1.dll 0x20c485c0
__declspec(noinline) int PropellerForceRecord::UpdateFeatherState(const IntegrationHistory& state, unsigned engineIndex,
                                                                  double step)
{
    if (featherRequestTime != 0.0f) {
        featherRequestTime = featherRequestTime - static_cast<float>(step);
        featherRequestTime = featherRequestTime > 0.0f ? featherRequestTime : 0.0f;
    }

    const PropulsionConfiguration& settings = *state.propulsionConfiguration;
    const bool automatic = settings.engineType == 5;
    const double automaticTorqueThreshold = 0.10000000149011612;
    if (featherActive == 0) {
        if ((automatic && state.turbineEngines[engineIndex].normalizedShaftTorque < automaticTorqueThreshold &&
             automaticFeatherArmed != 0) ||
            (featherRequestTime != 0.0f && fabs(state.propellerForces[engineIndex].normalizedSpeed) > 0.5)) {
            featherActive = 1;
        }
    } else if ((automatic && (automaticFeatherArmed == 0 ||
               state.turbineEngines[engineIndex].normalizedShaftTorque > automaticTorqueThreshold)) ||
               (settings.featheringSwitches != 0 && fabs(bladePitch - settings.featherBladePitch) < 0.0001 &&
                fabs(state.propellerForces[engineIndex].normalizedSpeed) < 0.0001)) {
        featherActive = 0;
    }
    return featherActive;
}

// Original: sim1.dll 0x20c47720, extracted latch and feather call at 0x20c47818–0x20c47878.
__declspec(noinline) int PropellerForceRecord::UpdateGovernorFeatherState(const IntegrationHistory& state,
                                                                          unsigned engineIndex,
                                                                          const PropellerStepInputs& inputs, double step)
{
    const PropulsionConfiguration& settings = *state.propulsionConfiguration;
    const double rpm = inputs.angularSpeed * 9.549296586;
    const double command = state.engineForces[engineIndex].governorCommand;
    if (governorLowSpeedLatch == 0) {
        if (rpm < settings.minimumRpmForFeather && command > 0.0 && featherActive == 0) {
            governorLowSpeedLatch = 1;
        }
    } else if (rpm >= settings.minimumRpmForFeather && command > 0.0) {
        governorLowSpeedLatch = 0;
    }
    return UpdateFeatherState(state, engineIndex, step);
}

// Original: sim1.dll 0x20c47720, extracted governor calculation at 0x20c47879–0x20c47a2e.
__declspec(noinline) void PropellerForceRecord::ApplyGovernorPitch(const FlightDynamics& airplane,
                                                                   const EngineForceRecord& engine,
                                                                   const PropellerStepInputs& inputs, double step,
                                                                   bool featherActive)
{
    const IntegrationHistory& state = *airplane.history;
    const PropulsionConfiguration& settings = *state.propulsionConfiguration;
    const double command = engine.governorCommand;
    const double reverseInput = engine.reverseControlInput;
    const bool reversing = settings.reverseThrustFactor > 0.0f && reverseInput < 0.0;
    double upperLimit = settings.featherBladePitch;
    double targetSpeed = 0.0;
    if (governorLowSpeedLatch || (!featherActive && !(command < 0.0))) {
        const double correctedCommand = command + synchronizationCorrection;
        const double fraction = correctedCommand < 1.0 ? correctedCommand : 1.0;
        targetSpeed = ((state.propellerRpmReference - settings.minimumGovernorRpm) * fraction +
            settings.minimumGovernorRpm) * 0.1047197551;
        upperLimit = settings.maximumBladePitch;
    }

    double pitchRate = 0.0;
    if (!(fabs(bladePitch - settings.featherBladePitch) < 0.0001) || !(command < 0.0)) {
        if (reversing) {
            pitchRate = -0.2;
        } else {
            pitchRate = (settings.gearReductionRatio * inputs.angularSpeed - targetSpeed) * settings.governorGain;
            if (bladePitch > settings.featherBladePitch && settings.defeatheringAccumulatorAvailable) {
                pitchRate += 0.0010000000474974513;
            }
        }
    }

    double lowerLimit = settings.minimumBladePitch;
    if (settings.reverseThrustFactor > 0.0f && reverseInput <= 0.007 && airplane.UsesGroundContactLimits()) {
        lowerLimit = settings.minimumGroundBladePitch;
    }
    if (reversing) {
        lowerLimit = ((settings.minimumGroundBladePitch - settings.minimumReverseBladePitch) /
            -settings.reverseControlScale) * reverseInput + settings.minimumGroundBladePitch;
    }

    bladePitch += pitchRate * step;
    bladePitch = bladePitch < upperLimit ? bladePitch : upperLimit;
    if (pitchRate < 0.0 && !(bladePitch > lowerLimit)) {
        bladePitch = lowerLimit;
    }
}

struct CoefficientInterval {
    unsigned lower;
    unsigned upper;
    double fraction;
};

// Reconstruction-only helper shared by both axes of the coefficient table.
static __forceinline CoefficientInterval FindCoefficientInterval(const double* axis, unsigned count, unsigned stride,
                                                                 double value)
{
    CoefficientInterval interval = {1, 2, 0.0};
    if (value <= axis[stride]) {
        return interval;
    }

    const unsigned last = count - 1;
    if (value > axis[last * stride]) {
        interval.lower = last - 1;
        interval.upper = last;
        interval.fraction = 1.0;
        return interval;
    }

    unsigned upper = 2;
    while (upper < last && axis[upper * stride] < value) {
        ++upper;
    }
    if (value == axis[upper * stride]) {
        interval.lower = upper;
        interval.upper = upper < last ? upper + 1 : upper;
    } else {
        interval.lower = upper - 1;
        interval.upper = upper;
        const double lowerValue = axis[interval.lower * stride];
        interval.fraction = (value - lowerValue) / (axis[upper * stride] - lowerValue);
    }
    return interval;
}

// Original: sim1.dll 0x20bcc030
__declspec(noinline) double PropellerCoefficientTable::Evaluate(double advanceRatio, double bladePitchDegrees) const
{
    if (_isnan(advanceRatio) || _isnan(bladePitchDegrees)) {
        return values[columns + 1];
    }

    const CoefficientInterval pitch = FindCoefficientInterval(values, rows, columns, bladePitchDegrees);
    const CoefficientInterval advance = FindCoefficientInterval(values, columns, 1, advanceRatio);
    const double* lowerRow = values + pitch.lower * columns;
    const double* upperRow = values + pitch.upper * columns;
    const double left = (upperRow[advance.lower] - lowerRow[advance.lower]) * pitch.fraction + lowerRow[advance.lower];
    const double right = (upperRow[advance.upper] - lowerRow[advance.upper]) * pitch.fraction + lowerRow[advance.upper];
    return left + (right - left) * advance.fraction;
}

// Original: sim1.dll 0x20c47ae0
double __fastcall PropellerForceRecord::CalculateAerodynamicTorque(const FlightDynamics& airplane,
                                                                   const PropellerStepInputs& inputs) const
{
    const IntegrationHistory& state = *airplane.history;
    const PropulsionConfiguration& settings = *state.propulsionConfiguration;
    double coefficient;
    if (fabs(bladePitch - settings.featherBladePitch) < 0.0001) {
        coefficient = settings.featherPowerCoefficient;
    } else {
        double lookupPitch = bladePitch;
        double advanceRatio = inputs.advanceRatio;
        if (lookupPitch < 0.0) {
            lookupPitch = -lookupPitch;
            advanceRatio = 0.0;
        }
        coefficient = settings.powerCoefficientTable.Evaluate(advanceRatio, lookupPitch * 57.29577951308232);
    }
    if (inputs.angularSpeed < 0.0) {
        coefficient = -coefficient;
    }
    const double speedSquared = inputs.rotationsPerSecond * inputs.rotationsPerSecond;
    return ((airplane.environment->airDensity * coefficient) * speedSquared * state.propellerDiameterToFifth) /
        6.283185307179586;
}

// Original: sim1.dll 0x20c47ba0
void __fastcall PropellerForceRecord::IntegrateSpeed(const IntegrationHistory& state, PropellerStepInputs& inputs,
                                                     double engineTorque, double step)
{
    const PropulsionConfiguration& settings = *state.propulsionConfiguration;
    inputs.aerodynamicPower = inputs.aerodynamicTorque * inputs.angularSpeed;
    if (settings.engineType == 0) {
        engineTorque = settings.gearReductionRatio * engineTorque;
    }

    const double load = (aerodynamicLoadFactor * 0.3 + 1.0) * inputs.aerodynamicTorque;
    double acceleration = (engineTorque - load) / settings.propellerMomentOfInertia;
    if (state.propellerStopRequested) {
        acceleration = -(inputs.angularSpeed / step);
    }
    inputs.angularSpeed = acceleration * step + inputs.angularSpeed;
    if (fabs(inputs.angularSpeed) < 2.0 && fabs(acceleration) < 8.0) {
        inputs.angularSpeed = 0.0;
    }

    signedRpm = inputs.angularSpeed * 9.549296586;
    if (signedRotationFactor < 0.0) {
        signedRpm = -signedRpm;
    }
    normalizedSpeed = signedRpm / (state.propellerRpmReference / settings.gearReductionRatio);
    rotationPhase = static_cast<float>(AdvanceWrappedAngle(rotationPhase, signedRpm * 0.1047197551 * step));
}

// Original: sim1.dll 0x20c47c90
void PropellerForceRecord::UpdateThrust(const IntegrationHistory& state, const PropellerStepInputs& inputs,
                                        double engineTorque)
{
    static const AerodynamicCurveStorage<13> staticThrustByPowerDensity = {{13}, {
        {1.0, 7.5}, {1.5, 6.5}, {2.0, 5.9}, {2.5, 5.5}, {3.0, 5.2}, {4.0, 4.7}, {5.0, 4.4},
        {6.0, 4.0}, {8.0, 3.7}, {10.0, 3.3}, {20.0, 2.6}, {30.0, 2.0}, {40.0, 1.8}
    }};
    static const AerodynamicCurveStorage<5> staticThrustByAxialSpeed = {{5}, {
        {0.0, 0.5}, {10.0, 1.0}, {40.0, 0.82}, {60.0, 0.74}, {80.0, 0.66}
    }};
    static const AerodynamicCurveStorage<5> staticThrustByBladePitch = {{5}, {
        {-1.0, -0.3}, {0.0, 0.0}, {1.0, 0.21}, {5.0, 0.73}, {10.0, 1.0}
    }};

    const PropulsionConfiguration& settings = *state.propulsionConfiguration;
    const double axialSpeed = state.airRelativeVelocity.x;
    double airflowThrust = 0.0;
    if (!(fabs(axialSpeed) < 1.0e-7)) {
        double efficiency;
        if (bladePitch < 0.0) {
            efficiency = settings.reverseThrustFactor * -0.3;
            if (axialSpeed < 0.0) {
                const double reverseSpeedFactor = 1.0 - axialSpeed / -6.0;
                efficiency *= reverseSpeedFactor > 0.0 ? reverseSpeedFactor : 0.0;
            }
        } else {
            efficiency = settings.thrustEfficiencyTable.Evaluate(inputs.advanceRatio, bladePitch * 57.29577951308232);
        }
        airflowThrust = inputs.aerodynamicPower * efficiency / axialSpeed;
    }

    const double limit = settings.lowSpeedTheoryLimit;
    if (axialSpeed < limit) {
        const double powerDensity = (inputs.aerodynamicPower / 550.0) / inputs.diskArea;
        const double thrustPerHorsepower = SampleAerodynamicCurve(&staticThrustByPowerDensity.curve, powerDensity);
        double staticThrust = ((inputs.angularSpeed * engineTorque) / 550.0) * thrustPerHorsepower;
        if (bladePitch < 0.17453292519943295) {
            staticThrust = SampleAerodynamicCurve(&staticThrustByBladePitch.curve, bladePitch * 57.29577951308232) *
                staticThrust;
            if (staticThrust < 0.0) {
                staticThrust *= settings.reverseThrustFactor;
            }
        } else if (fabs(bladePitch - settings.featherBladePitch) < 0.0001) {
            staticThrust = 0.0;
        }
        if (bladePitch >= 0.0 && axialSpeed <= 0.0 && !(staticThrust > 0.0)) {
            staticThrust = 0.0;
        }

        const double speedFactor = SampleAerodynamicCurve(&staticThrustByAxialSpeed.curve, axialSpeed);
        thrust = (((limit - axialSpeed) * (speedFactor * staticThrust)) / limit) + (axialSpeed / limit) * airflowThrust;
    } else {
        thrust = airflowThrust;
    }
    thrust *= 1.0 - aerodynamicLoadFactor * 0.2;
    thrust *= settings.propellerThrustScalar;
}

// Original: sim1.dll 0x20c48150
void __fastcall PropellerForceRecord::UpdateSynchronization(const IntegrationHistory& state,
                                                             const EngineForceRecord& engine, double step)
{
    const PropulsionConfiguration* settings = state.propulsionConfiguration;
    const double correctionScale = settings->propellerMomentOfInertia * 0.04 * step;
    const EngineForceRecord* reference = settings && static_cast<int>(settings->engineCount) > 0 ?
        state.engineForces : 0;
    // The mapped propeller step requires an installed reference engine.
    const double speedDifference = reference->speedFraction - engine.speedFraction;
    if (settings->propellerSyncAvailable && state.propellerSynchronizationEnabled &&
        this != state.propellerForces && fabs(speedDifference) < 0.088) {
        synchronizationCorrection += speedDifference * correctionScale;
    } else {
        synchronizationCorrection = 0.0;
    }
}

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

    if (state->propulsionConfiguration->suppressFirstMoment || propulsionMomentControl->simpleAerodynamics) {
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
