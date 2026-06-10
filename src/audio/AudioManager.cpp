#include "AudioManager.h"
#include "AudioWorker.h"
#include "PlaybackWorker.h"
#include "SimWorker.h"
#include "WatchSynthStream.h"
#include <QThread>
#include <QAudioDevice>
#include <QMediaDevices>

AudioManager::AudioManager(QObject *parent) : QObject(parent)
{
    mRawAudio = new TMasterAudioDataRaw();
}

AudioManager::~AudioManager()
{
    stop();
    delete mRawAudio;
    mRawAudio = nullptr;
}

void AudioManager::startLive(int sampleRate, int /*bph*/, double /*liftAngle*/,
                              int /*averagingPeriod*/, bool /*useOnset*/)
{
    stop();
    auto *thread = new QThread(this);
    mAudioWorker = new TAudioWorker(mRawAudio);
    mAudioWorker->moveToThread(thread);

    connect(mAudioWorker, &TAudioWorker::AudioDataReady, this,
            [this]() { emit dataReady(mRawAudio); }, Qt::QueuedConnection);
    connect(thread, &QThread::finished, mAudioWorker, &QObject::deleteLater);

    thread->start();

    QAudioDevice preferredDev;
    for (const QAudioDevice &dev : QMediaDevices::audioInputs()) {
        preferredDev = dev;
        break;
    }
    emit liveStartRequested(preferredDev, sampleRate, 1.0f);
}

void AudioManager::startPlayback(const QString &filePath, int sampleRate,
                                  double /*liftAngle*/, int /*averagingPeriod*/, bool /*useOnset*/)
{
    stop();
    auto *thread = new QThread(this);
    mPlaybackWorker = new TPlaybackWorker(mRawAudio, sampleRate);
    mPlaybackWorker->moveToThread(thread);

    connect(mPlaybackWorker, &TPlaybackWorker::PlaybackDataReady, this,
            [this]() { emit dataReady(mRawAudio); }, Qt::QueuedConnection);
    connect(mPlaybackWorker, &TPlaybackWorker::PlaybackDoneReadingFile,
            this, &AudioManager::playbackDone, Qt::QueuedConnection);
    connect(thread, &QThread::finished, mPlaybackWorker, &QObject::deleteLater);

    thread->start();
    QMetaObject::invokeMethod(mPlaybackWorker, "StartPlayback",
                               Qt::QueuedConnection, Q_ARG(QString, filePath));
}

void AudioManager::startSim(int bph, double liftAngle, int sampleRate,
                             int /*averagingPeriod*/, bool /*useOnset*/)
{
    stop();
    auto *thread = new QThread(this);
    mSimWorker = new TSimWorker(mRawAudio, sampleRate);
    mSimWorker->moveToThread(thread);

    connect(mSimWorker, &TSimWorker::SimDataReady, this,
            [this]() { emit dataReady(mRawAudio); }, Qt::QueuedConnection);
    connect(mSimWorker, &TSimWorker::SimDone,
            this, &AudioManager::simDone, Qt::QueuedConnection);
    connect(thread, &QThread::finished, mSimWorker, &QObject::deleteLater);

    thread->start();

    WatchSynthStreamConfig cfg;
    cfg.bph                 = bph;
    cfg.lift_angle_degrees  = liftAngle;
    QMetaObject::invokeMethod(mSimWorker, "StartSim",
                               Qt::QueuedConnection,
                               Q_ARG(WatchSynthStreamConfig, cfg));
}

void AudioManager::stop()
{
    mAudioWorker    = nullptr;
    mPlaybackWorker = nullptr;
    mSimWorker      = nullptr;
    // Threads will clean up via finished()/deleteLater() chains set up at start
}
