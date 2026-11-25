// Definition included inside namespace sim1 by the owning TU or its comparison context.
// Original: sim1.dll 0x20bcbf60. The original lookup receives the curve in ESI.
__declspec(noinline) double __fastcall SampleAerodynamicCurve(const AerodynamicCurve* curve, double angle)
{
    if (curve == 0 || curve->count == 0) {
        return 0.0;
    }
    // Every loader stores the samples immediately after the common count.
    const AerodynamicSample* samples =
        reinterpret_cast<const AerodynamicSample*>(reinterpret_cast<const char*>(curve) + sizeof(*curve));
    if (angle <= samples[0].angle) {
        return samples[0].coefficient;
    }
    unsigned last = curve->count - 1;
    if (angle > samples[last].angle) {
        return samples[last].coefficient;
    }

    unsigned first = 0;
    for (;;) {
        const unsigned middle = (first + last) / 2;
        if (middle == first || middle == last) {
            const AerodynamicSample& lower = samples[middle];
            const AerodynamicSample& upper = samples[middle + 1];
            const double lowerAngle = lower.angle;
            const double lowerCoefficient = lower.coefficient;
            const double fraction = (angle - lowerAngle) / (upper.angle - lowerAngle);
            return (upper.coefficient - lowerCoefficient) * fraction + lowerCoefficient;
        }
        if (angle > samples[middle].angle) {
            first = middle;
        } else if (angle < samples[middle].angle) {
            last = middle;
        } else {
            return samples[middle].coefficient;
        }
    }
}
