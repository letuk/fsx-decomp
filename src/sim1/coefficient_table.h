#ifndef SIM1_COEFFICIENT_TABLE_H
#define SIM1_COEFFICIENT_TABLE_H

#include <float.h>

namespace sim1 {

struct CoefficientInterval {
    unsigned lower;
    unsigned upper;
    double fraction;
};

// Reconstruction-only helper shared by both axes of the coefficient table.
static __forceinline CoefficientInterval FindCoefficientInterval(const double* axis, unsigned count, unsigned stride,
                                                                 double value)
{
    CoefficientInterval interval = {1, 2, 0.0};
    if (value <= axis[stride]) {
        return interval;
    }

    const unsigned last = count - 1;
    if (value > axis[last * stride]) {
        interval.lower = last - 1;
        interval.upper = last;
        interval.fraction = 1.0;
        return interval;
    }

    unsigned upper = 2;
    while (upper < last && axis[upper * stride] < value) {
        ++upper;
    }
    if (value == axis[upper * stride]) {
        interval.lower = upper;
        interval.upper = upper < last ? upper + 1 : upper;
    } else {
        interval.lower = upper - 1;
        interval.upper = upper;
        const double lowerValue = axis[interval.lower * stride];
        interval.fraction = (value - lowerValue) / (axis[upper * stride] - lowerValue);
    }
    return interval;
}

// Shared interpolation for the table storage sizes that reach the original 0x20bcc030.
template <class Table>
__forceinline double EvaluateCoefficientTable(const Table& table, double columnInput, double rowInput)
{
    if (_isnan(columnInput) || _isnan(rowInput)) {
        return table.values[table.columns + 1];
    }

    const CoefficientInterval row = FindCoefficientInterval(table.values, table.rows, table.columns, rowInput);
    const CoefficientInterval column = FindCoefficientInterval(table.values, table.columns, 1, columnInput);
    const double* lowerRow = table.values + row.lower * table.columns;
    const double* upperRow = table.values + row.upper * table.columns;
    const double left = (upperRow[column.lower] - lowerRow[column.lower]) * row.fraction + lowerRow[column.lower];
    const double right = (upperRow[column.upper] - lowerRow[column.upper]) * row.fraction + lowerRow[column.upper];
    return left + (right - left) * column.fraction;
}

} // namespace sim1

#endif
