struct GeographicDegrees {
    double latitude;
    double longitude;
};

// Reconstructed minimum layout. Only the high words are used in this path.
struct FixedGeographicPosition {
    unsigned latitudeLow;
    int latitudeHigh;
    unsigned longitudeLow;
    unsigned longitudeHigh;
};

extern unsigned magneticLatitudeCount;
extern unsigned magneticLongitudeCount;
extern const float* magneticVariationGrid;

// Original: fe.dll 0x203ecb90
__declspec(noinline) double InterpolateMagneticAngle(
    const double& first, const double& second, const double& fraction)
{
    const double difference = first - second;
    if (difference >= -180.0 && difference <= 180.0) {
        return first + (second - first) * fraction;
    }

    double start = first;
    if (start > 180.0) {
        start -= 360.0;
    } else if (start < -180.0) {
        start += 360.0;
    }

    double end = second;
    if (end > 180.0) {
        end -= 360.0;
    } else if (end < -180.0) {
        end += 360.0;
    }

    const double angle = start + (end - start) * fraction;
    if (angle >= 360.0) {
        return angle - 360.0;
    }
    if (angle < 0.0) {
        return angle + 360.0;
    }
    return angle;
}

// Original: fe.dll 0x203ec9a0
__declspec(noinline) float SampleMagneticVariation(const GeographicDegrees* position)
{
    const float* grid = magneticVariationGrid;
    if (grid == 0) {
        return 0.0f;
    }

    double shiftedLatitude = position->latitude + 90.0;
    if (shiftedLatitude > 180.0) {
        shiftedLatitude = 180.0;
    } else if (shiftedLatitude < 0.0) {
        shiftedLatitude = 0.0;
    }

    const unsigned latitudeCount = magneticLatitudeCount;
    const double latitude = (shiftedLatitude / 180.0) * (latitudeCount - 1);

    double wrappedLongitude = position->longitude;
    if (wrappedLongitude >= 360.0) {
        wrappedLongitude -= 360.0;
    } else if (wrappedLongitude < 0.0) {
        wrappedLongitude += 360.0;
    }

    const unsigned longitudeCount = magneticLongitudeCount;
    const double longitude = (wrappedLongitude / 360.0) * longitudeCount;
    const unsigned column = static_cast<unsigned>(static_cast<__int64>(longitude));
    const unsigned row = static_cast<unsigned>(static_cast<__int64>(latitude));
    const double latitudeFraction = latitude - row;
    const double longitudeFraction = longitude - column;

    const float* currentColumn = grid + latitudeCount * column + row;
    unsigned next = column + 1;
    if (next >= longitudeCount) {
        next = 0;
    }
    const float* nextColumn = grid + latitudeCount * next + row;

    const double first = InterpolateMagneticAngle(currentColumn[0], currentColumn[1], latitudeFraction);
    const double second = InterpolateMagneticAngle(nextColumn[0], nextColumn[1], latitudeFraction);
    double variation = InterpolateMagneticAngle(first, second, longitudeFraction);
    if (variation > 180.0) {
        variation -= 360.0;
    } else if (variation < -180.0) {
        variation += 360.0;
    }
    return static_cast<float>(variation);
}

// Original: fe.dll 0x203ec5d0
__declspec(noinline) int SampleFixedMagneticVariation(const FixedGeographicPosition* position)
{
    GeographicDegrees degrees;
    degrees.latitude = position->latitudeHigh * 8.998425275576774e-06;
    degrees.longitude = position->longitudeHigh * (360.0 / 4294967296.0);
    return static_cast<int>(static_cast<__int64>(SampleMagneticVariation(&degrees) *
                                               (4294967296.0 / 360.0)));
}

// Original: fe.dll 0x203efc30 (ordinal 3)
extern "C" __declspec(dllexport) int __stdcall GetFixedMagneticVariation(
    const FixedGeographicPosition* position)
{
    return SampleFixedMagneticVariation(position);
}

// Original: fe.dll 0x203efc50 (ordinal 4)
extern "C" __declspec(dllexport) float __stdcall GetMagneticVariation(const GeographicDegrees* position)
{
    return SampleMagneticVariation(position);
}
