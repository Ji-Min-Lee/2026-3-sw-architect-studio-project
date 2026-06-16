// SysStats.cpp — ported from baseline/experiments2
#include "SysStats.h"
#include <QtGlobal>

SysStats::SysStats() {}

#if defined(Q_OS_LINUX)
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>

static void readCpuTimes(std::vector<SysStats::CpuTimes> &out)
{
    out.clear();
    std::ifstream f("/proc/stat");
    if (!f.is_open()) return;
    std::string line;
    while (std::getline(f, line)) {
        if (line.compare(0, 3, "cpu") != 0) break;
        std::istringstream ss(line);
        std::string tag; ss >> tag;
        unsigned long long fieldValue, total = 0, idle = 0;
        int col = 0;
        while (ss >> fieldValue) {
            total += fieldValue;
            if (col == 3 || col == 4) idle += fieldValue;
            ++col;
        }
        out.push_back({ total - idle, total });
    }
}

static double readFirstDouble(const char *path, double scale)
{
    std::ifstream f(path);
    if (!f.is_open()) return 0.0;
    double rawValue = 0.0; f >> rawValue;
    return rawValue * scale;
}

static unsigned readThrottled()
{
    FILE *p = popen("vcgencmd get_throttled 2>/dev/null", "r");
    if (!p) return 0;
    char pipeOutput[128] = {0};
    unsigned throttledFlags = 0;
    if (fgets(pipeOutput, sizeof(pipeOutput), p)) {
        const char *eq = strchr(pipeOutput, '=');
        if (eq) throttledFlags = (unsigned)strtoul(eq + 1, nullptr, 0);
    }
    pclose(p);
    return throttledFlags;
}

SysSample SysStats::sample()
{
    SysSample result;

    std::vector<CpuTimes> cur;
    readCpuTimes(cur);
    if (mHavePrev && cur.size() == mPrev.size() && !cur.empty()) {
        auto util = [](const CpuTimes &a, const CpuTimes &b) -> double {
            unsigned long long dt = b.total - a.total;
            unsigned long long db = b.busy - a.busy;
            return dt ? (100.0 * (double)db / (double)dt) : 0.0;
        };
        result.cpu_total = util(mPrev[0], cur[0]);
        for (size_t i = 1; i < cur.size(); ++i)
            result.cpu_cores.push_back(util(mPrev[i], cur[i]));
        result.valid = true;
    }
    mPrev = cur;
    mHavePrev = !cur.empty();

    {
        std::ifstream f("/proc/meminfo");
        std::string key; unsigned long long kb; std::string unit;
        unsigned long long total = 0, avail = 0;
        while (f >> key >> kb >> unit) {
            if (key == "MemTotal:")          total = kb;
            else if (key == "MemAvailable:") { avail = kb; break; }
        }
        result.mem_total_mb = total / 1024.0;
        result.mem_used_mb  = (total - avail) / 1024.0;
    }

    result.temp_c   = readFirstDouble("/sys/class/thermal/thermal_zone0/temp", 0.001);
    result.freq_mhz = readFirstDouble(
        "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", 0.001);
    result.throttled = readThrottled();

    return result;
}

#else

SysSample SysStats::sample()
{
    SysSample result;
    result.valid = false;
    return result;
}

#endif
