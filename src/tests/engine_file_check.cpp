// Verification harness: feeds a WAV test file through MeasurementEngine
// exactly as MainWindow does (4096-sample float blocks) and checks the
// emitted Measurement values against the ground truth encoded in the
// file name (e.g. 21600BPH_NH35.wav -> 21600 BPH).
//
// Usage: EngineFileCheck <file.wav> <expectedBph> [out.csv]
#include <QCoreApplication>
#include <QFile>
#include <QVector>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "MeasurementEngine.h"
#include "MovementSpec.h"
#include "AcquisitionConfig.h"

static bool readWavFloatMono(const QString &path, QVector<float> &samples, int &rate)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QByteArray fileData = f.readAll();
    if (fileData.size() < 44 || memcmp(fileData.constData(), "RIFF", 4) || memcmp(fileData.constData() + 8, "WAVE", 4))
        return false;
    int pos = 12, fmt = 0, channels = 0, bits = 0;
    const char *p = fileData.constData();
    while (pos + 8 <= fileData.size()) {
        quint32 sz; memcpy(&sz, p + pos + 4, 4);
        if (!memcmp(p + pos, "fmt ", 4)) {
            quint16 v16; quint32 v32;
            memcpy(&v16, p + pos + 8, 2);  fmt = v16;
            memcpy(&v16, p + pos + 10, 2); channels = v16;
            memcpy(&v32, p + pos + 12, 4); rate = (int)v32;
            memcpy(&v16, p + pos + 22, 2); bits = v16;
        } else if (!memcmp(p + pos, "data", 4)) {
            if ((fmt != 3 && fmt != 1) || channels != 1) return false; // float or pcm16, mono
            int sampleCount = (int)sz / (bits / 8);
            samples.resize(sampleCount);
            if (fmt == 3 && bits == 32) {
                memcpy(samples.data(), p + pos + 8, (size_t)sampleCount * 4);
            } else if (fmt == 1 && bits == 16) {
                const qint16 *rawSamples = (const qint16 *)(p + pos + 8);
                for (int i = 0; i < sampleCount; i++) samples[i] = rawSamples[i] / 32768.0f;
            } else return false;
            return true;
        }
        pos += 8 + (int)sz + ((int)sz & 1);
    }
    return false;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    if (argc < 3) { fprintf(stderr, "usage: %s <file.wav> <expectedBph> [out.csv]\n", argv[0]); return 2; }

    QVector<float> pcm; int rate = 0;
    if (!readWavFloatMono(argv[1], pcm, rate)) { fprintf(stderr, "cannot read %s\n", argv[1]); return 2; }
    const int expectedBph = atoi(argv[2]);
    FILE *csv = (argc > 3) ? fopen(argv[3], "w") : nullptr;
    if (csv) fprintf(csv, "block,timeSec,synced,detectedBph,rateValid,rateErrorSpd,beatErrorValid,beatErrorMs,amplitudeValid,amplitudeDeg,nEvents\n");

    MeasurementEngine engine;
    MovementSpec movement{0, 52.0};
    AcquisitionConfig config{rate, 200.0, 12};
    engine.init(movement, config);
    engine.reset();

    // Collected results
    int blockNo = 0;
    Measurement last;
    QVector<double> aEventTimes;          // A(T1) event times in seconds
    int firstSyncBlock = -1;

    QObject::connect(&engine, &MeasurementEngine::measurementReady,
                     [&](const Measurement &m) {
        last = m;
        if (m.synced && firstSyncBlock < 0) firstSyncBlock = blockNo;
        for (const AcousticEvent &ev : m.events)
            if (ev.isA) aEventTimes.append(ev.samplePos / m.signal.samplesPerSecond);
        if (csv)
            fprintf(csv, "%d,%.3f,%d,%d,%d,%.3f,%d,%.4f,%d,%.1f,%d\n",
                    blockNo, (double)blockNo * 4096.0 / rate, m.synced, m.detectedBph,
                    m.metrics.rate.has_value(), m.metrics.rate.value_or(0.0),
                    m.metrics.beatError.has_value(), m.metrics.beatError.value_or(0.0),
                    m.metrics.amplitude.has_value(), m.metrics.amplitude.value_or(0.0),
                    (int)m.events.size());
    });

    const int BLOCK = 4096; // DETECTOR_NUMBER_OF_SAMPLES, same as MainWindow
    for (int off = 0; off + BLOCK <= pcm.size(); off += BLOCK, blockNo++)
        engine.processBlock(pcm.constData() + off, BLOCK);

    if (csv) fclose(csv);

    // Independent BPH estimate from median A->A interval (one beat period)
    double bphFromEvents = 0.0;
    if (aEventTimes.size() > 10) {
        QVector<double> iv;
        for (int i = 1; i < aEventTimes.size(); i++) {
            double interval = aEventTimes[i] - aEventTimes[i - 1];
            if (interval > 0.02 && interval < 1.0) iv.append(interval);
        }
        std::sort(iv.begin(), iv.end());
        if (!iv.isEmpty()) bphFromEvents = 3600.0 / iv[iv.size() / 2];
    }

    double fileSec = (double)pcm.size() / rate;
    printf("file            : %s (%.1f s, %d Hz, %d samples)\n", argv[1], fileSec, rate, (int)pcm.size());
    printf("expected BPH    : %d\n", expectedBph);
    printf("synced          : %s (first at block %d, t=%.1fs)\n",
           last.synced ? "yes" : "NO", firstSyncBlock, firstSyncBlock * 4096.0 / rate);
    printf("detected BPH    : %d\n", last.detectedBph);
    printf("BPH from A-A iv : %.0f  (median beat interval, %d A-events)\n", bphFromEvents, (int)aEventTimes.size());
    printf("rate error      : %s %.2f s/d\n", last.metrics.rate.has_value() ? "valid" : "INVALID", last.metrics.rate.value_or(0.0));
    printf("beat error      : %s %.3f ms\n", last.metrics.beatError.has_value() ? "valid" : "INVALID", last.metrics.beatError.value_or(0.0));
    printf("amplitude       : %s %.1f deg\n", last.metrics.amplitude.has_value() ? "valid" : "INVALID", last.metrics.amplitude.value_or(0.0));

    bool ok = last.synced
           && last.detectedBph == expectedBph
           && std::abs(bphFromEvents - expectedBph) / expectedBph < 0.01
           && last.metrics.rate.has_value() && last.metrics.beatError.has_value() && last.metrics.amplitude.has_value();
    printf("VERDICT         : %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
