// PlaybackWorker.cpp
#include <QtGlobal>
#include <QFile>
#include <QThread>
#include <QDebug>
#include "PlaybackWorker.h"
#include "Logger.h"   // nowUs() / TG_NOW()
#include "WaveHeader.h"

#if defined(Q_OS_WIN)
#define PLAYBACK_SAMPLE_PERIOD_MSEC 10
#define DELAY_FUGE_TIME_MS 1
#elif defined(Q_OS_LINUX)
#define PLAYBACK_SAMPLE_PERIOD_MSEC 20
#define DELAY_FUGE_TIME_MS 1
#elif defined(Q_OS_APPLE)
#define PLAYBACK_SAMPLE_PERIOD_MSEC 10
#define DELAY_FUGE_TIME_MS 1
#elif defined(Q_OS_ANDROID)
#define PLAYBACK_SAMPLE_PERIOD_MSEC 20
#define DELAY_FUGE_TIME_MS 1
#endif

#define PLAYBACK_NUMBER_OF_SAMPLES (mSamplesPerSecond/(1000/PLAYBACK_SAMPLE_PERIOD_MSEC))

TPlaybackWorker::TPlaybackWorker(AudioRingBuffer *ring, int SamplesPerSecond, QObject *parent) : IAudioSource(parent)
{
    mRawAudio = ring;
    mRawAudio->reset();
    mTimerStarted    = false;
    mSamplesPerSecond = SamplesPerSecond;
    mLastTime         = 0.0;
    mFrameCount       = 0;
    mSampleCount      = 0;
    mDataInSize       = SAMPLE_SIZE * PLAYBACK_NUMBER_OF_SAMPLES;
    mDataIn           = new char[mDataInSize];
}

TPlaybackWorker::~TPlaybackWorker()
{
    delete [] mDataIn;
    // Clean up if necessary
    qInfo() << "PlaybackWorker Destructor";
}


void TPlaybackWorker::StartPlayback(const QString &FileName)
{
    int BytesIn;
    double CurrentTime;
    qint64 Start,elapsedMs,SleepTime;

    if (!mTimerStarted)
    {
        mTimerStarted=true;
        mTimer.start();
    }

    QFile *file = new QFile(FileName);
    TWaveHeader header;
    if (!file->exists()) {
        delete file;
        emit sourceComplete();
        emit finished();
        return;
    }

    if (!file->open(QIODevice::ReadOnly))
    {
        delete file;
        emit sourceComplete();
        emit finished();
        return;
    }

    QDataStream in(file);
    in.setByteOrder(QDataStream::LittleEndian); // WAV is Little Endian

    file->read(header.riffId, 4);
    in >> header.fileSize;
    file->read(header.waveId, 4);
    file->read(header.fmtId, 4);
    in >> header.fmtSize;
    in >> header.audioFormat;
    in >> header.numChannels;
    in >> header.sampleRate;
    in >> header.byteRate;
    in >> header.blockAlign;
    in >> header.bitsPerSample;

    // Skip any extra fmt bytes if fmtSize > 16
    if (header.fmtSize > 16) file->seek(file->pos() + (header.fmtSize - 16));

    // Look for "data" chunk (it might not be immediately after fmt)
    char chunkId[4];
    while (!file->atEnd()) {
        file->read(chunkId, 4);
        uint32_t chunkSize;
        in >> chunkSize;
        if (qstrncmp(chunkId, "data", 4) == 0) {
            header.dataSize = chunkSize;
            break;
        }
        file->seek(file->pos() + chunkSize);
    }
    if (qstrncmp(header.riffId, "RIFF", 4) != 0 || (header.sampleRate!=mSamplesPerSecond) ||
        (header.numChannels!=1)|| (header.bitsPerSample != 32)||
        (header.audioFormat != 3))
    {
     emit sourceComplete();
        file->close();
        delete file;
        emit sourceComplete();
        emit finished();
        return;
    }
    int numSamples = header.dataSize / sizeof(float);

    while (!in.atEnd() && (numSamples>0))
    {
        Start=mTimer.elapsed();

        BytesIn=in.readRawData(mDataIn, mDataInSize);
        if (BytesIn<0)
        {
           qInfo() << "Read Error ="<<BytesIn;
           break;
        }
        else if ((BytesIn%4)!=0)
        {
          qInfo() << "Read Error not Modulus of 4";
          break;
        }
        else if (BytesIn==0)
        {
          qInfo() << "Read Error 0";
          break;
        }
        else if (QThread::currentThread()->isInterruptionRequested())
        {
            break; // Exit loop early
        }
        int NumberOfSamples = BytesIn / SAMPLE_SIZE;

        mRawAudio->write(reinterpret_cast<const float *>(mDataIn), NumberOfSamples);
        emit dataReady(TG_NOW()); // TS1: emit timestamp for wait_us measurement

        ++mFrameCount;
        mSampleCount += NumberOfSamples;
        numSamples   -= NumberOfSamples;
        CurrentTime = mTimer.elapsed() / 1000.0;
        if (CurrentTime - mLastTime > 2.0) {
            double elapsed = CurrentTime - mLastTime;
            mRawAudio->updateStats(mFrameCount / elapsed, mSampleCount / elapsed,
                                   mSampleCount / static_cast<double>(mFrameCount));
            mLastTime    = CurrentTime;
            mFrameCount  = 0;
            mSampleCount = 0;
        }
        elapsedMs=(mTimer.elapsed()-Start)+DELAY_FUGE_TIME_MS;
        SleepTime=PLAYBACK_SAMPLE_PERIOD_MSEC-elapsedMs;
        if (SleepTime<0) SleepTime=0;
        QThread::msleep(SleepTime);
    }
    qInfo()<<"Before Close";
    file->close();
    delete file;
    emit sourceComplete();
    emit finished();
    qInfo()<<"After Finish";
}

