#include "WatchMath.h"
#include <cmath>
#include <cstdlib>

namespace WatchMath {

double beatErrorMs(double t0, double t1, double t2, int fs)
{
    double interval1 = (t1 - t0) / fs;  // seconds
    double interval2 = (t2 - t1) / fs;  // seconds
    return std::abs(((interval1 - interval2) / 2.0) * 1000.0);
}

double amplitudeDeg(double aPos, double cPos, int fs, double liftAngle, int bph)
{
    double t_AC   = (cPos - aPos) / static_cast<double>(fs);
    double period = 7200.0 / bph;  // same-phase period in seconds
    double arg    = (2.0 * M_PI * t_AC) / period;
    double sinValue = std::sin(arg);
    if (std::abs(sinValue) < 1e-12) return -1.0;  // degenerate
    double amp = liftAngle / sinValue;
    if (amp >= 360.0 || amp <= 0.0) return -1.0;
    return amp;
}

double escapementMs(double aPos, double cPos, int fs)
{
    return (cPos - aPos) / static_cast<double>(fs) * 1000.0;
}

double wrapInRange(double value, double lo, double hi)
{
    double range   = hi - lo;
    double shifted = std::fmod(value - lo, range);
    if (shifted < 0) shifted += range;
    return shifted + lo;
}

double applyZeroOffset(double firstError, double currentError)
{
    return currentError - firstError;
}

double halfBeatInterval(int bph)
{
    return 3600.0 / bph;
}

double instErrorSec(double timeMeasured, double tStart, int beatIndex, int bph)
{
    double itarget = halfBeatInterval(bph);
    return timeMeasured - (tStart + beatIndex * itarget);
}

double rateSpdFromPhase(double tTicSec, double tTacSec, int bph)
{
    double tNom    = 7200.0 / bph;
    double rateTic = 86400.0 * (tNom / tTicSec - 1.0);
    double rateTac = 86400.0 * (tNom / tTacSec - 1.0);
    return (rateTic + rateTac) / 2.0;
}

} // namespace WatchMath
