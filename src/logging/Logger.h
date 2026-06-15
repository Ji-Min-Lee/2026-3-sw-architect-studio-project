// Logger.h — ported from baseline/experiments2
//
// Per-frame performance logger for the TimeGrapher audio pipeline.
// Streams rows to CSV on every flush interval (consoleEvery frames)
// so data is preserved even if the app is force-quit.
// Console summary printed at the same interval.
//
// Gated by ENABLE_LOGGING compile definition:
//   defined   -> TG_NOW() reads the monotonic clock
//   undefined -> TG_NOW() is 0; instrumentation folds away at zero cost.
//
// Feature/layer mapping vs baseline:
//   copy_us   ring-buffer copy in HandleInputData
//   tg_us     MeasurementEngine::processBlock() (DSP + measurement calc)
//   ui_us     MainWindow::onMeasurementReady() DisplayResults update
//   sound_us  unused (sound render moved to SoundPrintTab, async)
//   plot_us   unused (tab rendering is Qt::QueuedConnection, not in exec path)
//
#ifndef LOGGER_H
#define LOGGER_H

#include <cstdint>
#include <chrono>
#include <vector>
#include <utility>
#include <QString>
#include <QFile>
#include <QTextStream>
#include "SysStats.h"

// Monotonic microsecond clock — same steady_clock on both threads.
static inline int64_t nowUs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

#ifdef ENABLE_LOGGING
  #define TG_NOW() nowUs()
#else
  #define TG_NOW() ((int64_t)0)
#endif

class Logger
{
public:
    struct Frame {
        int     samples  = 0;
        int64_t wait_us  = 0;   // AudioDataReady emit → HandleInputData start
        int64_t exec_us  = 0;   // HandleInputData start → end
        // exec breakdown
        int64_t copy_us  = 0;   // ring-buffer copy
        int64_t sound_us = 0;   // unused in feature/layer (kept for CSV compat)
        int64_t tg_us    = 0;   // MeasurementEngine::processBlock()
        int64_t ui_us    = 0;   // onMeasurementReady() / DisplayResults
        int64_t plot_us  = 0;   // unused in feature/layer (tabs are async)
        int     replot_count = 0; // number of replot() calls this beat (R1 measurement)
        // throughput snapshots (~every 2s)
        double  bg_fps = 0, bg_sps = 0, bg_spf = 0;
        double  fg_fps = 0, fg_sps = 0, fg_spf = 0;
    };

    explicit Logger(const QString &csvPath, int consoleEvery = 100, int sampleRate = 0);
    ~Logger();

    void record(const Frame &f);

private:
    void writeHeader();
    void flushBatch();
    void consoleSummary();
    void writeSysCsv();

    QString      mPath;
    int          mConsoleEvery;
    int          mSampleRate;
    uint64_t     mTotalFrames = 0;

    // Streaming: batch held since last flush
    std::vector<Frame> mBatch;

    QFile        mFile;
    QTextStream  mOut;

    SysStats                                    mSys;
    std::vector<std::pair<uint64_t, SysSample>> mSysSamples;
};

#endif // LOGGER_H
