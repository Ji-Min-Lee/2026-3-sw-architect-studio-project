// Logger.cpp — ported from baseline/experiments2
#include "Logger.h"
#include <QtGlobal>
#include <QSysInfo>

Logger::Logger(const QString &csvPath, int consoleEvery, int sampleRate)
    : mPath(csvPath),
      mConsoleEvery(consoleEvery > 0 ? consoleEvery : 100),
      mSampleRate(sampleRate),
      mFile(csvPath),
      mOut(&mFile)
{
    if (!mFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning("[Logger] failed to open CSV: %s", qPrintable(mPath));
        return;
    }
    writeHeader();
    mBatch.reserve(mConsoleEvery);
    mSys.sample();   // seed CPU delta baseline
    qInfo("[Logger] streaming per-frame logging -> %s (flush every %d frames)",
          qPrintable(mPath), mConsoleEvery);
}

Logger::~Logger()
{
    flushBatch();   // write any remaining frames
    if (mFile.isOpen()) mFile.close();
    writeSysCsv();
    qInfo("[Logger] closed — %llu frames total -> %s",
          (unsigned long long)mTotalFrames, qPrintable(mPath));
}

void Logger::writeHeader()
{
    mOut << "# platform=" << QSysInfo::productType()
         << " kernel="    << QSysInfo::kernelType()
         << " host="      << QSysInfo::machineHostName()
         << " sample_rate=" << mSampleRate << '\n';
    mOut << "frame,samples,total_ms,wait_ms,exec_ms,"
         << "copy_ms,sound_ms,tg_ms,ui_ms,plot_ms,"
         << "bg_fps,bg_sps,bg_spf,fg_fps,fg_sps,fg_spf,"
         << "replot_count,fg_wait_ms\n";
    mOut.flush();
}

void Logger::record(const Frame &f)
{
    mBatch.push_back(f);
    ++mTotalFrames;
    if ((int)mBatch.size() >= mConsoleEvery) {
        consoleSummary();
        flushBatch();
        SysSample s = mSys.sample();
        if (s.valid)
            mSysSamples.emplace_back(mTotalFrames, s);
    }
}

void Logger::flushBatch()
{
    if (mBatch.empty() || !mFile.isOpen()) return;
    for (const Frame &f : mBatch) {
        mOut << mTotalFrames - mBatch.size() + (&f - mBatch.data()) + 1 << ','
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
             << QString::number(f.fg_spf, 'f', 1) << ','
             << f.replot_count << ','
             << QString::number(f.fg_wait_us / 1000.0, 'f', 3) << '\n';
    }
    mOut.flush();
    mBatch.clear();
}

void Logger::consoleSummary()
{
    const size_t cnt = (size_t)mConsoleEvery;

    int64_t sWait=0, sExec=0, sCopy=0, sTg=0, sPlot=0, sFgWait=0, sSamp=0;
    for (const Frame &f : mBatch) {
        sSamp    += f.samples;  sWait   += f.wait_us;  sExec  += f.exec_us;
        sCopy    += f.copy_us;  sTg     += f.tg_us;
        sPlot    += f.plot_us;  sFgWait += f.fg_wait_us;
    }
    const double n = (double)cnt;
    const Frame &last = mBatch.back();

    qInfo("[%06llu] avg_samples=%-7.1f  BG: fps=%-6.1f sps=%-8.1f spf=%-6.1f  DSP: fps=%-6.1f sps=%-8.1f spf=%-6.1f",
          (unsigned long long)mTotalFrames, sSamp / n,
          last.bg_fps, last.bg_sps, last.bg_spf,
          last.fg_fps, last.fg_sps, last.fg_spf);
    qInfo("[%06llu] DSP: wait=%.2f exec=%.2f [copy=%.3f tg=%.3f] ms  |  FG: wait=%.2f plot=%.3f ms",
          (unsigned long long)mTotalFrames,
          sWait   / n / 1000.0,
          sExec   / n / 1000.0,
          sCopy   / n / 1000.0,
          sTg     / n / 1000.0,
          sFgWait / n / 1000.0,
          sPlot   / n / 1000.0);
}

void Logger::writeSysCsv()
{
    if (mSysSamples.empty()) return;

    QString sysPath = mPath.endsWith(".csv", Qt::CaseInsensitive)
                          ? mPath.left(mPath.length() - 4) + "_sys.csv"
                          : mPath + "_sys.csv";
    QFile file(sysPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning("[Logger] failed to open sys CSV: %s", qPrintable(sysPath));
        return;
    }
    const int cores = (int)mSysSamples.front().second.cpu_cores.size();
    QTextStream out(&file);
    out << "frame,cpu_total";
    for (int c = 0; c < cores; ++c) out << ",cpu" << c;
    out << ",mem_used_mb,mem_total_mb,temp_c,freq_mhz,throttled\n";

    for (const auto &pr : mSysSamples) {
        const SysSample &s = pr.second;
        out << pr.first << ',' << QString::number(s.cpu_total, 'f', 1);
        for (int c = 0; c < cores; ++c) {
            double v = (c < (int)s.cpu_cores.size()) ? s.cpu_cores[c] : 0.0;
            out << ',' << QString::number(v, 'f', 1);
        }
        out << ',' << QString::number(s.mem_used_mb,  'f', 1)
            << ',' << QString::number(s.mem_total_mb, 'f', 1)
            << ',' << QString::number(s.temp_c,       'f', 1)
            << ',' << QString::number(s.freq_mhz,     'f', 1)
            << ',' << QString::number((double)s.throttled, 'f', 0) << '\n';
    }
    out.flush();
    file.close();
    qInfo("[Logger] wrote %llu system samples -> %s",
          (unsigned long long)mSysSamples.size(), qPrintable(sysPath));
}
