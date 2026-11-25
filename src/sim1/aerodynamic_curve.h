#ifndef SIM1_AERODYNAMIC_CURVE_H
#define SIM1_AERODYNAMIC_CURVE_H

#include "flight_dynamics.h"

namespace sim1 {

#pragma pack(push, 4)

struct AerodynamicSample {
    double angle;
    double coefficient;
};

template <unsigned Count>
struct AerodynamicCurveStorage {
    AerodynamicCurve curve;
    AerodynamicSample samples[Count];
};

#pragma pack(pop)

double __fastcall SampleAerodynamicCurve(const AerodynamicCurve* curve, double angle);

} // namespace sim1

#endif
