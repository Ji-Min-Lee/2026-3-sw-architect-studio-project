#include "AudioRingBuffer.h"
#include <algorithm>
#include <cstring>

AudioRingBuffer::AudioRingBuffer(int capacitySamples)
    : mBuffer(std::make_unique<float[]>(capacitySamples))
    , mCapacity(capacitySamples)
{}

// ─── Writer side ─────────────────────────────────────────────────────────────

void AudioRingBuffer::reset()
{
    QMutexLocker lock(&mMutex);
    mWriteIndex   = 0;
    mTotalWritten = 0;
    mFPS = mSPS = mSPF = 0.0;
    // Reader state is reset here too — reset() is called before any read thread exists.
    mReadIndex = 0;
    mLastTotal = 0;
    mSnapshot  = 0;
}

void AudioRingBuffer::write(const float *src, int n)
{
    // Phase 1: snapshot the write index (lock-free read would require atomics;
    // use the same mutex protocol as the original code).
    mMutex.lock();
    unsigned int idx = mWriteIndex;
    mMutex.unlock();

    // Phase 2: copy — no lock held; only the writer advances mWriteIndex.
    int tail = static_cast<int>(mCapacity) - static_cast<int>(idx);
    int firstChunk = std::min(n, tail);
    std::memcpy(&mBuffer[idx], src, firstChunk * sizeof(float));
    if (firstChunk < n)
        std::memcpy(&mBuffer[0], src + firstChunk, (n - firstChunk) * sizeof(float));

    // Phase 3: commit — update index and total under lock.
    mMutex.lock();
    mWriteIndex    = (static_cast<unsigned int>(idx) + static_cast<unsigned int>(n)) % static_cast<unsigned int>(mCapacity);
    mTotalWritten += static_cast<uint64_t>(n);
    mMutex.unlock();
}

void AudioRingBuffer::updateStats(double fps, double sps, double spf)
{
    mFPS = fps;
    mSPS = sps;
    mSPF = spf;
}

// ─── Reader side ─────────────────────────────────────────────────────────────

int AudioRingBuffer::poll()
{
    mMutex.lock();
    mSnapshot = mTotalWritten;
    mMutex.unlock();
    return static_cast<int>(mSnapshot - mLastTotal);
}

int AudioRingBuffer::drops() const
{
    int avail = static_cast<int>(mSnapshot - mLastTotal);
    return avail > mCapacity ? avail - mCapacity : 0;
}

double AudioRingBuffer::bufferPct() const
{
    int avail = static_cast<int>(mSnapshot - mLastTotal);
    return avail >= mCapacity ? 100.0 : static_cast<double>(avail) / mCapacity * 100.0;
}

void AudioRingBuffer::readInto(float *dst, int n)
{
    for (int i = 0; i < n; ++i) {
        dst[i]     = mBuffer[mReadIndex];
        mReadIndex = (mReadIndex + 1) % static_cast<unsigned int>(mCapacity);
    }
}

void AudioRingBuffer::commitRead()
{
    mLastTotal = mSnapshot;
}
