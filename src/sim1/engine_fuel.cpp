#include "turbine.h"

namespace sim1 {

typedef char CheckFuelControl[offsetof(EngineForceRecord, fuelControlPosition) == 0x14 ? 1 : -1];
typedef char CheckFuelSelector[offsetof(EngineForceRecord, fuelSelector) == 0xb4 ? 1 : -1];
typedef char CheckFuelFlow[offsetof(EngineForceRecord, fuelFlow) == 0xc0 ? 1 : -1];
typedef char CheckFuelTankStart[offsetof(IntegrationHistory, internalFuelTanks) == 0x2cc ? 1 : -1];
typedef char CheckFuelTankFlags[offsetof(ExternalFuelTankState, selectionMask) == 0x10 ? 1 : -1];

// Original: sim1.dll 0x20bf8170. The turbine-specific fuel demand uses the previous step's outputs.
__declspec(noinline) static void CalculateTurbineFuelFlow(const IntegrationHistory& history, unsigned engineIndex)
{
    const PropulsionConfiguration& settings = *history.propulsionConfiguration;
    const TurbinePerformanceData* performance = settings.turbinePerformance;
    if (!history.engineForces || !history.turbineEngines || !performance) {
        return;
    }
    EngineForceRecord& engine = history.engineForces[engineIndex];
    const TurbineEngineState& turbine = history.turbineEngines[engineIndex];
    double effectiveThrust = turbine.netThrust;
    if (engine.running) {
        const double idleThrust = performance->staticThrust * 0.065;
        if (!(effectiveThrust > idleThrust)) {
            effectiveThrust = idleThrust;
        }
    }
    const float fraction = turbine.AfterburnerFraction(history);
    engine.fuelFlow = performance->afterburnerStages && fraction > 0.0f
        ? performance->afterburnerSpecificFuelConsumption * effectiveThrust * fraction
        : performance->thrustSpecificFuelConsumption * effectiveThrust;

    if (settings.engineType == 3 || settings.engineType == 5) {
        double torque = engine.propellerTorque;
        if (engine.running) {
            const double idleTorque = settings.maximumTorque * 0.1;
            if (!(torque > idleTorque)) {
                torque = idleTorque;
            }
        }
        double shaftPower = 0.0;
        if (settings.maximumTorque > 0.0 && performance->ratedShaftHorsepower > 0.0f) {
            shaftPower = (performance->ratedShaftHorsepower * engine.speedFraction) * (torque / settings.maximumTorque);
        } else if (settings.engineType == 5) {
            shaftPower = (engine.propellerDerivedRpm * 0.1047197551 / settings.gearReductionRatio) * torque / 550.0;
        }
        engine.fuelFlow += shaftPower * settings.shaftSpecificFuelConsumption;
    }
    const double demand = engine.fuelFlow > 0.0 ? engine.fuelFlow : 0.0;
    engine.fuelFlow = demand * settings.fuelFlowScalar;
}

// Reconstruction-only shared steps in the selector's repeated tank branches.
static __forceinline bool SelectFuelTank(EngineForceRecord& engine, const ExternalFuelTankState& tank)
{
    if (!(tank.amount > tank.minimumAmount)) {
        return false;
    }
    engine.selectedFuelMask |= tank.selectionMask;
    ++engine.selectedFuelCount;
    engine.ignitionRequested = 1;
    return true;
}

static __forceinline void SelectFuelBank(EngineForceRecord& engine, const ExternalFuelTankState* tanks, unsigned side)
{
    if (!SelectFuelTank(engine, tanks[4 + side]) && !SelectFuelTank(engine, tanks[2 + side])) {
        SelectFuelTank(engine, tanks[side]);
    }
}

// Original: sim1.dll 0x20bf79e0. The original receiver points to engine +0xb4.
__declspec(noinline) static void SelectEngineFuelTanks(const IntegrationHistory& history, EngineForceRecord& engine)
{
    const ExternalFuelTankState* tanks = history.internalFuelTanks;
    engine.selectedFuelMask = 0;
    engine.selectedFuelCount = 0;
    switch (engine.fuelSelector) {
    case 0:
        engine.ignitionRequested = 0;
        return;
    case 2:
    case 3:
        SelectFuelBank(engine, tanks, engine.fuelSelector - 2);
        return;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8: {
        static const unsigned index[] = {2, 3, 6, 7, 8};
        SelectFuelTank(engine, tanks[index[engine.fuelSelector - 4]]);
        return;
    }
    case 9:
        SelectFuelTank(engine, history.firstExternalTank);
        return;
    case 10:
        SelectFuelTank(engine, history.secondExternalTank);
        return;
    case 11:
        SelectFuelTank(engine, tanks[5]);
        return;
    case 12:
        SelectFuelTank(engine, tanks[4]);
        return;
    case 0x10:
        SelectFuelBank(engine, tanks, 0);
        SelectFuelBank(engine, tanks, 1);
        return;
    case 0x11:
        SelectFuelTank(engine, history.firstExternalTank);
        SelectFuelTank(engine, history.secondExternalTank);
        return;
    case 0x12:
        if (history.fuelCrossfeedMode != 1) {
            engine.ignitionRequested = 0;
        } else if (engine.mount->position.x > 0.0) {
            engine.selectedFuelMask = tanks[0].selectionMask;
            engine.selectedFuelCount = 1;
            engine.ignitionRequested = 1;
        } else if (engine.mount->position.x == 0.0) {
            engine.selectedFuelMask = tanks[1].selectionMask;
            engine.selectedFuelCount = 1;
            engine.ignitionRequested = 1;
        }
        return;
    case 0x13:
    case 0x14:
        SelectFuelTank(engine, tanks[engine.fuelSelector - 0x13]);
        return;
    default:
        SelectFuelTank(engine, history.firstExternalTank);
        SelectFuelTank(engine, history.secondExternalTank);
        if (engine.selectedFuelCount) {
            return;
        }
        if (!SelectFuelTank(engine, tanks[8]) && !SelectFuelTank(engine, tanks[7])) {
            SelectFuelTank(engine, tanks[6]);
        }
        if (history.fuelCrossfeedMode != 3 || engine.mount->position.x <= 0.0) {
            SelectFuelBank(engine, tanks, 0);
        }
        if (history.fuelCrossfeedMode != 2 || engine.mount->position.x >= 0.0) {
            SelectFuelBank(engine, tanks, 1);
        }
        return;
    }
}

// Original: sim1.dll 0x20bf77c0, scoped to the conventional turbine branch and common selector handoff.
void UpdateTurbineFuelDelivery(const FlightDynamics& airplane, unsigned engineIndex)
{
    const IntegrationHistory& history = *airplane.history;
    const PropulsionConfiguration& settings = *history.propulsionConfiguration;
    EngineForceRecord& engine = history.engineForces[engineIndex];
    if (settings.engineType == 1 || settings.engineType == 5 ||
        (settings.engineType == 3 && !settings.rotorConfiguration)) {
        CalculateTurbineFuelFlow(history, engineIndex);
    }
    engine.ignitionRequested = airplane.propulsionMomentControl->defaultFuelAvailability != 0;
    SelectEngineFuelTanks(history, engine);
    if (!engine.ignitionEnabled) {
        engine.ignitionRequested = 0;
        engine.selectedFuelMask = 0;
        engine.selectedFuelCount = 0;
    }
}

} // namespace sim1
