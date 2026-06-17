#pragma once
#include <QObject>
#include <QElapsedTimer>
#include "SharedAudio.h"
#include "MeasurementEngine.h"
#include "MovementSpec.h"
#include "AcquisitionConfig.h"
#include "Logger.h"

// T2 tactic: DSP Offload Thread.
// Owns MeasurementEngine and runs the ring-buffer copy + processBlock pipeline
// on a dedicated thread, decoupled from the Qt main thread.
class DSPWorker : public QObject
{
    Q_OBJECT
public:
    explicit DSPWorker(TMasterAudioDataRaw *raw,
                       const MovementSpec &movement,
                       const AcquisitionConfig &config,
                       bool useOnset,
                       QObject *parent = nullptr);
    ~DSPWorker() override;

    MeasurementEngine *engine() const { return mEngine; }

public slots:
    void onDataReady(int64_t ts1);
    void setUseOnset(bool on);

signals:
    void frameLogged(Logger::Frame frame);

private:
    static constexpr unsigned kBlockSize = 4096u;

    TMasterAudioDataRaw *mRaw;
    MeasurementEngine   *mEngine;
    float               *mInputBlock;

    QElapsedTimer mTimer;
    bool     mTimerStarted = false;
    double   mLastTime     = 0.0;
    uint64_t mFrameCount   = 0;
    uint64_t mSampleCount  = 0;
    double   mDspFPS = 0, mDspSPS = 0, mDspSPF = 0;
};
