#ifndef SIM1_TURBINE_H
#define SIM1_TURBINE_H

#include "aerodynamic_curve.h"

namespace sim1 {

extern double turbineInletTemperatureRoot;
extern double turbineInletPressureRatio;
extern double turbineInletTemperatureRatio;
extern double turbineGrossThrust;
extern double turbineRamDrag;
extern double turbineGovernorTarget;

#pragma pack(push, 4)

template <unsigned Capacity>
struct TurbineCoefficientTable {
    double Evaluate(double mach, double loading) const;

    unsigned rows;
    unsigned columns;
    double values[Capacity];
};

struct TurbineBleedData {
    float afterburnerGain;
    TurbineCoefficientTable<12> schedule;
};

// Minimum input view for the existing shared engine effects and start-command code.
class TurbineStartActions {
public:
    virtual void EmitIgnitionEffect(FlightDynamics& airplane, const Vector3& mountPositionMeters) = 0;
    virtual void CompleteEngineStart(FlightDynamics& airplane, unsigned eventId, int argument, int reserved) = 0;
    virtual void ShowAutoStartMessage(FlightDynamics& airplane, unsigned messageId, double duration) = 0;
    virtual void UpdateEngineEffects(FlightDynamics& airplane, EngineForceRecord& engine, double step) = 0;
};

// Reconstructed turbine state with the verified per-engine stride.
struct TurbineEngineState {
    int HasAutomaticFeatherTorque() const;
    void __fastcall UpdateInletConditions(const IntegrationHistory& history);
    void UpdateSpoolSpeed(const EngineForceRecord& engine, double axialSpeed, double step,
                          double inletTemperatureRatio);
    void UpdateGrossThrust(const IntegrationHistory& history) const;
    void UpdateRamDrag(const IntegrationHistory& history) const;
    float AfterburnerFraction(const IntegrationHistory& history) const;
    double ReverserThrustFactor(const IntegrationHistory& history) const;
    void PublishNetThrust(const IntegrationHistory& history, EngineForceRecord& engine);
    void UpdateSpeedBlend(double step);
    double CalculateThrottleInput(const IntegrationHistory& history, const EngineForceRecord& engine);
    void UpdateGovernorTarget(const IntegrationHistory& history, const EngineForceRecord& engine, double throttle) const;
    void UpdateFuelSchedule(const IntegrationHistory& history, const EngineForceRecord& engine, double step);
    bool CanSustainCombustion(unsigned engineType) const;
    void UpdateCombustion(FlightDynamics& airplane, EngineForceRecord& engine, TurbineStartActions& actions);
    void UpdateReverserPosition(const IntegrationHistory& history, const EngineForceRecord& engine, double step);
    void UpdateBleedFactor(const IntegrationHistory& history, const EngineForceRecord& engine, double step);
    void UpdateJetHighSpool(const IntegrationHistory& history, EngineForceRecord& engine, double step);
    void UpdateJetLowSpool(const IntegrationHistory& history, double step);
    void UpdatePressureRatio(const IntegrationHistory& history, double step);
    void UpdateTurbineTemperature(const FlightDynamics& airplane, const EngineForceRecord& engine, double step);

    double turbineSpeed;
    double shaftSpeed;
    double turbineTorqueCurveInput;
    double correctedHighSpoolSpeed;
    double spoolScheduleInput;
    double normalizedShaftTorque;
    double pressureRatio;
    double turbineTemperature;
    double inletMach;
    double netThrust;
    double speedScaledOutput;
    double bleedFactor;
    double reverserPosition;
    double filteredSpeedBlend;
    int bypassIgnitionInhibit;
    EngineConditionProvider* ignitionInhibit;
    int autoStartReady;
    unsigned afterburnerStage;
};

// Minimum layout of the AIR data allocation used by the turbine inlet and shaft.
struct TurbinePerformanceData {
    double staticThrust;
    double inletArea;
    double fuelFlowGain;
    double ratedHighSpoolRpm;
    float ratedShaftHorsepower;
    unsigned afterburnerStages;
    float afterburnerThrottleThreshold;
    char unknown2c[8];
    float thrustSpecificFuelConsumption;
    float afterburnerSpecificFuelConsumption;
    const TurbineBleedData* bleedData;
    int useHalfMachLimit;
    float reverserEffectiveness;
    int useTabulatedReverser;
    TurbineCoefficientTable<143> jetLowSpoolTable;
    TurbineCoefficientTable<90> lowMachGovernorTable;
    TurbineCoefficientTable<90> highMachGovernorTable;
    AerodynamicCurveStorage<10> jetHighSpoolCurve;
    TurbineCoefficientTable<231> grossThrustTable;
    TurbineCoefficientTable<231> inletFlowTable;
    AerodynamicCurveStorage<10> afterburnerMachCurve;
    AerodynamicCurveStorage<3> torqueByTurbineSpeed;
    AerodynamicCurveStorage<10> torqueByAirDensity;
};

// The other slots have unresolved signatures. Slot +0x48 uses a stack receiver.
struct EngineAuxiliaryDispatch {
    void* slots00To14[6];
    double (__stdcall* level)(EngineAuxiliaryControl*);
    void* slots1cTo44[11];
    void (__stdcall* setLevel)(EngineAuxiliaryControl*, float level, const void* context);
};

struct EngineAuxiliaryControl {
    const EngineAuxiliaryDispatch* dispatch;
};

// Reconstructed minimum projections of the provider slots used by turbine state updates.
struct EngineConditionDispatch {
    void* slots00To14[6];
    double (__stdcall* level)(EngineConditionProvider*);
    unsigned char (__stdcall* active)(EngineConditionProvider*);
    void* slots20To30[5];
    double (__stdcall* responseRate)(EngineConditionProvider*);
};

struct EngineConditionProvider {
    const EngineConditionDispatch* dispatch;
};

struct EngineTurbineEffectState {
    char unknown00[8];
    float throttlePenalty;
};

void UpdateTurbineExhaustTemperature(const FlightDynamics& airplane, EngineForceRecord& engine,
                                     const TurbineEngineState& turbine, double step);
void UpdateTurbineDriveSignal(const IntegrationHistory& history, EngineForceRecord& engine,
                              const TurbineEngineState& turbine, double step);
void UpdateTurbineOilTemperature(const FlightDynamics& airplane, EngineForceRecord& engine,
                                 const TurbineEngineState& turbine, double step);
void UpdateTurbineOilPressure(const FlightDynamics& airplane, unsigned engineIndex, float step);
void UpdateEngineStarterPower(const FlightDynamics& airplane, int engineNumber);
void ToggleAutomaticEngineStart(FlightDynamics& airplane, unsigned result, TurbineStartActions& actions);
void UpdateTurbineAutoStart(FlightDynamics& airplane, EngineForceRecord& engine, TurbineEngineState& turbine,
                            unsigned engineIndex, double step, TurbineStartActions& actions);
void UpdateTurbineFuelDelivery(const FlightDynamics& airplane, unsigned engineIndex);

#pragma pack(pop)

} // namespace sim1

#endif
