// SysStats.cpp
#include "SysStats.h"
#include <QtGlobal>

SysStats::SysStats() {}

#if defined(Q_OS_LINUX)
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>

// Parse /proc/stat -> busy/total jiffies for "cpu" and each "cpuN".
static void readCpuTimes(std::vector<SysStats::CpuTimes> &out)
{
    out.clear();
    std::ifstream f("/proc/stat");
    if (!f.is_open()) return;
    std::string line;
    while (std::getline(f, line)) {
        if (line.compare(0, 3, "cpu") != 0) break;   // cpu lines are first
        std::istringstream ss(line);
        std::string tag; ss >> tag;                  // "cpu" or "cpuN"
        unsigned long long v, total = 0, idle = 0;
        int col = 0;
        while (ss >> v) {
            total += v;
            if (col == 3 || col == 4) idle += v;     // idle + iowait
            ++col;
        }
        out.push_back({ total - idle, total });
    }
}

static double readFirstDouble(const char *path, double scale)
{
    std::ifstream f(path);
    if (!f.is_open()) return 0.0;
    double v = 0.0; f >> v;
    return v * scale;
}

static unsigned readThrottled()
{
    FILE *p = popen("vcgencmd get_throttled 2>/dev/null", "r");
    if (!p) return 0;
    char buf[128] = {0};
    unsigned val = 0;
    if (fgets(buf, sizeof(buf), p)) {
        // format: "throttled=0x0"
        const char *eq = strchr(buf, '=');
        if (eq) val = (unsigned)strtoul(eq + 1, nullptr, 0);
    }
    pclose(p);
    return val;
}

SysSample SysStats::sample()
{
    SysSample s;

    // ── CPU (delta based) ─────────────────────────────────────
    std::vector<CpuTimes> cur;
    readCpuTimes(cur);
    if (mHavePrev && cur.size() == mPrev.size() && !cur.empty()) {
        auto util = [](const CpuTimes &a, const CpuTimes &b) -> double {
            unsigned long long dt = b.total - a.total;
            unsigned long long db = b.busy - a.busy;
            return dt ? (100.0 * (double)db / (double)dt) : 0.0;
        };
        s.cpu_total = util(mPrev[0], cur[0]);
        for (size_t i = 1; i < cur.size(); ++i)
            s.cpu_cores.push_back(util(mPrev[i], cur[i]));
        s.valid = true;
    }
    mPrev = cur;
    mHavePrev = !cur.empty();

    // ── memory ────────────────────────────────────────────────
    {
        std::ifstream f("/proc/meminfo");
        std::string key; unsigned long long kb; std::string unit;
        unsigned long long total = 0, avail = 0;
        while (f >> key >> kb >> unit) {
            if (key == "MemTotal:")     total = kb;
            else if (key == "MemAvailable:") { avail = kb; break; }
        }
        s.mem_total_mb = total / 1024.0;
        s.mem_used_mb  = (total - avail) / 1024.0;
    }

    // ── temperature (millidegC) ───────────────────────────────
    s.temp_c = readFirstDouble("/sys/class/thermal/thermal_zone0/temp", 0.001);

    // ── frequency (kHz -> MHz) ────────────────────────────────
    s.freq_mhz = readFirstDouble(
        "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", 0.001);

    // ── throttled bitmask ─────────────────────────────────────
    s.throttled = readThrottled();

    return s;
}

#else  // ── non-Linux stub ──────────────────────────────────────

SysSample SysStats::sample()
{
    SysSample s;            // valid = false; all zeros
    s.valid = false;
    return s;
}

#endif
