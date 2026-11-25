#include "turbine.h"
#include "electrical.h"

#include <math.h>

namespace sim1 {

typedef char CheckAutoStartEnabled[offsetof(IntegrationHistory, autoStartActive) == 0x484 ? 1 : -1];
typedef char CheckAutoStartEngine[offsetof(IntegrationHistory, autoStartEngine) == 0x4b0 ? 1 : -1];
typedef char CheckAutoStartCrankTime[offsetof(IntegrationHistory, autoStartCrankTime) == 0x4c4 ? 1 : -1];
typedef char CheckAutoStartArm[offsetof(IntegrationHistory, autoStartArm) == 0x824 ? 1 : -1];
typedef char CheckElectricalOverride[offsetof(ElectricalRuntime, fallbackAvailability) == 0x244 ? 1 : -1];

// Original: sim1.dll 0x20c214b0. The original also receives the starter record in EBX.
void UpdateEngineStarterPower(const FlightDynamics& airplane, int engineNumber)
{
    IntegrationHistory& history = *airplane.history;
    const PropulsionConfiguration* settings = history.propulsionConfiguration;
    if (!settings || engineNumber < 1 || static_cast<unsigned>(engineNumber) > settings->engineCount ||
        !history.engineForces) {
        return;
    }
    EngineForceRecord& engine = history.engineForces[engineNumber - 1];
    if (engine.starterDrive <= 0.0f) {
        engine.starterEngaged = 0;
    } else if (!settings->bypassStarterCircuit && engineNumber <= 4) {
        engine.starterEngaged = history.electrical->CircuitAvailability(airplane, 17 + engineNumber);
    } else {
        engine.starterEngaged = 1;
    }
}

// Original: sim1.dll 0x20c2a130.
__declspec(noinline) static double __stdcall AdvanceEngineControl(double current, double target, double rate, double step)
{
    if (current < target) {
        const double next = current + rate * step;
        return next < target ? next : target;
    }
    if (current > target) {
        const double next = current - rate * step;
        return next > target ? next : target;
    }
    return current;
}

// Original: sim1.dll 0x20c2d410. The shared start event and effects dispatch use the owning adapter.
void UpdateTurbineAutoStart(FlightDynamics& airplane, EngineForceRecord& engine, TurbineEngineState& turbine,
                            unsigned engineIndex, double step, TurbineStartActions& actions)
{
    IntegrationHistory& history = *airplane.history;
    history.electrical->fallbackAvailability = 1;
    if (engineIndex != history.autoStartEngine) {
        return;
    }
    if (engine.running) {
        if (history.autoStartRunningTime < 5.0) {
            history.autoStartRunningTime += step;
            return;
        }
        engine.starterDrive = 0.0f;
        history.autoStartLeverReady = 0;
        history.autoStartRunningTime = 0.0;
        history.autoStartCrankTime = 0.0;
        history.autoStartLeverTime = 0.0;
        engine.autoStartCompleted = 1;
        actions.CompleteEngineStart(airplane, 0x1048d, 1, 0);
        ++history.autoStartEngine;
        if (history.autoStartEngine >= history.propulsionConfiguration->engineCount) {
            history.autoStartEngine = 0;
            ToggleAutomaticEngineStart(airplane, 1, actions);
        }
        return;
    }

    engine.fuelControlPosition = 0.0;
    if (!history.autoStartLeverReady) {
        const double command = AdvanceEngineControl(engine.governorCommand, 1.0, 1.0, step);
        engine.governorCommand = command;
        if (fabs(command - 1.0) < 0.001 || history.autoStartLeverTime > 3.0) {
            history.autoStartLeverReady = 1;
            history.autoStartLeverTime = 0.0;
        } else {
            history.autoStartLeverTime += step;
        }
    } else if (!history.autoStartArm) {
        history.autoStartArm = 1;
    } else if (!turbine.bypassIgnitionInhibit) {
        turbine.bypassIgnitionInhibit = 1;
    } else if (engine.fuelSelector != 1) {
        engine.fuelSelector = 1;
    } else if (!engine.ignitionEnabled) {
        engine.ignitionEnabled = 1;
    }
    if (!turbine.autoStartReady) {
        turbine.autoStartReady = 1;
    } else {
        engine.fuelSelector = 1;
        engine.governorCommand = 1.0;
        engine.starterDrive = 1.5f;
        history.autoStartCrankTime += step;
    }
    history.autoStartRunningTime = 0.0;
    if (history.autoStartCrankTime > 20.0) {
        engine.fuelControlPosition = 1.0;
    }
    if (history.autoStartCrankTime > 25.0) {
        engine.starterDrive = 0.0f;
        history.autoStartCrankTime = 0.0;
        const unsigned result = !engine.ignitionRequested ? 3 : (history.propellerStopRequested ? 4 : 5);
        ToggleAutomaticEngineStart(airplane, result, actions);
    }
}

// Reconstruction-only shared query for the original automatic start gate checks.
static __forceinline bool AutomaticStartAllowed(const FlightDynamics& airplane)
{
    return airplane.electricalOwnerBypass ||
           airplane.electricalOwner->dispatch->circuitMode(airplane.electricalOwner) == 1;
}

// Reconstruction-only common count expression from both loops in the original starter toggle.
static __forceinline unsigned CountRunningEngines(const IntegrationHistory& history)
{
    unsigned running = 0;
    for (unsigned index = 0; index < history.propulsionConfiguration->engineCount; ++index) {
        if (history.engineForces[index].running) {
            ++running;
        }
    }
    return running;
}

// Original: sim1.dll 0x20c211c0. The shared message renderer remains an adapter call.
void ToggleAutomaticEngineStart(FlightDynamics& airplane, unsigned result, TurbineStartActions& actions)
{
    IntegrationHistory* history = airplane.history;
    if (!history || !history->propulsionConfiguration || !AutomaticStartAllowed(airplane)) {
        return;
    }
    unsigned messageId = 0;
    if (!history->autoStartActive) {
        history->autoStartInitialRunningCount = CountRunningEngines(*history);
        history->autoStartActive = 1;
        messageId = 0x9612;
    } else {
        history->autoStartActive = 0;
        if (result == 1) {
            const unsigned count = CountRunningEngines(*history);
            if (count > history->autoStartInitialRunningCount) {
                messageId = 0x9613;
            } else if (count == history->autoStartInitialRunningCount) {
                messageId = 0x9615;
            }
        } else if (result == 2) {
            messageId = 0x9614;
        } else if (result >= 3 && result <= 5) {
            messageId = 0x9613 + result;
        }
    }
    if (messageId && AutomaticStartAllowed(airplane)) {
        actions.ShowAutoStartMessage(airplane, messageId, 2.0);
    }
}

} // namespace sim1
