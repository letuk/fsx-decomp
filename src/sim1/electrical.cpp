#include "electrical.h"
#include "flight_dynamics.h"

namespace sim1 {

typedef char CheckElectricalRuntime[offsetof(IntegrationHistory, electrical) == 0x534 ? 1 : -1];
typedef char CheckElectricalOwner[offsetof(FlightDynamics, electricalOwner) == 0x64 ? 1 : -1];
typedef char CheckElectricalOwnerBypass[offsetof(FlightDynamics, electricalOwnerBypass) == 0xb4 ? 1 : -1];
typedef char CheckElectricalOwnerSlot[offsetof(ElectricalOwnerDispatch, circuitMode) == 0x114 ? 1 : -1];
typedef char CheckElectricalBuses[offsetof(ElectricalRuntime, buses) == 0x4 && sizeof(ElectricalBus) == 0x10 ? 1 : -1];
typedef char CheckElectricalCircuits[offsetof(ElectricalRuntime, circuits) == 0xb0 && sizeof(ElectricalCircuit) == 0xc ? 1 : -1];
typedef char CheckElectricalFallback[offsetof(ElectricalRuntime, fallbackAvailability) == 0x244 ? 1 : -1];
typedef char CheckElectricalCircuitBus[offsetof(ElectricalCircuitConfiguration, busIndex) == 0x10 ? 1 : -1];

// Original: sim1.dll 0x20c7c008. The configuration reader writes this option.
int electricAlwaysAvailable;

// Original: sim1.dll 0x20bfef40.
__declspec(noinline) const double* ElectricalRuntime::FindBusVoltage(unsigned busIndex) const
{
    if (busIndex < sizeof(buses) / sizeof(buses[0])) {
        return &buses[busIndex].voltage;
    }
    return 0;
}

// Original: sim1.dll 0x20bfee30.
__declspec(noinline) int ElectricalRuntime::CircuitAvailability(const FlightDynamics& airplane,
                                                               unsigned circuitIndex) const
{
    if (airplane.electricalOwnerBypass == 0) {
        ElectricalOwner* owner = airplane.electricalOwner;
        if (owner->dispatch->circuitMode(owner) != 1) {
            return fallbackAvailability;
        }
    }

    const ElectricalCircuit& circuit = circuits[circuitIndex];
    const ElectricalCircuitConfiguration* configuration = circuit.configuration;
    if (configuration == 0) {
        return 0;
    }
    const double* voltage = FindBusVoltage(configuration->busIndex);
    if (voltage == 0) {
        return 1;
    }
    return (configuration->minimumVoltage < *voltage && circuit.inhibit == 0) ||
        supply->maximumBatteryVoltage == 0.0 || electricAlwaysAvailable != 0;
}

} // namespace sim1
