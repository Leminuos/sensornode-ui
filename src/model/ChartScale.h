#ifndef CHART_SCALE_H
#define CHART_SCALE_H

#include <QtMath>

struct ChartScale
{
    double min;
    double max;
    double step;
};

// Trục Y có đúng 3 khoảng, mốc là số nguyên tròn theo 1, 2, 5 × 10^k.
inline ChartScale niceScale(double low, double high)
{
    static constexpr int kIntervals = 3;

    for (double magnitude = 1; ; magnitude *= 10) {
        for (double factor : {1.0, 2.0, 5.0}) {
            const double step = factor * magnitude;
            const double base = qFloor(low / step) * step;
            if (base + kIntervals * step >= high) {
                return {base, base + kIntervals * step, step};
            }
        }
    }
}

#endif // CHART_SCALE_H
