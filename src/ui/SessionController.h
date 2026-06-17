#pragma once
#include <QObject>
#include <QThread>
#include <QAudioDevice>
#include "IAudioSource.h"
#include "DSPWorker.h"
#include "SharedAudio.h"
#include "MovementSpec.h"
#include "AcquisitionConfig.h"
#include "Logger.h"
#include "WatchSynthStream.h"
#include "BaseGraphTab.h"

// Acquisition-layer coordinator: owns one source-thread / DSP-thread pair
// for the duration of a measurement session.
//
// Responsibility split:
//   SessionController — thread/worker/ring-buffer/logger lifetime
//   MainWindow        — UI state, tab construction, results display
//
// Lifecycle:
//   1. MainWindow constructs SessionController once and calls connectObservers().
//   2. Per session: MainWindow builds MovementSpec + AcquisitionConfig from UI
//      and calls startLive / startPlayback / startSim.
//   3. On completion (Playback/Sim EOF) or stop(): sessionStopped() is emitted.
class SessionController : public QObject
{
    Q_OBJECT
public:
    explicit SessionController(QObject *parent = nullptr);
    ~SessionController() override;

    // Wire MeasurementEngine → tabs + one results receiver.
    // Must be called once before the first session starts; re-applied each session.
    void connectObservers(const QList<BaseGraphTab *> &tabs,
                          QObject *resultsReceiver, const char *measurementSlot);

    // Session start — caller is responsible for building VOs from UI state.
    void startLive(const MovementSpec &movement, const AcquisitionConfig &config,
                   bool useOnset, const QAudioDevice &device, float volume);
    void startPlayback(const MovementSpec &movement, const AcquisitionConfig &config,
                       bool useOnset, const QString &filePath);
    void startSim(const MovementSpec &movement, const AcquisitionConfig &config,
                  bool useOnset, WatchSynthStreamConfig cfg);

    // Session control — safe to call at any time; no-op if no session active.
    void stop();                    // request source-thread interruption (Playback/Sim)
    void stopLiveAudio();           // tell AudioWorker to stop recording (Live only)
    void setMicVolume(float v);     // forward to AudioWorker (Live only)

    DSPWorker *dspWorker() const { return mDspWorker; }

signals:
    void sessionStopped();              // emitted for both EOF and explicit stop()
    void frameLogged(Logger::Frame);    // forwarded from DSPWorker for CSV + status bar

    // Internal per-session audio control — re-connected each live session.
    void _stopAudio();
    void _setVolume(float v);

private slots:
    void onSourceComplete();

private:
    void initRawAudio(int sampleRate);
    void startSourceThread(IAudioSource *source,
                           const MovementSpec &movement,
                           const AcquisitionConfig &config,
                           bool useOnset);

    QList<BaseGraphTab *> mObserverTabs;
    QObject              *mResultsReceiver = nullptr;
    const char           *mResultsSlot     = nullptr;

    TMasterAudioDataRaw  *mRawAudio     = nullptr;
    IAudioSource         *mActiveSource = nullptr;
    QThread              *mSourceThread = nullptr;
    DSPWorker            *mDspWorker    = nullptr;
    QThread              *mDspThread    = nullptr;
    Logger               *mLogger       = nullptr;
};
