#include "turbine.h"

#include <math.h>

namespace sim1 {

// Original shared turbine governor target at sim1.dll 0x20c7ba68.
double turbineGovernorTarget;

static const AerodynamicCurveStorage<4> reverserPositionSchedule = {
    {4}, {{0.0, 0.25}, {0.5, 0.25}, {1.0, 0.11}, {1.15, 0.0}}
};

// Original: sim1.dll 0x20c2bc90.
double TurbineEngineState::CalculateThrottleInput(const IntegrationHistory& history, const EngineForceRecord& engine)
{
    const PropulsionConfiguration& settings = *history.propulsionConfiguration;
    const TurbinePerformanceData& performance = *settings.turbinePerformance;
    const double throttle = engine.reverseControlInput;
    volatile double scheduled = throttle;
    if (!(throttle >= 0.0)) {
        if (settings.engineType == 5) {
            scheduled = throttle * -1.5;
        } else if (settings.engineType == 3) {
            scheduled = throttle * 0.0;
        } else if (settings.engineType == 1) {
            scheduled = throttle * -1.6;
        }
    } else if (performance.afterburnerStages && performance.afterburnerThrottleThreshold > 0.0f) {
        const float threshold = performance.afterburnerThrottleThreshold;
        if (throttle < threshold) {
            scheduled = throttle / threshold;
            afterburnerStage = 0;
        } else {
            scheduled = 1.0;
            const double fraction = (throttle + 0.01 - threshold) / (1.0 - threshold);
            const double limited = fraction < 1.0 ? fraction : 1.0;
            afterburnerStage = engine.running
                ? static_cast<unsigned>(static_cast<__int64>(performance.afterburnerStages * limited)) : 0;
        }
    }

    return engine.auxiliaryControl->dispatch->level(engine.auxiliaryControl) * scheduled *
           (1.0 - engine.turbineEffect->throttlePenalty);
}

// Original: sim1.dll 0x20c2bdd0.
void TurbineEngineState::UpdateGovernorTarget(const IntegrationHistory& history, const EngineForceRecord& engine,
                                              double throttle) const
{
    const PropulsionConfiguration& settings = *history.propulsionConfiguration;
    const TurbinePerformanceData& performance = *settings.turbinePerformance;
    const double inversePressure = 1.0 / history.ambientPressureRatio;
    const volatile double low = performance.lowMachGovernorTable.Evaluate(inversePressure, throttle);
    const double high = performance.highMachGovernorTable.Evaluate(inversePressure, throttle);
    const double lowMach = performance.lowMachGovernorTable.values[0];
    const double highMach = performance.highMachGovernorTable.values[0];
    const double target = low + ((inletMach - lowMach) / (highMach - lowMach)) * (high - low);
    turbineGovernorTarget = target;
    if (settings.engineType == 3 || settings.engineType == 5) {
        const double idleFloor = 62.0 + engine.fuelControlPosition * 8.0;
        if (!(target > idleFloor)) {
            turbineGovernorTarget = idleFloor;
        }
    }
}

// Original: sim1.dll 0x20c2beb0.
void TurbineEngineState::UpdateFuelSchedule(const IntegrationHistory& history, const EngineForceRecord& engine,
                                           double step)
{
    const PropulsionConfiguration& settings = *history.propulsionConfiguration;
    const unsigned type = settings.engineType;
    if (engine.fuelControlPosition <= 0.009999999776482582 || (type == 1 && shaftSpeed == 0.0) ||
        ((type == 3 || type == 5) && turbineSpeed == 0.0)) {
        spoolScheduleInput = 0.0;
        return;
    }

    const TurbinePerformanceData& performance = *settings.turbinePerformance;
    double correctedSpeed = 0.0;
    double response = 0.0;
    if (type == 1 || type == 3 || type == 5) {
        correctedSpeed = type == 1 ? correctedHighSpoolSpeed : turbineTorqueCurveInput;
        response = performance.fuelFlowGain;
    }
    const double thrustScale = performance.staticThrust > 0.0 ? performance.staticThrust : 100.0;
    const double gain = thrustScale * response * step;
    const double requested = spoolScheduleInput + (gain < 1.0 ? gain : 1.0) *
                              (turbineGovernorTarget - correctedSpeed);
    spoolScheduleInput = requested > 1e-5 ? requested : 1e-5;
    if (fabs(engine.oilPressure) < 0.0001) {
        spoolScheduleInput = 0.0;
    }
    if (!engine.running && type == 1) {
        const double limit = performance.staticThrust * 0.4;
        if (!(spoolScheduleInput < limit)) {
            spoolScheduleInput = limit;
        }
    }
}

// Original: sim1.dll 0x20c2bff0.
bool TurbineEngineState::CanSustainCombustion(unsigned engineType) const
{
    if (engineType == 1) {
        return shaftSpeed > 20.0;
    }
    return (engineType == 3 || engineType == 5) && turbineSpeed > 10.0;
}

// Original: sim1.dll 0x20c205d0. The shared effect creation is provided by the owning adapter.
__declspec(noinline) static void InitiateTurbineCombustion(FlightDynamics& airplane, EngineForceRecord& engine,
                                                          TurbineStartActions& actions)
{
    engine.running = 1;
    const Vector3& mount = engine.mount->position;
    const Vector3 positionMeters = {mount.x * 0.3048, mount.y * 0.3048, mount.z * 0.3048};
    actions.EmitIgnitionEffect(airplane, positionMeters);
    engine.ignitionElapsed = 0.0;
}

// Original: sim1.dll 0x20c2c040.
void TurbineEngineState::UpdateCombustion(FlightDynamics& airplane, EngineForceRecord& engine,
                                         TurbineStartActions& actions)
{
    const bool disabled = engine.auxiliaryControl &&
                          engine.auxiliaryControl->dispatch->level(engine.auxiliaryControl) == 0.0;
    if (!(spoolScheduleInput > 0.0) || !engine.ignitionRequested || disabled ||
        (!bypassIgnitionInhibit && ignitionInhibit &&
         ignitionInhibit->dispatch->active(ignitionInhibit))) {
        engine.running = 0;
        return;
    }
    const unsigned engineType = airplane.history->propulsionConfiguration->engineType;
    if (engine.running) {
        if (!CanSustainCombustion(engineType)) {
            engine.running = 0;
        }
    } else if (engine.starterEngaged && CanSustainCombustion(engineType) &&
               !(fabs(engine.spoolDriveSignal) < 0.0001)) {
        InitiateTurbineCombustion(airplane, engine, actions);
    }
}

// Original: sim1.dll 0x20c2c410.
void TurbineEngineState::UpdateReverserPosition(const IntegrationHistory& history, const EngineForceRecord& engine,
                                              double step)
{
    const PropulsionConfiguration& settings = *history.propulsionConfiguration;
    const TurbinePerformanceData& performance = *settings.turbinePerformance;
    if (!(performance.reverserEffectiveness > 0.0f)) {
        reverserPosition = 0.0;
        return;
    }

    double target = 0.0;
    if (engine.reverseControlInput < 0.0) {
        target = engine.reverseControlInput / settings.reverseControlScale;
    } else if (performance.useTabulatedReverser) {
        target = SampleAerodynamicCurve(&reverserPositionSchedule.curve, history.mach);
    }
    const double change = target - reverserPosition;
    const float response = change > 0.0 ? 8.0f : 2.0f;
    reverserPosition += change * response * step;
}

// Original: sim1.dll 0x20c2c350.
void TurbineEngineState::UpdateBleedFactor(const IntegrationHistory& history, const EngineForceRecord& engine,
                                           double step)
{
    const TurbinePerformanceData& performance = *history.propulsionConfiguration->turbinePerformance;
    if (!performance.bleedData) {
        bleedFactor = 1.0;
        return;
    }

    double target = 0.0;
    if (!(engine.reverseControlInput < 0.0 && engine.running)) {
        const volatile double base = performance.bleedData->schedule.Evaluate(history.mach, turbineTorqueCurveInput);
        target = base + AfterburnerFraction(history) * static_cast<double>(performance.bleedData->afterburnerGain);
        target = target < 1.0 ? target : 1.0;
    }
    bleedFactor += (target - bleedFactor) * 4.0 * step;
}

// Original: sim1.dll 0x20c2c120.
void TurbineEngineState::UpdateJetHighSpool(const IntegrationHistory& history, EngineForceRecord& engine, double step)
{
    const TurbinePerformanceData& performance = *history.propulsionConfiguration->turbinePerformance;
    double target;
    double response;
    if (!engine.running) {
        target = engine.starterEngaged ? 25.0 : 0.0;
        response = 0.1;
    } else {
        target = SampleAerodynamicCurve(&performance.jetHighSpoolCurve.curve, spoolScheduleInput / performance.staticThrust);
        const double gain = (target - 70.0) * 7.4 / 20.0 + 0.6;
        response = gain > 0.6 ? (gain < 8.0 ? gain : 8.0) : 0.6;
        if (fabs(engine.spoolDriveSignal) < 0.0001) {
            target = 0.0;
        }
    }
    correctedHighSpoolSpeed += (target - correctedHighSpoolSpeed) * response * step;
    const volatile double& storedSpeed = correctedHighSpoolSpeed;
    const double actualSpeed = sqrt(turbineInletTemperatureRatio) * storedSpeed;
    shaftSpeed = actualSpeed;
    const double highSpoolFraction = actualSpeed / 100.0;
    engine.speedFraction = highSpoolFraction;
    engine.propellerDerivedRpm = highSpoolFraction * performance.ratedHighSpoolRpm;
}

// Original: sim1.dll 0x20c2c230.
void TurbineEngineState::UpdateJetLowSpool(const IntegrationHistory& history, double step)
{
    const TurbinePerformanceData& performance = *history.propulsionConfiguration->turbinePerformance;
    const double difference = performance.jetLowSpoolTable.Evaluate(inletMach, correctedHighSpoolSpeed) -
                              turbineTorqueCurveInput;
    const double rate = difference * (difference > 0.0 ? 6.0 : 12.0);
    const double limitedRate = rate > -33.0 ? (rate < 33.0 ? rate : 33.0) : -33.0;
    turbineTorqueCurveInput += limitedRate * step;
    const volatile double& storedSpeed = turbineTorqueCurveInput;
    turbineSpeed = sqrt(turbineInletTemperatureRatio) * storedSpeed;
}

} // namespace sim1
