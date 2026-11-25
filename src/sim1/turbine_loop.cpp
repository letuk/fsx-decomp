#include "turbine.h"

#include <math.h>

namespace sim1 {

// Original: sim1.dll 0x20c2b6d0. Shared start and visual effects dispatch pass through the owning adapter.
void FlightDynamics::UpdateTurbineEngines(double step, TurbineStartActions& actions)
{
    IntegrationHistory& state = *history;
    if (!state.propulsionConfiguration->turbinePerformance) {
        return;
    }
    for (unsigned index = 0; index < state.propulsionConfiguration->engineCount; ++index) {
        EngineForceRecord& engine = state.engineForces[index];
        TurbineEngineState& turbine = state.turbineEngines[index];
        UpdateEngineStarterPower(*this, index + 1);
        UpdateTurbineFuelDelivery(*this, index);
        if (state.autoStartActive) {
            UpdateTurbineAutoStart(*this, engine, turbine, index, step, actions);
        }
        turbine.UpdateInletConditions(state);
        const double throttle = turbine.CalculateThrottleInput(state, engine);
        turbine.UpdateGovernorTarget(state, engine, throttle);
        turbine.UpdateFuelSchedule(state, engine, step);
        turbine.UpdateCombustion(*this, engine, actions);
        turbine.UpdateReverserPosition(state, engine, step);
        turbine.UpdateBleedFactor(state, engine, step);
        const unsigned type = state.propulsionConfiguration->engineType;
        if (type == 1) {
            turbine.UpdateJetHighSpool(state, engine, step);
            turbine.UpdateJetLowSpool(state, step);
        } else if (type == 3 || type == 5) {
            turbine.UpdateSpoolSpeed(engine, state.airRelativeVelocity.x, step, turbineInletTemperatureRatio);
            const volatile double speedPercent = engine.speedFraction * 100.0;
            turbine.shaftSpeed = speedPercent;
            turbine.correctedHighSpoolSpeed = speedPercent / sqrt(turbineInletTemperatureRatio);
            engine.UpdateTurbineShaftTorque(*this, turbine, step);
        }
        turbine.UpdateGrossThrust(state);
        turbine.UpdateRamDrag(state);
        turbine.PublishNetThrust(state, engine);
        engine.UpdateTurbineSpeedOutput(&turbine);
        turbine.UpdatePressureRatio(state, step);
        turbine.UpdateTurbineTemperature(*this, engine, step);
        UpdateTurbineExhaustTemperature(*this, engine, turbine, step);
        UpdateTurbineDriveSignal(state, engine, turbine, step);
        UpdateTurbineOilTemperature(*this, engine, turbine, step);
        UpdateTurbineOilPressure(*this, index, static_cast<float>(step));
        turbine.UpdateSpeedBlend(step);
        actions.UpdateEngineEffects(*this, engine, step);
    }
}

} // namespace sim1
