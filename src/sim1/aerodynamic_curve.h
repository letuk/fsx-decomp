#ifndef SIM1_AERODYNAMIC_CURVE_H
#define SIM1_AERODYNAMIC_CURVE_H

#include "flight_dynamics.h"

namespace sim1 {

double __fastcall SampleAerodynamicCurve(const AerodynamicCurve* curve, double angle);

} // namespace sim1

#endif
