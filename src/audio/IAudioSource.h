#pragma once
#include <QObject>
#include <cstdint>

// Abstract interface for all PCM audio sources (Live mic, WAV playback, Simulation).
//
// Rationale: TAudioWorker / TPlaybackWorker / TSimWorker share identical thread
// lifecycle and DSP wiring, but MainWindow previously held 3 separate concrete
// pointers and duplicated the connect() block for each mode.  Extracting this
// interface collapses 3 source pointers → 1 (mActiveSource) and lets
// startSourceThread() be written once.
//
// Contract:
//   dataReady(ts)    — emitted each time new PCM is written to the ring buffer.
//                      ts is the emit wall-clock timestamp (µs) for latency logging.
//   finished()       — emitted when the worker's event loop should stop (always).
//   sourceComplete() — emitted when the source has no more data to produce.
//                      Live mic never emits this; Playback/Sim emit it at EOF/end.
class IAudioSource : public QObject
{
    Q_OBJECT
public:
    explicit IAudioSource(QObject *parent = nullptr) : QObject(parent) {}
    ~IAudioSource() override = default;

signals:
    void dataReady(int64_t emitTimestampUs);
    void finished();
    void sourceComplete();
};
