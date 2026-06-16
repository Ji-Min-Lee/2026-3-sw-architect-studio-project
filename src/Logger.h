// Logger.h
//
// Reusable per-frame performance logger for the TimeGrapher audio pipeline.
// Captures one record per processed frame, buffers them in memory, and writes
// a single per-frame CSV at shutdown. A lightweight console summary is printed
// every `consoleEvery` frames so you can see it is alive while running.
//
// All windowed/aggregate analysis is done offline from the per-frame CSV
// (see src/tools/analyze_log.py) -- the program only emits raw frames.
//
// Logging is gated by the ENABLE_LOGGING compile definition:
//   - ENABLE_LOGGING defined -> TG_NOW() reads the monotonic clock
//   - not defined (default)  -> TG_NOW() is 0, so instrumentation folds away
//                               and the release build carries zero overhead.
//
#ifndef LOGGER_H
#define LOGGER_H

#include <cstdint>
#include <chrono>
#include <vector>
#include <utility>
#include <QString>
#include "SysStats.h"

// Monotonic microsecond clock shared across worker/main threads.
// Same steady_clock source on both sides, so timestamps are directly comparable.
static inline int64_t nowUs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

// Timing probe: real clock read when logging is on, compile-time 0 when off.
#ifdef ENABLE_LOGGING
  #define TG_NOW() nowUs()
#else
  #define TG_NOW() ((int64_t)0)
#endif

class Logger
{
public:
    // One frame's worth of measurements (all durations in microseconds).
    struct Frame {
        int     samples = 0;      // samples processed this frame (backlog indicator)
        int     block_drops = 0;  // samples overwritten before read (WriteIndex lapped ReadIndex)
        double  buffer_pct = 0.0; // ring buffer fill level 0-100%
        int64_t wait_us = 0;      // BG emit -> FG handler start (queue + scheduling)
        int64_t exec_us = 0;      // FG handler start -> end (processing time)
        // exec breakdown
        int64_t copy_us  = 0;
        int64_t sound_us = 0;
        int64_t tg_us    = 0;
        int64_t ui_us    = 0;
        int64_t plot_us  = 0;
        // throughput snapshots (slow-moving, updated ~every 2s by workers)
        double  bg_fps = 0, bg_sps = 0, bg_spf = 0;
        double  fg_fps = 0, fg_sps = 0, fg_spf = 0;
    };

    explicit Logger(const QString &csvPath, int consoleEvery = 100, int sampleRate = 0);
    ~Logger();

    // Record one valid frame (caller should skip frames with samples == 0).
    void record(const Frame &f);

private:
    void consoleSummary();   // average the last consoleEvery frames -> qInfo
    void writeCsv();         // dump all buffered frames -> csvPath
    void writeSysCsv();      // dump system samples -> *_sys.csv

    QString            mPath;
    int                mConsoleEvery;
    int                mSampleRate;  // configured rate, for the CSV metadata line
    std::vector<Frame> mFrames;   // per-frame buffer (written once at shutdown)

    // System metrics, sampled once per console window (~1s), written separately.
    SysStats                              mSys;
    std::vector<std::pair<uint64_t, SysSample>> mSysSamples;  // (frame index, sample)
};

#endif // LOGGER_H
