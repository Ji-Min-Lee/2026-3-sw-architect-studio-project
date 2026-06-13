// SysStats.h — ported from baseline/experiments2
//
// Lightweight system metrics sampler (RPi / Linux focused).
//   - overall + per-core CPU utilization (%)   from /proc/stat (delta based)
//   - memory used / total (MB)                  from /proc/meminfo
//   - CPU temperature (C)                       from /sys thermal zone
//   - CPU frequency (MHz)                       from cpufreq
//   - throttled bitmask                         from `vcgencmd get_throttled`
//
// Call sample() about once per second. CPU% needs two readings, so the first
// call seeds the deltas and returns valid=false.
//
// On non-Linux platforms this compiles to a stub returning valid=false, so the
// rest of the code is portable; meaningful data is only produced on the RPi.
//
#ifndef SYSSTATS_H
#define SYSSTATS_H

#include <vector>

struct SysSample {
    double              cpu_total = 0.0;
    std::vector<double> cpu_cores;
    double              mem_used_mb  = 0.0;
    double              mem_total_mb = 0.0;
    double              temp_c    = 0.0;
    double              freq_mhz  = 0.0;
    unsigned            throttled = 0;
    bool                valid = false;
};

class SysStats
{
public:
    struct CpuTimes { unsigned long long busy = 0, total = 0; };

    SysStats();
    SysSample sample();

private:
    std::vector<CpuTimes> mPrev;
    bool mHavePrev = false;
};

#endif // SYSSTATS_H
