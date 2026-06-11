// Logger.cpp
#include "Logger.h"
#include <QtGlobal>
#include <QFile>
#include <QTextStream>

Logger::Logger(const QString &csvPath, int consoleEvery)
    : mPath(csvPath), mConsoleEvery(consoleEvery > 0 ? consoleEvery : 100)
{
    qInfo("[Logger] per-frame logging -> %s (console every %d frames)",
          qPrintable(mPath), mConsoleEvery);
    mFrames.reserve(1 << 16);   // ~65k frames up front to avoid early reallocs
}

Logger::~Logger()
{
    writeCsv();
}

void Logger::record(const Frame &f)
{
    mFrames.push_back(f);   // cheap: memory only, no per-frame disk I/O
    if ((int)(mFrames.size() % mConsoleEvery) == 0)
        consoleSummary();
}

void Logger::consoleSummary()
{
    // average over the most recent mConsoleEvery frames
    const size_t total = mFrames.size();
    const size_t cnt   = (size_t)mConsoleEvery;
    const size_t start = total - cnt;

    int64_t sWait=0, sExec=0, sCopy=0, sSound=0, sTg=0, sUi=0, sPlot=0, sSamp=0;
    for (size_t i = start; i < total; ++i) {
        const Frame &f = mFrames[i];
        sSamp += f.samples; sWait += f.wait_us; sExec += f.exec_us;
        sCopy += f.copy_us; sSound += f.sound_us; sTg += f.tg_us;
        sUi += f.ui_us; sPlot += f.plot_us;
    }
    const double n = (double)cnt;
    const Frame &last = mFrames.back();   // latest throughput snapshot

    qInfo("[%06llu] avg_samples=%-7.1f  BG: fps=%-6.1f sps=%-8.1f spf=%-6.1f  FG: fps=%-6.1f sps=%-8.1f spf=%-6.1f",
          (unsigned long long)total, sSamp / n,
          last.bg_fps, last.bg_sps, last.bg_spf,
          last.fg_fps, last.fg_sps, last.fg_spf);
    qInfo("[%06llu] total=%.2fms [wait=%.2f + exec=%.2f]  exec=[copy=%.3f sound=%.3f tg=%.3f ui=%.3f plot=%.3f] ms",
          (unsigned long long)total,
          (sWait + sExec) / n / 1000.0,
          sWait / n / 1000.0,
          sExec / n / 1000.0,
          sCopy / n / 1000.0,
          sSound / n / 1000.0,
          sTg / n / 1000.0,
          sUi / n / 1000.0,
          sPlot / n / 1000.0);
}

void Logger::writeCsv()
{
    if (mFrames.empty()) return;
    QFile file(mPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning("[Logger] failed to open CSV: %s", qPrintable(mPath));
        return;
    }
    QTextStream out(&file);
    // per-frame rows. durations measured in us, written in ms (3 decimals).
    out << "frame,samples,total_ms,wait_ms,exec_ms,"
        << "copy_ms,sound_ms,tg_ms,ui_ms,plot_ms,"
        << "bg_fps,bg_sps,bg_spf,fg_fps,fg_sps,fg_spf\n";
    uint64_t idx = 0;
    for (const Frame &f : mFrames) {
        ++idx;
        out << idx << ','
            << f.samples << ','
            << QString::number((f.wait_us + f.exec_us) / 1000.0, 'f', 3) << ','
            << QString::number(f.wait_us  / 1000.0, 'f', 3) << ','
            << QString::number(f.exec_us  / 1000.0, 'f', 3) << ','
            << QString::number(f.copy_us  / 1000.0, 'f', 3) << ','
            << QString::number(f.sound_us / 1000.0, 'f', 3) << ','
            << QString::number(f.tg_us    / 1000.0, 'f', 3) << ','
            << QString::number(f.ui_us    / 1000.0, 'f', 3) << ','
            << QString::number(f.plot_us  / 1000.0, 'f', 3) << ','
            << QString::number(f.bg_fps, 'f', 1) << ','
            << QString::number(f.bg_sps, 'f', 1) << ','
            << QString::number(f.bg_spf, 'f', 1) << ','
            << QString::number(f.fg_fps, 'f', 1) << ','
            << QString::number(f.fg_sps, 'f', 1) << ','
            << QString::number(f.fg_spf, 'f', 1) << '\n';
    }
    out.flush();
    file.close();
    qInfo("[Logger] wrote %llu frames -> %s",
          (unsigned long long)mFrames.size(), qPrintable(mPath));
}
