// SimWorker.cpp
#include <QtGlobal>
#include <QFile>
#include <QThread>
#include <QDebug>
#include "SimWorker.h"
#include "Logger.h"   // nowUs() / TG_NOW()

#if defined(Q_OS_WIN)
#define SIM_SAMPLE_PERIOD_MSEC 10
#define DELAY_FUGE_TIME_MS 1
#elif defined(Q_OS_LINUX)
#define SIM_SAMPLE_PERIOD_MSEC 20
#define DELAY_FUGE_TIME_MS 1
#elif defined(Q_OS_APPLE)
#define SIM_SAMPLE_PERIOD_MSEC 10
#define DELAY_FUGE_TIME_MS 1
#elif defined(Q_OS_ANDROID)
#define SIM_SAMPLE_PERIOD_MSEC 20
#define DELAY_FUGE_TIME_MS 1
#endif

#define SIM_NUMBER_OF_SAMPLES (mSamplesPerSecond/(1000/SIM_SAMPLE_PERIOD_MSEC))

TSimWorker::TSimWorker(AudioRingBuffer *ring, int SamplesPerSecond, QObject *parent) : IAudioSource(parent)
{
    mRawAudio = ring;
    mRawAudio->reset();
    mTimerStarted     = false;
    mSamplesPerSecond = SamplesPerSecond;
    mLastTime         = 0.0;
    mFrameCount       = 0;
    mSampleCount      = 0;
    mDataInSize       = SIM_NUMBER_OF_SAMPLES;
    mDataIn           = new float[mDataInSize];
}

TSimWorker::~TSimWorker()
{
    delete [] mDataIn;
    // Clean up if necessary
    qInfo() << "SimWorker Destructor";
}


void TSimWorker::StartSim(WatchSynthStreamConfig cfg)
{
    int                        BytesIn;
    double                     CurrentTime;
    qint64                     Start,elapsedMs,SleepTime;
    char                       initError[256];
    WatchSynthStream           stream;
    WatchSynthStreamEvent      events[16];
    WatchSynthStreamFillResult fillResult;
    cfg.sample_rate_hz=mSamplesPerSecond;

    if (!watch_synth_stream_init(&stream, &cfg, initError, sizeof(initError)))
    {
        fprintf(stderr, "init failed: %s\n", initError);
        emit sourceComplete();
        emit finished();
        return;
    }

    if (!mTimerStarted)
    {
        mTimerStarted=true;
        mTimer.start();
    }

    while (1)
    {
        Start=mTimer.elapsed();

        fillResult = watch_synth_stream_fill_f32(&stream,  (float *)mDataIn, mDataInSize, events, 16);
        if (fillResult.samples_written != mDataInSize) {
            fprintf(stderr, "short fill\n");
            break;
        }
        if (QThread::currentThread()->isInterruptionRequested())
        {
            break; // Exit loop early
        }
        int NumberOfSamples = static_cast<int>(fillResult.samples_written);

        mRawAudio->write(mDataIn, NumberOfSamples);
        emit dataReady(TG_NOW()); // TS1: emit timestamp for wait_us measurement

        ++mFrameCount;
        mSampleCount += NumberOfSamples;
        CurrentTime = mTimer.elapsed() / 1000.0;
        if (CurrentTime - mLastTime > 2.0) {
            double elapsed = CurrentTime - mLastTime;
            mRawAudio->updateStats(mFrameCount / elapsed, mSampleCount / elapsed,
                                   mSampleCount / static_cast<double>(mFrameCount));
            mLastTime    = CurrentTime;
            mFrameCount  = 0;
            mSampleCount = 0;
        }
        elapsedMs=(mTimer.elapsed()-Start)+DELAY_FUGE_TIME_MS;
        SleepTime=SIM_SAMPLE_PERIOD_MSEC-elapsedMs;
        if (SleepTime<0) SleepTime=0;
        QThread::msleep(SleepTime);
    }
    emit sourceComplete();
    emit finished();
    qInfo()<<"After Finish";
}

