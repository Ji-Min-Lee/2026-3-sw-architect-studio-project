#include "DSPWorker.h"
#include <cstdlib>
#include <algorithm>

DSPWorker::DSPWorker(AudioRingBuffer *ring,
                     const MovementSpec &movement,
                     const AcquisitionConfig &config,
                     bool useOnset,
                     QObject *parent)
    : QObject(parent), mRaw(ring)
{
    mEngine = new MeasurementEngine(this);
    mEngine->init(movement, config);
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

    int samplesToAdd  = mRaw->poll();

    Logger::Frame frame;
    frame.samples     = samplesToAdd;
    frame.block_drops = mRaw->drops();
    frame.buffer_pct  = mRaw->bufferPct();
    frame.wait_us     = dspStart - ts1;  // Worker emit → DSP thread pickup (queue + sched)

    if (!mTimerStarted) {
        mTimer.start();
        mTimerStarted = true;
    }

    if (samplesToAdd > 0) {
        while (samplesToAdd > 0) {
            int slice = std::min(samplesToAdd, static_cast<int>(kBlockSize));

            int64_t copyStart = TG_NOW();
            mRaw->readInto(mInputBlock, slice);
            frame.copy_us += TG_NOW() - copyStart;

            int64_t tgStart = TG_NOW();
            mEngine->processBlock(mInputBlock, slice);
            frame.tg_us += TG_NOW() - tgStart;

            mSampleCount += static_cast<uint64_t>(slice);
            samplesToAdd -= slice;
        }
        mRaw->commitRead();

        mFrameCount++;
        double now = mTimer.elapsed() / 1000.0;
        if (now - mLastTime > 2.0) {
            double elapsedSec = now - mLastTime;
            mDspFPS = mFrameCount  / elapsedSec;
            mDspSPS = mSampleCount / elapsedSec;
            mDspSPF = mSampleCount / static_cast<double>(mFrameCount);
            mLastTime    = now;
            mFrameCount  = 0;
            mSampleCount = 0;
        }
    }

    frame.exec_us     = TG_NOW() - dspStart;
    frame.bg_fps      = mRaw->fps();
    frame.bg_sps      = mRaw->sps();
    frame.bg_spf      = mRaw->spf();
    frame.fg_fps      = mDspFPS;  // fg = DSP thread (T2 semantics)
    frame.fg_sps      = mDspSPS;
    frame.fg_spf      = mDspSPF;
    frame.dsp_emit_ts = TG_NOW();  // FG wait = onFrameLogged start - this timestamp

    emit frameLogged(frame);
}

void DSPWorker::setUseOnset(bool on)
{
    mEngine->setUseOnset(on);
}
