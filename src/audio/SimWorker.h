// SimWorker.h
#ifndef SIMWORKER_H
#define SIMWORKER_H
#include <QObject>
#include <QMutex>
#include <QElapsedTimer>
#include <cstdint>
#include "SharedAudio.h"
#include "AudioRingBuffer.h"
#include "IAudioSource.h"
#include "WatchSynthStream.h"
#include "Logger.h"   // TG_NOW()

class TSimWorker : public IAudioSource
{
    Q_OBJECT

public:
    TSimWorker(AudioRingBuffer *ring, int SamplesPerSecond, QObject *parent = nullptr);
    ~TSimWorker();

private:
    AudioRingBuffer *mRawAudio;
public slots:
    void StartSim(WatchSynthStreamConfig cfg);

// IAudioSource signals: dataReady(int64_t), finished(), sourceComplete()
// TSimWorker emits sourceComplete() when synthesis ends (was SimDone).
signals:
    void cancelled();

private:
    bool                    mTimerStarted=false;
    double                  mLastTime=0.0;
    uint64_t                mFrameCount=0;
    uint64_t                mSampleCount=0;
    QElapsedTimer           mTimer;
    int                     mSamplesPerSecond;
    float                   *mDataIn;
    int                     mDataInSize;
};

#endif // SIMWORKER_H
