#include "turbine.h"

namespace sim1 {

static const TurbineCoefficientTable<20> pressureRatioSchedule = {
    5, 4, {
        0.0,   0.0,   0.45,  0.9,
        0.0,   0.0,   0.0,   0.0,
        56.0,  0.751, 0.751, 1.86,
        90.0,  0.957, 0.957, 3.28,
        110.0, 1.0,   1.0,   4.23
    }
};
static const AerodynamicCurveStorage<5> turbineTemperatureSchedule = {
    {5}, {{40.0, 0.86}, {60.0, 0.69}, {80.0, 0.743}, {100.0, 0.835}, {110.0, 1.0}}
};
static const AerodynamicCurveStorage<5> exhaustTemperatureSchedule = {
    {5}, {{45.0, 1.0}, {56.0, 0.66}, {80.0, 0.88}, {100.0, 0.979}, {110.0, 1.0}}
};
static const AerodynamicCurveStorage<4> driveSchedule = {
    {4}, {{0.0, 0.0}, {21.0, 0.88}, {90.0, 0.954}, {100.0, 1.0}}
};
static const AerodynamicCurveStorage<4> oilTemperatureSchedule = {
    {4}, {{21.0, 0.98}, {31.0, 0.946}, {42.0, 0.935}, {90.0, 0.905}}
};

// Reconstruction-only shared expression for the binary32 ambient input to the thermal updates.
static __forceinline float AmbientRankine(const FlightEnvironment& environment)
{
    return static_cast<float>(environment.ambientTemperatureCelsius * 1.8 + 32.0 + 459.7);
}

// Reconstruction-only selection used by the two gas-temperature schedules.
static __forceinline double IndicationSpoolSpeed(const TurbineEngineState& turbine, unsigned engineType)
{
    if (engineType == 1) {
        return turbine.correctedHighSpoolSpeed;
    }
    return engineType == 3 || engineType == 5 ? turbine.turbineTorqueCurveInput : 0.0;
}

// Original: sim1.dll 0x20c2c980. The turbine record originally arrives in EDI.
void TurbineEngineState::UpdatePressureRatio(const IntegrationHistory& history, double step)
{
    const PropulsionConfiguration& settings = *history.propulsionConfiguration;
    double target = pressureRatioSchedule.Evaluate(inletMach, correctedHighSpoolSpeed) *
                    settings.pressureRatioReference * history.ambientPressureRatio;
    if (inletMach < 0.05 && !(target > 1.0)) {
        target = 1.0;
    }
    pressureRatio += (target - pressureRatio) * settings.pressureRatioResponse * step;
}

// Original: sim1.dll 0x20c2caa0.
void TurbineEngineState::UpdateTurbineTemperature(const FlightDynamics& airplane, const EngineForceRecord& engine,
                                                double step)
{
    const PropulsionConfiguration& settings = *airplane.history->propulsionConfiguration;
    const float ambient = AmbientRankine(*airplane.environment);
    double target = ambient;
    double response = 0.11;
    if (engine.running) {
        target = SampleAerodynamicCurve(&turbineTemperatureSchedule.curve, IndicationSpoolSpeed(*this, settings.engineType)) *
                 settings.turbineTemperatureReference * settings.turbineTemperatureScale;
        response = settings.turbineTemperatureResponse;
    }
    if (!(target > ambient)) {
        target = ambient;
    }
    turbineTemperature += (turbineInletTemperatureRatio * target - turbineTemperature) * response * step;
}

// Original: sim1.dll 0x20c2ce50.
void UpdateTurbineExhaustTemperature(const FlightDynamics& airplane, EngineForceRecord& engine,
                                     const TurbineEngineState& turbine, double step)
{
    const PropulsionConfiguration& settings = *airplane.history->propulsionConfiguration;
    const float ambient = AmbientRankine(*airplane.environment);
    double target = ambient;
    if (engine.running) {
        target = SampleAerodynamicCurve(&exhaustTemperatureSchedule.curve, IndicationSpoolSpeed(turbine, settings.engineType)) *
                 settings.exhaustTemperatureReference * settings.exhaustTemperatureScale;
    }
    const double updated = engine.exhaustTemperature + (turbineInletTemperatureRatio * target - engine.exhaustTemperature) *
                           settings.exhaustTemperatureResponse * step;
    engine.exhaustTemperature = updated > ambient ? updated : ambient;
}

// Original: sim1.dll 0x20c2cbe0.
void UpdateTurbineDriveSignal(const IntegrationHistory& history, EngineForceRecord& engine,
                              const TurbineEngineState& turbine, double step)
{
    const PropulsionConfiguration& settings = *history.propulsionConfiguration;
    volatile double nominal = SampleAerodynamicCurve(&driveSchedule.curve, turbine.turbineSpeed) *
                              settings.spoolDriveReference * settings.spoolDriveScale;
    if (engine.propellerDerivedRpm > settings.minimumGovernorRpm && settings.propellerType == 0 &&
        settings.engineType == 5) {
        const double bias = (1.0 - engine.governorCommand) * 864.0;
        nominal += settings.highRpmDriveBias ? bias : -bias;
    }
    double target;
    if (!engine.driveSupply || !engine.driveWear) {
        engine.driveAvailability = 1.0f;
        target = engine.spoolDriveSignal;
    } else if (!(engine.driveWear->dispatch->level(engine.driveWear) < 1.0)) {
        engine.driveAvailability = 1.0f;
        target = engine.driveSupply->dispatch->level(engine.driveSupply) * nominal;
    } else {
        volatile double lostFraction = 1.0 - engine.driveWear->dispatch->level(engine.driveWear);
        const float loss = static_cast<float>(engine.driveWear->dispatch->responseRate(engine.driveWear) *
                                              lostFraction * step);
        const float remaining = engine.driveAvailability - loss;
        engine.driveAvailability = remaining > 0.0f ? remaining : 0.0f;
        target = nominal;
    }
    engine.spoolDriveSignal += (target * engine.driveAvailability - engine.spoolDriveSignal) *
                               settings.spoolDriveResponse * step;
}

// Original: sim1.dll 0x20c2cd80.
void UpdateTurbineOilTemperature(const FlightDynamics& airplane, EngineForceRecord& engine,
                                 const TurbineEngineState& turbine, double step)
{
    const PropulsionConfiguration& settings = *airplane.history->propulsionConfiguration;
    double target = AmbientRankine(*airplane.environment);
    if (engine.running) {
        target = SampleAerodynamicCurve(&oilTemperatureSchedule.curve, turbine.turbineSpeed) *
                 settings.oilTemperatureReference * settings.oilTemperatureScale;
    }
    engine.oilTemperature += (target - engine.oilTemperature) * settings.oilTemperatureResponse * step;
}

// Original: sim1.dll 0x20c2cf80.
void UpdateTurbineOilPressure(const FlightDynamics& airplane, unsigned engineIndex, float step)
{
    const IntegrationHistory& history = *airplane.history;
    const PropulsionConfiguration& settings = *history.propulsionConfiguration;
    EngineForceRecord& engine = history.engineForces[engineIndex];
    double target = 0.0;
    if (!(settings.engineConditionFlags & 0x10) || !engine.oilPressureInhibit ||
        !engine.oilPressureInhibit->dispatch->active(engine.oilPressureInhibit)) {
        const double speed = engine.starterEngaged && !(engine.speedFraction > 0.1) ? 0.1 : engine.speedFraction;
        target = speed * 4.0;
    }
    if (!(target > engine.minimumOilPressureFraction)) {
        target = engine.minimumOilPressureFraction;
    }
    if (!(target < 1.0)) {
        target = 1.0;
    }
    engine.oilPressure += (target * settings.oilPressureReference - engine.oilPressure) *
                          settings.oilPressureResponse * step;
}

} // namespace sim1
