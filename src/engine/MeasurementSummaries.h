#pragma once

#include <QtGlobal>
#include <cmath>

struct RunningStats
{
    int    count   = 0;
    double current = 0.0;
    double min     = 0.0;
    double max     = 0.0;
    double sum     = 0.0;
    double sumSq   = 0.0;

    void clear()
    {
        count = 0;
        current = 0.0;
        min = 0.0;
        max = 0.0;
        sum = 0.0;
        sumSq = 0.0;
    }

    void add(double value)
    {
        current = value;
        if (count == 0) {
            min = value;
            max = value;
        } else {
            min = qMin(min, value);
            max = qMax(max, value);
        }
        sum += value;
        sumSq += value * value;
        count++;
    }

    double mean() const
    {
        return (count > 0) ? (sum / count) : 0.0;
    }

    double variance() const
    {
        if (count <= 0) return 0.0;
        double m = mean();
        return qMax(0.0, (sumSq / count) - (m * m));
    }

    double stddev() const
    {
        return std::sqrt(variance());
    }

    double delta() const
    {
        return (count > 0) ? (max - min) : 0.0;
    }
};

struct SequenceSummary
{
    RunningStats rate;
    RunningStats amplitude;
    RunningStats beatError;

    void clear()
    {
        rate.clear();
        amplitude.clear();
        beatError.clear();
    }

    void addPosition(double rateSpd, double amplitudeDeg, double beatErrorMs)
    {
        rate.add(rateSpd);
        amplitude.add(amplitudeDeg);
        beatError.add(beatErrorMs);
    }
};
