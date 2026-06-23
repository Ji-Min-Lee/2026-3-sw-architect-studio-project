#pragma once

#include <QString>
#include <QVector>

enum class UserGuideSection {
    Overview,
    CoreMeasurements,
    RunParameters,
    WatchParameters,
    RateScope,
    SoundPrint,
    Trace,
    Vario,
    Sequence,
    BeatScope,
    BeatError,
    LongTerm,
    Escapement,
    Spectrogram,
    Waveform,
    Sweep,
    Filters,
};

struct UserGuideEntry {
    UserGuideSection section;
    QString          group;
    QString          title;
    QString          html;
};

class UserGuideContent
{
public:
    static QVector<UserGuideEntry> entries();
    static int indexOf(UserGuideSection section);
};
