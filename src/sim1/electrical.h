#ifndef SIM1_ELECTRICAL_H
#define SIM1_ELECTRICAL_H

namespace sim1 {

class FlightDynamics;
struct ElectricalOwner;

#pragma pack(push, 4)

// Minimum ABI projection of the owner dispatch used by the electrical query.
struct ElectricalOwnerDispatch {
    char unknown00[0x114];
    int (__thiscall* circuitMode)(ElectricalOwner*);
};

struct ElectricalOwner {
    const ElectricalOwnerDispatch* dispatch;
};

struct ElectricalSupplyConfiguration {
    char unknown00[8];
    double maximumBatteryVoltage;
};

struct ElectricalCircuitConfiguration {
    char unknown00[8];
    double minimumVoltage;
    unsigned busIndex;
};

struct ElectricalCircuit {
    int inhibit;
    const ElectricalCircuitConfiguration* configuration;
    char unknown08[4];
};

struct ElectricalBus {
    double voltage;
    char unknown08[8];
};

// Minimum runtime layout for the availability query and its propeller callers.
struct ElectricalRuntime {
    const double* FindBusVoltage(unsigned busIndex) const;
    int CircuitAvailability(const FlightDynamics& airplane, unsigned circuitIndex) const;

    const ElectricalSupplyConfiguration* supply;
    ElectricalBus buses[8];
    char unknown084[0x2c];
    ElectricalCircuit circuits[33];
    char unknown23c[8];
    int fallbackAvailability;
};

#pragma pack(pop)

extern int electricAlwaysAvailable;

} // namespace sim1

#endif
