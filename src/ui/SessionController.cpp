#include "SessionController.h"
#include "AudioWorker.h"
#include "PlaybackWorker.h"
#include "SimWorker.h"
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>

SessionController::SessionController(QObject *parent)
    : QObject(parent)
{}

SessionController::~SessionController()
{
    // Workers are parented to their threads (via deleteLater); ring buffer is
    // ours to free.
    delete mLogger;
    if (mRawAudio) {
        delete[] mRawAudio->Samples;
        delete mRawAudio;
    }
}

void SessionController::connectObservers(const QList<BaseGraphTab *> &tabs,
                                         QObject *resultsReceiver,
                                         const char *measurementSlot)
{
    mObserverTabs    = tabs;
    mResultsReceiver = resultsReceiver;
    mResultsSlot     = measurementSlot;
}

// ─────────────────────────────────────────────────────────────────────────────
// Session start — mode-specific factory methods
// ─────────────────────────────────────────────────────────────────────────────

void SessionController::startLive(const MovementSpec &movement,
                                  const AcquisitionConfig &config,
                                  bool useOnset,
                                  const QAudioDevice &device,
                                  float volume)
{
    initRawAudio(config.sampleRate);

    auto *worker = new TAudioWorker(mRawAudio);
    // Live-only controls: connect signals directly to the concrete worker.
    // These connections die when the worker is deleted at session end.
    connect(this, &SessionController::_stopAudio,
            worker, &TAudioWorker::StopAudioRecording);
    connect(this, &SessionController::_setVolume,
            worker, &TAudioWorker::SetAudioInputVolume);

    startSourceThread(worker, movement, config, useOnset);

    // Kick off recording after the thread has started.
    QMetaObject::invokeMethod(worker, "StartAudioRecording",
                              Qt::QueuedConnection,
                              Q_ARG(QAudioDevice, device),
                              Q_ARG(int, config.sampleRate),
                              Q_ARG(float, volume));
}

void SessionController::startPlayback(const MovementSpec &movement,
                                      const AcquisitionConfig &config,
                                      bool useOnset,
                                      const QString &filePath)
{
    initRawAudio(config.sampleRate);

    auto *worker = new TPlaybackWorker(mRawAudio, config.sampleRate);
    startSourceThread(worker, movement, config, useOnset);
    QMetaObject::invokeMethod(worker, "StartPlayback",
                              Qt::QueuedConnection,
                              Q_ARG(QString, filePath));
}

void SessionController::startSim(const MovementSpec &movement,
                                 const AcquisitionConfig &config,
                                 bool useOnset,
                                 WatchSynthStreamConfig cfg)
{
    initRawAudio(config.sampleRate);

    auto *worker = new TSimWorker(mRawAudio, config.sampleRate);
    startSourceThread(worker, movement, config, useOnset);
    QMetaObject::invokeMethod(worker, "StartSim",
                              Qt::QueuedConnection,
                              Q_ARG(WatchSynthStreamConfig, cfg));
}

// ─────────────────────────────────────────────────────────────────────────────
// Session control
// ─────────────────────────────────────────────────────────────────────────────

void SessionController::stop()
{
    if (mSourceThread) mSourceThread->requestInterruption();
}

void SessionController::stopLiveAudio()
{
    emit _stopAudio();
}

void SessionController::setMicVolume(float v)
{
    emit _setVolume(v);
}

// ─────────────────────────────────────────────────────────────────────────────
// Private — shared thread lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void SessionController::initRawAudio(int sampleRate)
{
#ifdef ENABLE_LOGGING
    delete mLogger;
    {
        QString logDir = QCoreApplication::applicationDirPath() + "/logs/EXP-02";
        QDir().mkpath(logDir);
        QString csvPath = logDir + "/log_" +
                          QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv";
        mLogger = new Logger(csvPath, 100, sampleRate);
    }
#endif

    if (mRawAudio) {
        delete[] mRawAudio->Samples;
        delete mRawAudio;
    }
    mRawAudio = new TMasterAudioDataRaw;
    mRawAudio->NumberOfAudioSamples = sampleRate * SECONDS_OF_BUFFER;
    mRawAudio->Samples = new float[mRawAudio->NumberOfAudioSamples];
}

void SessionController::startSourceThread(IAudioSource *source,
                                          const MovementSpec &movement,
                                          const AcquisitionConfig &config,
                                          bool useOnset)
{
    mActiveSource = source;
    mSourceThread = new QThread();
    source->moveToThread(mSourceThread);

    connect(source, &IAudioSource::finished,     mSourceThread, &QThread::quit);
    connect(mSourceThread, &QThread::finished,   source,        &QObject::deleteLater);
    connect(mSourceThread, &QThread::finished,   mSourceThread, &QObject::deleteLater);
    connect(source, &IAudioSource::sourceComplete, this, &SessionController::onSourceComplete);

    // T2: DSP thread
    mDspWorker = new DSPWorker(mRawAudio, movement, config, useOnset);
    mDspThread = new QThread();
    mDspWorker->moveToThread(mDspThread);

    connect(source,       &IAudioSource::dataReady,
            mDspWorker,   &DSPWorker::onDataReady, Qt::QueuedConnection);
    connect(mDspThread,   &QThread::finished,
            mDspWorker,   &QObject::deleteLater);
    connect(mDspThread,   &QThread::finished,
            mDspThread,   &QObject::deleteLater);
    connect(mDspWorker,   &DSPWorker::frameLogged,
            this,         &SessionController::frameLogged, Qt::QueuedConnection);

    // Wire MeasurementEngine → observers
    MeasurementEngine *eng = mDspWorker->engine();
    for (BaseGraphTab *tab : mObserverTabs)
        connect(eng, &MeasurementEngine::measurementReady,
                tab, &BaseGraphTab::onMeasurement, Qt::QueuedConnection);
    if (mResultsReceiver && mResultsSlot)
        connect(eng, SIGNAL(measurementReady(Measurement)),
                mResultsReceiver, mResultsSlot, Qt::QueuedConnection);

    mDspThread->start();
    mSourceThread->start(QThread::TimeCriticalPriority);
}

void SessionController::onSourceComplete()
{
    emit sessionStopped();
}
