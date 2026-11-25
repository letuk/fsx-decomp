#include "turbine.h"
#include "coefficient_table.h"

#include <math.h>

namespace sim1 {

// Shared turbine-loop values at sim1.dll 0x20c7ba70, 0x20c7ba78 and 0x20c7ba80.
double turbineInletTemperatureRoot;
double turbineInletPressureRatio;
double turbineInletTemperatureRatio;

// Loader-owned gravity value at sim1.dll 0x20c7c180.
extern double gravityAcceleration;

// Shared turbine-loop values at sim1.dll 0x20c7ba58 and 0x20c7ba60.
double turbineRamDrag;
double turbineGrossThrust;

typedef char CheckAuxiliaryControl[offsetof(EngineForceRecord, auxiliaryControl) == 0x04 ? 1 : -1];
typedef char CheckRunning[offsetof(EngineForceRecord, running) == 0x08 ? 1 : -1];
typedef char CheckTorqueReduction[offsetof(EngineForceRecord, shaftTorqueReductionInput) == 0x28 ? 1 : -1];
typedef char CheckStarter[offsetof(EngineForceRecord, starterEngaged) == 0x2c ? 1 : -1];
typedef char CheckSpoolDrive[offsetof(EngineForceRecord, spoolDriveSignal) == 0x6c ? 1 : -1];
typedef char CheckSpoolSchedule[offsetof(TurbineEngineState, spoolScheduleInput) == 0x20 ? 1 : -1];
typedef char CheckAuxiliarySlot[offsetof(EngineAuxiliaryDispatch, setLevel) == 0x48 ? 1 : -1];
typedef char CheckMaximumTorque[offsetof(PropulsionConfiguration, maximumTorque) == 0x2d8 ? 1 : -1];
typedef char CheckDragCurve[offsetof(PropulsionConfiguration, shaftDragBySpeed) == 0x620 ? 1 : -1];
typedef char CheckPerformanceData[offsetof(PropulsionConfiguration, turbinePerformance) == 0x19e4 ? 1 : -1];
typedef char CheckInletMachLimit[offsetof(TurbinePerformanceData, useHalfMachLimit) == 0x40 ? 1 : -1];
typedef char CheckInletMach[offsetof(TurbineEngineState, inletMach) == 0x40 ? 1 : -1];
typedef char CheckMachInput[offsetof(IntegrationHistory, mach) == 0x15c ? 1 : -1];
typedef char CheckAmbientTemperatureRatio[offsetof(IntegrationHistory, ambientTemperatureRatio) == 0x254 ? 1 : -1];
typedef char CheckAmbientPressureRatio[offsetof(IntegrationHistory, ambientPressureRatio) == 0x25c ? 1 : -1];
typedef char CheckDriveCurve[offsetof(TurbinePerformanceData, torqueByTurbineSpeed) == 0x1a44 ? 1 : -1];
typedef char CheckDensityCurve[offsetof(TurbinePerformanceData, torqueByAirDensity) == 0x1a78 ? 1 : -1];
typedef char CheckShaftSpeed[offsetof(TurbineEngineState, shaftSpeed) == 0x08 ? 1 : -1];
typedef char CheckDriveCurveInput[offsetof(TurbineEngineState, turbineTorqueCurveInput) == 0x10 ? 1 : -1];
typedef char CheckSpeedOfSound[offsetof(IntegrationHistory, speedOfSound) == 0x164 ? 1 : -1];
typedef char CheckGrossThrustTable[offsetof(TurbinePerformanceData, grossThrustTable) == 0xb20 ? 1 : -1];
typedef char CheckInletFlowTable[offsetof(TurbinePerformanceData, inletFlowTable) == 0x1260 ? 1 : -1];
typedef char CheckAfterburnerCurve[offsetof(TurbinePerformanceData, afterburnerMachCurve) == 0x19a0 ? 1 : -1];
typedef char CheckAfterburnerStage[offsetof(TurbineEngineState, afterburnerStage) == 0x7c ? 1 : -1];
typedef char CheckReverserPosition[offsetof(TurbineEngineState, reverserPosition) == 0x60 ? 1 : -1];
typedef char CheckTurbineNetThrust[offsetof(TurbineEngineState, netThrust) == 0x48 ? 1 : -1];
typedef char CheckTurbineSpeedBlend[offsetof(TurbineEngineState, filteredSpeedBlend) == 0x68 ? 1 : -1];

static const AerodynamicCurveStorage<4> spoolSpeedSchedule = {
    {4}, {{0.0, 0.0}, {100.0, 62.0}, {150.0, 50.0}, {610.0, 101.5}}
};

static const TurbineCoefficientTable<25> reverserThrustTable = {
    5, 5, {
        0.0,  0.5,  1.0, 1.15, 2.0,
        0.0,  0.95, 0.9, 1.0,  1.0,
        0.11, 0.99, 1.0, 0.9,  0.83,
        0.25, 1.0,  0.9, 0.8,  0.71,
        1.0, -0.5, -0.5, -0.5, -0.5
    }
};

// Reconstruction-only typed adapter to the shared table evaluator at sim1.dll 0x20bcc030.
template <unsigned Capacity>
__declspec(noinline) double TurbineCoefficientTable<Capacity>::Evaluate(double mach, double loading) const
{
    return EvaluateCoefficientTable(*this, mach, loading);
}

template double TurbineCoefficientTable<12>::Evaluate(double, double) const;
template double TurbineCoefficientTable<20>::Evaluate(double, double) const;
template double TurbineCoefficientTable<90>::Evaluate(double, double) const;
template double TurbineCoefficientTable<143>::Evaluate(double, double) const;

// Reconstruction-only extraction of the original inline range at sim1.dll 0x20c2b879–0x20c2b8be.
void TurbineEngineState::UpdateGrossThrust(const IntegrationHistory& history) const
{
    const TurbinePerformanceData& performance = *history.propulsionConfiguration->turbinePerformance;
    const double coefficient = performance.grossThrustTable.Evaluate(history.mach, turbineTorqueCurveInput);
    turbineGrossThrust = coefficient * performance.staticThrust * turbineInletPressureRatio;
}

// Original: sim1.dll 0x20c2c2e0. The original receives the turbine record in EDI.
void TurbineEngineState::UpdateRamDrag(const IntegrationHistory& history) const
{
    const TurbinePerformanceData& performance = *history.propulsionConfiguration->turbinePerformance;
    const double inletFlow = performance.inletFlowTable.Evaluate(inletMach, turbineTorqueCurveInput);
    const double massFlow = inletFlow * performance.inletArea * turbineInletPressureRatio / turbineInletTemperatureRoot;
    turbineRamDrag = history.speedOfSound * inletMach * massFlow / gravityAcceleration;
}

// Original: sim1.dll 0x20c2bb20. History and the turbine record originally arrive in EAX and EDX.
__declspec(noinline) float TurbineEngineState::AfterburnerFraction(const IntegrationHistory& history) const
{
    const TurbinePerformanceData* performance = history.propulsionConfiguration->turbinePerformance;
    if (performance == 0 || performance->afterburnerStages == 0) {
        return 0.0f;
    }
    return static_cast<float>(static_cast<double>(afterburnerStage) / performance->afterburnerStages);
}

// Original: sim1.dll 0x20c2c4e0. History and the turbine record originally arrive in EAX and EDX.
__declspec(noinline) double TurbineEngineState::ReverserThrustFactor(const IntegrationHistory& history) const
{
    const TurbinePerformanceData& performance = *history.propulsionConfiguration->turbinePerformance;
    if (performance.useTabulatedReverser) {
        return reverserThrustTable.Evaluate(history.mach, reverserPosition);
    }
    if (performance.reverserEffectiveness > 0.0f) {
        return 1.0 - (1.0 + performance.reverserEffectiveness * 0.375) * reverserPosition;
    }
    return 1.0;
}

// Original: sim1.dll 0x20c2c620. The original also receives history in EAX and the turbine in EBX.
void TurbineEngineState::PublishNetThrust(const IntegrationHistory& history, EngineForceRecord& engine)
{
    const volatile double reverserFactor = ReverserThrustFactor(history);
    const TurbinePerformanceData& performance = *history.propulsionConfiguration->turbinePerformance;
    double grossThrust = turbineGrossThrust;
    if (performance.afterburnerStages) {
        const float fraction = AfterburnerFraction(history);
        if (fraction > 0.0f) {
            const double factor = SampleAerodynamicCurve(&performance.afterburnerMachCurve.curve, history.mach);
            const double augmentation = fraction * (factor - 1.0);
            grossThrust *= 1.0 + (augmentation > 0.0 ? augmentation : 0.0);
        }
    }
    const double thrust = grossThrust * reverserFactor - turbineRamDrag;
    netThrust = thrust;
    engine.thrust = thrust;
}

// Reconstruction-only extraction of the original inline range at sim1.dll 0x20c2b951–0x20c2b97b.
void TurbineEngineState::UpdateSpeedBlend(double step)
{
    const double target = (shaftSpeed - turbineSpeed) * 0.01 + shaftSpeed * 0.023;
    filteredSpeedBlend += (target - filteredSpeedBlend) * 2.0 * step;
}

// Original: sim1.dll 0x20c2b9d0. The original receives history in ESI.
void __fastcall TurbineEngineState::UpdateInletConditions(const IntegrationHistory& history)
{
    const double machLimit = history.propulsionConfiguration->turbinePerformance->useHalfMachLimit ? 0.5 : 1.0;
    inletMach = history.mach < machLimit ? history.mach : machLimit;
    const volatile double& storedMach = inletMach;
    const double ramTemperatureFactor = 1.0 + storedMach * 0.2 * storedMach;
    turbineInletTemperatureRatio = ramTemperatureFactor * history.ambientTemperatureRatio;
    turbineInletPressureRatio = pow(ramTemperatureFactor, 3.5) * history.ambientPressureRatio;
    const volatile double& storedTemperatureRatio = turbineInletTemperatureRatio;
    turbineInletTemperatureRoot = sqrt(storedTemperatureRatio);
}

// Original: sim1.dll 0x20c2c6d0. EBX holds the turbine and EDI the engine in the original.
void TurbineEngineState::UpdateSpoolSpeed(const EngineForceRecord& engine, double axialSpeed, double step,
                                         double inletTemperatureRatio)
{
    double combustionTarget = 0.0;
    double responseRate = 0.5;
    if (!engine.running) {
        const double limitedAirspeed = axialSpeed < 130.0 ? axialSpeed : 130.0;
        const double responseDrop = 0.13999999999999999;
        responseRate = 0.15 - limitedAirspeed * responseDrop / 130.0;
    } else if (!(fabs(engine.spoolDriveSignal) < 0.0001)) {
        combustionTarget = SampleAerodynamicCurve(&spoolSpeedSchedule.curve, spoolScheduleInput);
    }

    double starterTarget = 0.0;
    if (engine.starterEngaged) {
        starterTarget = 12.0;
        responseRate = 0.3;
    }

    // The original applies a zero windmilling gain only to ordered positive airspeed.
    const double windmillingGain = 0.0;
    double target = axialSpeed > 0.0 ? axialSpeed * windmillingGain : 0.0;
    const double poweredTarget = starterTarget < combustionTarget ? combustionTarget : starterTarget;
    if (target < poweredTarget) {
        target = poweredTarget;
    }

    double speedRate = (target - turbineTorqueCurveInput) * responseRate;
    if (speedRate < 20.0) {
        if (speedRate <= -20.0) {
            speedRate = -20.0;
        }
    } else {
        speedRate = 20.0;
    }

    turbineTorqueCurveInput += speedRate * step;
    const volatile double& storedSpoolSpeed = turbineTorqueCurveInput;
    turbineSpeed = sqrt(inletTemperatureRatio) * storedSpoolSpeed;
}

// Original: sim1.dll 0x20c2c850. The internal original receives the engine in EBX.
void EngineForceRecord::UpdateTurbineShaftTorque(const FlightDynamics& airplane, TurbineEngineState& turbine, double step)
{
    const PropulsionConfiguration& settings = *airplane.history->propulsionConfiguration;
    // The original stores the drive component and response rate at binary64 precision.
    volatile double driveTorque;
    volatile double responseRate;
    if (running) {
        driveTorque = SampleAerodynamicCurve(&settings.turbinePerformance->torqueByTurbineSpeed.curve,
                                            turbine.turbineTorqueCurveInput) *
                      settings.maximumTorque * (1.0 - shaftTorqueReductionInput * 0.08);
        responseRate = 12.0;
    } else {
        driveTorque = 0.0;
        responseRate = turbine.normalizedShaftTorque * 4.0 + 0.1;
    }

    double targetTorque;
    if (airplane.history->propellerStopRequested) {
        targetTorque = 0.0;
        if (auxiliaryControl) {
            auxiliaryControl->dispatch->setLevel(auxiliaryControl, 0.0f, 0);
        }
    } else {
        const double densityFactor = SampleAerodynamicCurve(&settings.turbinePerformance->torqueByAirDensity.curve,
                                                           airplane.environment->airDensity);
        driveTorque = densityFactor * driveTorque;
        const double dragFactor = SampleAerodynamicCurve(&settings.shaftDragBySpeed.curve, turbine.shaftSpeed);
        targetTorque = driveTorque - dragFactor * settings.maximumTorque;
    }

    if (turbine.shaftSpeed > 5.0) {
        targetTorque = (targetTorque - propellerTorque) * responseRate * step + propellerTorque;
    }
    propellerTorque = targetTorque;
    // Normalize the stored binary64 torque, after the x87 result has rounded.
    const volatile double& storedTorque = propellerTorque;
    turbine.normalizedShaftTorque = storedTorque / settings.maximumTorque;
}

} // namespace sim1
