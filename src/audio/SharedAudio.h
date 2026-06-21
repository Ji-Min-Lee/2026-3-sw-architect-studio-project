#pragma once
#include <QAudioFormat>

// PCM format constants shared across audio workers and the ring buffer.
#define CHANNELS      1
#define SAMPLE_FORMAT QAudioFormat::Float
#define SAMPLE_SIZE   sizeof(float)
#define SECONDS_OF_BUFFER 30
