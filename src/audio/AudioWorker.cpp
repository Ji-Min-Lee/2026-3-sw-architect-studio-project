// AudioWorker.cpp
#include "AudioWorker.h"
#include "Logger.h"   // nowUs() / TG_NOW()
#include <QThread>


TAudioWorker::TAudioWorker(TMasterAudioDataRaw *RawAudio,QObject *parent) : QObject(parent)
{
    mRawAudio=RawAudio;
    mRawAudio->TotalSamplesWritten=0;
    mRawAudio->WriteIndex=0;
    mRawAudio->MainThrd_LastTotalSamplesWritten=0;
    mRawAudio->MainThrd_LastWriteIndex=0;
    mRawAudio->FPS=0.0;
    mRawAudio->SPF=0.0;
    mRawAudio->SPS=0.0;
    TimerStarted=false;
    LastTime=0.0;
    FrameCount=0;
    SampleCount=0;
}

TAudioWorker::~TAudioWorker()
{
    // Clean up if necessary
    //qInfo() << "AudioWorker Destructor";
}

void TAudioWorker::stateChangeAudioInput(QAudio::State newState)
{
    qDebug() << "Input Audio State change: " << newState;
}

void TAudioWorker::StartAudioRecording(QAudioDevice InputDevice,int SampleRate,float Volume)
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

    mAudioInput = new QAudioSource(InputDevice,InputFormat,this);
    mAudioInput->setVolume(Volume);
    connect(mAudioInput, &QAudioSource::stateChanged,this, &TAudioWorker::stateChangeAudioInput);
    mAudioInputDevice = mAudioInput->start(); // Start recording
    connect( mAudioInputDevice, &QIODevice::readyRead, this, &TAudioWorker::ProcessAudioInput);
    qDebug() << "Audio recording started in worker thread.";
}

void TAudioWorker::SetAudioInputVolume(float Volume)
{
 if(mAudioInput) mAudioInput->setVolume(Volume);
}

void TAudioWorker::ProcessAudioInput()
{
    if (!TimerStarted)
    {
        TimerStarted=true;
        Timer.start();
    }
    double CurrentTime;

    QByteArray audioBytes =  mAudioInputDevice->readAll();

    unsigned int NumberOfSamples = audioBytes.length() / SAMPLE_SIZE;
    float *AudioSamples=(float *)audioBytes.constData();
    mRawAudio->Mutex.lock();
    unsigned int TempWriteIndex = mRawAudio->WriteIndex;
    mRawAudio->Mutex.unlock();
    int SamplesLeft=std::min(NumberOfSamples,mRawAudio->NumberOfAudioSamples-TempWriteIndex);
    memcpy(&mRawAudio->Samples[TempWriteIndex], AudioSamples, SamplesLeft * SAMPLE_SIZE);
    //qInfo() << "Bytes in "<< audioBytes.length();
    if(SamplesLeft < NumberOfSamples)
    {
        memcpy(mRawAudio->Samples, &AudioSamples[SamplesLeft], (NumberOfSamples - SamplesLeft) * SAMPLE_SIZE);
        qInfo() << "MasterAudioData Samples Rollover";
    }
    mRawAudio->Mutex.lock();
    mRawAudio->WriteIndex = (TempWriteIndex+ NumberOfSamples) % mRawAudio->NumberOfAudioSamples;
    mRawAudio->TotalSamplesWritten+=NumberOfSamples;
    mRawAudio->Mutex.unlock();
    ++FrameCount;
    SampleCount+=NumberOfSamples;
    CurrentTime = Timer.elapsed()/1000.0;
    if (CurrentTime-LastTime > 2) // average fps over 2 seconds
    {
        double elapsedSeconds;
        elapsedSeconds=CurrentTime-LastTime;
        mRawAudio->FPS=FrameCount/elapsedSeconds;
        mRawAudio->SPS=SampleCount/elapsedSeconds;
        mRawAudio->SPF=SampleCount/FrameCount;
        LastTime=CurrentTime;
        FrameCount=0;
        SampleCount=0;
    }
    //qDebug() << "worker thread: handleResults slot is running in thread" << QThread::currentThreadId()<<" "<<count;
    emit AudioDataReady(TG_NOW()); // TS1: emit timestamp for wait_us measurement

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
