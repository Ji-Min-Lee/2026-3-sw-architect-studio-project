#ifndef SHAREDAUDIO_H
#define SHAREDAUDIO_H
#include <QMutex>
#include <cstdint>   // int64_t used in worker signal signatures

#define CHANNELS      1
#define SAMPLE_FORMAT QAudioFormat::Float
#define SAMPLE_SIZE   sizeof(float)
#define SECONDS_OF_BUFFER 30

typedef struct
{
    QMutex         Mutex;
    float          *Samples;
    int            NumberOfAudioSamples;
    unsigned int   WriteIndex;
    uint64_t       TotalSamplesWritten;
    uint64_t       MainThrd_LastTotalSamplesWritten;
    unsigned int   MainThrd_LastWriteIndex;
    double         FPS;
    double         SPF;
    double         SPS;
} TMasterAudioDataRaw;

#endif // SHAREDAUDIO_H
