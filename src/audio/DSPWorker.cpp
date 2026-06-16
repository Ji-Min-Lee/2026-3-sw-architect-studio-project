#include "DSPWorker.h"
#include <cstdlib>
#include <algorithm>

DSPWorker::DSPWorker(TMasterAudioDataRaw *raw,
                     int sampleRate, int bph, double liftAngle,
                     int avgPeriod, double highPass, bool useOnset,
                     QObject *parent)
    : QObject(parent), mRaw(raw)
{
    mEngine = new MeasurementEngine(this);
    mEngine->init(sampleRate, bph, liftAngle, avgPeriod, highPass);
    mEngine->setUseOnset(useOnset);
    mEngine->reset();

    mInputBlock = static_cast<float *>(malloc(kBlockSize * sizeof(float)));
    if (!mInputBlock) throw std::runtime_error("DSPWorker: block alloc failed");
}

DSPWorker::~DSPWorker()
{
    free(mInputBlock);
}

void DSPWorker::onDataReady(int64_t ts1)
{
    int64_t dspStart = TG_NOW();

    mRaw->Mutex.lock();
    uint64_t totalWritten = mRaw->TotalSamplesWritten;
    mRaw->Mutex.unlock();

    int samplesToAdd = static_cast<int>(totalWritten - mRaw->MainThrd_LastTotalSamplesWritten);

    Logger::Frame frame;
    frame.samples = samplesToAdd;
    frame.wait_us = dspStart - ts1;  // Worker emit → DSP thread pickup (queue + sched)

    if (!mTimerStarted) {
        mTimer.start();
        mTimerStarted = true;
    }

    if (samplesToAdd > 0) {
        while (samplesToAdd > 0) {
            int slice = std::min(samplesToAdd, static_cast<int>(kBlockSize));

            int64_t copyStart = TG_NOW();
            for (int i = 0; i < slice; i++) {
                mInputBlock[i] = mRaw->Samples[mRaw->MainThrd_LastWriteIndex];
                mRaw->MainThrd_LastWriteIndex =
                    (mRaw->MainThrd_LastWriteIndex + 1) % mRaw->NumberOfAudioSamples;
            }
            frame.copy_us += TG_NOW() - copyStart;

            int64_t tgStart = TG_NOW();
            mEngine->processBlock(mInputBlock, slice);
            frame.tg_us += TG_NOW() - tgStart;

            mSampleCount += static_cast<uint64_t>(slice);
            samplesToAdd -= slice;
        }
        mRaw->MainThrd_LastTotalSamplesWritten = totalWritten;

        mFrameCount++;
        double now = mTimer.elapsed() / 1000.0;
        if (now - mLastTime > 2.0) {
            double elapsedSec = now - mLastTime;
            mDspFPS = mFrameCount  / elapsedSec;
            mDspSPS = mSampleCount / elapsedSec;
            mDspSPF = mSampleCount / static_cast<double>(mFrameCount);
            mLastTime     = now;
            mFrameCount   = 0;
            mSampleCount  = 0;
        }
    }

    frame.exec_us = TG_NOW() - dspStart;
    frame.bg_fps  = mRaw->FPS;
    frame.bg_sps  = mRaw->SPS;
    frame.bg_spf  = mRaw->SPF;
    frame.fg_fps  = mDspFPS;  // fg = DSP thread (T2 semantics)
    frame.fg_sps  = mDspSPS;
    frame.fg_spf  = mDspSPF;
    frame.dsp_emit_ts = TG_NOW();  // FG wait = onFrameLogged start - this timestamp

    emit frameLogged(frame);
}

void DSPWorker::setUseOnset(bool on)
{
    mEngine->setUseOnset(on);
}
