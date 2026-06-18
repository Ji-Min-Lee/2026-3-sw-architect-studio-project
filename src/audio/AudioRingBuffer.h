#pragma once
#include <QMutex>
#include <memory>
#include <cstdint>

// Thread-safe SPSC (single-producer, single-consumer) ring buffer for PCM float samples.
//
// Writer thread (TAudioWorker / TPlaybackWorker / TSimWorker):
//   reset()          — zero all state at session start
//   write(src, n)    — copy n samples into the ring (two-phase lock protocol)
//   updateStats(...) — store FPS/SPS/SPF for the DSP thread to read
//
// Reader thread (DSPWorker) — exclusive ownership of read-side state:
//   poll()           — mutex-snapshot TotalWritten; returns new sample count
//   drops()          — samples overwritten since last poll() (ring lapped reader)
//   bufferPct()      — ring fill percentage since last poll()
//   readInto(dst, n) — copy n samples from ring using reader-side index (no lock)
//   commitRead()     — advance reader bookmark to the poll() snapshot
//   fps/sps/spf()    — writer-side stats for latency logging
//
// Invariant: readInto() and commitRead() must only be called from one thread.
//            write() and updateStats() must only be called from one thread.
//            poll() acquires mMutex; all other calls are lock-free on their side.
class AudioRingBuffer
{
public:
    explicit AudioRingBuffer(int capacitySamples);
    ~AudioRingBuffer() = default;

    // --- Writer side ---
    void reset();
    void write(const float *src, int n);
    void updateStats(double fps, double sps, double spf);

    // --- Reader side ---
    int    poll();          // snapshot TotalWritten; returns (snapshot - lastTotal)
    int    drops() const;   // valid after poll(); samples lost to ring overwrite
    double bufferPct() const; // valid after poll(); 0–100 fill percentage
    void   readInto(float *dst, int n);
    void   commitRead();    // mLastTotal = mSnapshot

    int    capacity() const { return mCapacity; }
    double fps()      const { return mFPS; }
    double sps()      const { return mSPS; }
    double spf()      const { return mSPF; }

private:
    std::unique_ptr<float[]> mBuffer;
    int      mCapacity;
    QMutex   mMutex;

    // Writer-owned (mutex-protected)
    unsigned int mWriteIndex   = 0;
    uint64_t     mTotalWritten = 0;

    // Reader-owned (DSP thread exclusive — no mutex required)
    unsigned int mReadIndex  = 0;
    uint64_t     mLastTotal  = 0;
    uint64_t     mSnapshot   = 0;

    // Stats — written by audio thread, read by DSP thread.
    // Races here produce stale-but-plausible display values, which is acceptable.
    double mFPS = 0.0;
    double mSPS = 0.0;
    double mSPF = 0.0;
};
