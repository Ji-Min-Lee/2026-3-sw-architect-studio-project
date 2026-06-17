// PlaybackWorker.h
#ifndef PLAYBACKWORKER_H
#define PLAYBACKWORKER_H
#include <QObject>
#include <QMutex>
#include <QElapsedTimer>
#include <cstdint>
#include "SharedAudio.h"
#include "IAudioSource.h"
#include "Logger.h"   // TG_NOW()

class TPlaybackWorker : public IAudioSource
{
    Q_OBJECT

public:
    TPlaybackWorker(TMasterAudioDataRaw *RawAudio,int SamplesPerSecond,QObject *parent = nullptr);
     ~TPlaybackWorker();
    TMasterAudioDataRaw *mRawAudio;
public slots:
    void StartPlayback(const QString &FileName);

// IAudioSource signals: dataReady(int64_t), finished(), sourceComplete()
// TPlaybackWorker emits sourceComplete() at EOF (was PlaybackDoneReadingFile).
signals:
    void cancelled();

private:
    bool          mTimerStarted=false;
    double        mLastTime=0.0;
    uint64_t      mFrameCount=0;
    uint64_t      mSampleCount=0;
    QElapsedTimer mTimer;
    int           mSamplesPerSecond;
    char          *mDataIn;
    int           mDataInSize;
};

#endif // PLAYBACKWORKER_H
