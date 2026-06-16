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

void Logger::record(const Frame &frame)
{
    mBatch.push_back(frame);
    ++mTotalFrames;
    if ((int)mBatch.size() >= mConsoleEvery) {
        consoleSummary();
        flushBatch();
        SysSample sysSample = mSys.sample();
        if (sysSample.valid)
            mSysSamples.emplace_back(mTotalFrames, sysSample);
    }
}

void Logger::flushBatch()
{
    if (mBatch.empty() || !mFile.isOpen()) return;
    for (const Frame &frame : mBatch) {
        mOut << mTotalFrames - mBatch.size() + (&frame - mBatch.data()) + 1 << ','
             << frame.samples << ','
             << QString::number((frame.wait_us + frame.exec_us) / 1000.0, 'f', 3) << ','
             << QString::number(frame.wait_us  / 1000.0, 'f', 3) << ','
             << QString::number(frame.exec_us  / 1000.0, 'f', 3) << ','
             << QString::number(frame.copy_us  / 1000.0, 'f', 3) << ','
             << QString::number(frame.sound_us / 1000.0, 'f', 3) << ','
             << QString::number(frame.tg_us    / 1000.0, 'f', 3) << ','
             << QString::number(frame.ui_us    / 1000.0, 'f', 3) << ','
             << QString::number(frame.plot_us  / 1000.0, 'f', 3) << ','
             << QString::number(frame.bg_fps, 'f', 1) << ','
             << QString::number(frame.bg_sps, 'f', 1) << ','
             << QString::number(frame.bg_spf, 'f', 1) << ','
             << QString::number(frame.fg_fps, 'f', 1) << ','
             << QString::number(frame.fg_sps, 'f', 1) << ','
             << QString::number(frame.fg_spf, 'f', 1) << ','
             << frame.replot_count << ','
             << QString::number(frame.fg_wait_us / 1000.0, 'f', 3) << '\n';
    }
    mOut.flush();
    mBatch.clear();
}

void Logger::consoleSummary()
{
    const size_t frameCount = (size_t)mConsoleEvery;

    int64_t sumWaitUs=0, sumExecUs=0, sumCopyUs=0, sumTgUs=0, sumPlotUs=0, sumFgWaitUs=0, sumSamplesCount=0;
    for (const Frame &frame : mBatch) {
        sumSamplesCount += frame.samples;  sumWaitUs   += frame.wait_us;  sumExecUs   += frame.exec_us;
        sumCopyUs       += frame.copy_us;  sumTgUs     += frame.tg_us;
        sumPlotUs       += frame.plot_us;  sumFgWaitUs += frame.fg_wait_us;
    }
    const double frameCountD = (double)frameCount;
    const Frame &last = mBatch.back();

    qInfo("[%06llu] avg_samples=%-7.1f  BG: fps=%-6.1f sps=%-8.1f spf=%-6.1f  DSP: fps=%-6.1f sps=%-8.1f spf=%-6.1f",
          (unsigned long long)mTotalFrames, sumSamplesCount / frameCountD,
          last.bg_fps, last.bg_sps, last.bg_spf,
          last.fg_fps, last.fg_sps, last.fg_spf);
    qInfo("[%06llu] DSP: wait=%.2f exec=%.2f [copy=%.3f tg=%.3f] ms  |  FG: wait=%.2f plot=%.3f ms",
          (unsigned long long)mTotalFrames,
          sumWaitUs   / frameCountD / 1000.0,
          sumExecUs   / frameCountD / 1000.0,
          sumCopyUs   / frameCountD / 1000.0,
          sumTgUs     / frameCountD / 1000.0,
          sumFgWaitUs / frameCountD / 1000.0,
          sumPlotUs   / frameCountD / 1000.0);
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

    for (const auto &frameSampleEntry : mSysSamples) {
        const SysSample &sysSample = frameSampleEntry.second;
        out << frameSampleEntry.first << ',' << QString::number(sysSample.cpu_total, 'f', 1);
        for (int c = 0; c < cores; ++c) {
            double coreUtil = (c < (int)sysSample.cpu_cores.size()) ? sysSample.cpu_cores[c] : 0.0;
            out << ',' << QString::number(coreUtil, 'f', 1);
        }
        out << ',' << QString::number(sysSample.mem_used_mb,  'f', 1)
            << ',' << QString::number(sysSample.mem_total_mb, 'f', 1)
            << ',' << QString::number(sysSample.temp_c,       'f', 1)
            << ',' << QString::number(sysSample.freq_mhz,     'f', 1)
            << ',' << QString::number((double)sysSample.throttled, 'f', 0) << '\n';
    }
    out.flush();
    file.close();
    qInfo("[Logger] wrote %llu system samples -> %s",
          (unsigned long long)mSysSamples.size(), qPrintable(sysPath));
}
