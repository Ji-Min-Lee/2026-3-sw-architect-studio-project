// AudioWorker.cpp
#include "AudioWorker.h"
#include "Logger.h"   // nowUs() / TG_NOW()
#include <QThread>


TAudioWorker::TAudioWorker(AudioRingBuffer *ring, QObject *parent) : IAudioSource(parent)
{
    mRawAudio = ring;
    mRawAudio->reset();
    TimerStarted = false;
    LastTime     = 0.0;
    FrameCount   = 0;
    SampleCount  = 0;
}

TAudioWorker::~TAudioWorker()
{
}

void TAudioWorker::stateChangeAudioInput(QAudio::State newState)
{
    qDebug() << "Input Audio State change: " << newState;
}

void TAudioWorker::StartAudioRecording(QAudioDevice InputDevice, int SampleRate, float Volume)
{
    QAudioFormat InputFormat;
    InputFormat.setSampleRate(SampleRate);
    InputFormat.setChannelCount(CHANNELS);
    InputFormat.setSampleFormat(SAMPLE_FORMAT);

    if (mAudioInput) delete mAudioInput;
    mAudioInput = nullptr;

    if (InputDevice.isNull()) {
        qWarning() << "No default audio input device found.";
        emit finished();
        return;
    }

    mAudioInput = new QAudioSource(InputDevice, InputFormat, this);
    mAudioInput->setVolume(Volume);
    connect(mAudioInput, &QAudioSource::stateChanged, this, &TAudioWorker::stateChangeAudioInput);
    mAudioInputDevice = mAudioInput->start();
    connect(mAudioInputDevice, &QIODevice::readyRead, this, &TAudioWorker::ProcessAudioInput);
    qDebug() << "Audio recording started in worker thread.";
}

void TAudioWorker::SetAudioInputVolume(float Volume)
{
    if (mAudioInput) mAudioInput->setVolume(Volume);
}

void TAudioWorker::ProcessAudioInput()
{
    if (!TimerStarted) {
        TimerStarted = true;
        Timer.start();
    }

    QByteArray audioBytes = mAudioInputDevice->readAll();
    int NumberOfSamples = audioBytes.length() / SAMPLE_SIZE;
    const float *AudioSamples = reinterpret_cast<const float *>(audioBytes.constData());

    mRawAudio->write(AudioSamples, NumberOfSamples);

    ++FrameCount;
    SampleCount += NumberOfSamples;
    double CurrentTime = Timer.elapsed() / 1000.0;
    if (CurrentTime - LastTime > 2.0) {
        double elapsed = CurrentTime - LastTime;
        mRawAudio->updateStats(FrameCount / elapsed, SampleCount / elapsed,
                               SampleCount / static_cast<double>(FrameCount));
        LastTime    = CurrentTime;
        FrameCount  = 0;
        SampleCount = 0;
    }

    emit dataReady(TG_NOW());
}

void TAudioWorker::StopAudioRecording()
{
    if (mAudioInput) {
        mAudioInput->stop();
        delete mAudioInput;
        mAudioInput = nullptr;
        qDebug() << "Audio recording stopped.";
    }
    emit finished();
}
