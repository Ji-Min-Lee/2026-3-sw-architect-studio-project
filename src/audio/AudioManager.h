#pragma once
#include <QObject>
#include <QAudioDevice>
#include "SharedAudio.h"
#include "AudioWorker.h"
#include "PlaybackWorker.h"
#include "SimWorker.h"

// Acquisition Layer: lifecycle manager for audio worker threads.

class AudioManager : public QObject
{
    Q_OBJECT
public:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager() override;

    void startLive(int sampleRate, int bph, double liftAngle,
                   int averagingPeriod, bool useOnset);
    void startPlayback(const QString &filePath, int bph, double liftAngle,
                       int averagingPeriod, bool useOnset);
    void startSim(int bph, double liftAngle, int sampleRate,
                  int averagingPeriod, bool useOnset);
    void stop();

signals:
    void dataReady(TMasterAudioDataRaw *data);
    void playbackDone();
    void simDone();
    // Internal: forwarded to AudioWorker thread
    void liveStartRequested(QAudioDevice dev, int sampleRate, float volume);

private:
    TMasterAudioDataRaw *mRawAudio      = nullptr;
    TAudioWorker        *mAudioWorker    = nullptr;
    TPlaybackWorker     *mPlaybackWorker = nullptr;
    TSimWorker          *mSimWorker      = nullptr;

    void stopCurrent();
};
