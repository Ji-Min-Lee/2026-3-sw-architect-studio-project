// AudioWorker.h
#ifndef AUDIOWORKER_H
#define AUDIOWORKER_H

#include <QObject>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioSource>
#include <QAudioSink>
#include <QBuffer>
#include <QIODevice>
#include <QByteArray>
#include <QDebug>
#include <QMutex>
#include <QElapsedTimer>
#include <cstdint>
#include "SharedAudio.h"
#include "AudioRingBuffer.h"
#include "IAudioSource.h"
#include "Logger.h"   // TG_NOW()


class TAudioWorker : public IAudioSource
{
    Q_OBJECT

public:
    TAudioWorker(AudioRingBuffer *ring, QObject *parent = nullptr);
    ~TAudioWorker();

private:
    AudioRingBuffer *mRawAudio;
public slots:
    void StartAudioRecording(QAudioDevice InputDevice,int SampleRate,float Volume);
    void SetAudioInputVolume(float Volume);
    void StopAudioRecording();

private slots:
    void stateChangeAudioInput(QAudio::State s);
    void ProcessAudioInput();

// IAudioSource signals: dataReady(int64_t), finished(), sourceComplete()
// TAudioWorker never emits sourceComplete() — live mic has no EOF.

private:
    QAudioSource *mAudioInput = nullptr;
    QIODevice    *mAudioInputDevice = nullptr;
    bool          TimerStarted=false;
    double        LastTime=0.0;
    uint64_t      FrameCount=0;
    uint64_t      SampleCount=0;
    QElapsedTimer Timer;
};

#endif // AUDIOWORKER_H
