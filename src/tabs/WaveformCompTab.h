#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QLabel>
#include <QList>

// Graph 11: Waveform Comparison — three time-pass beat windows.
//
// Displays HPF-filtered PCM (bipolar, delay-aligned with events) from the
// same DSP chain as libtimegrapher — NOT mic raw, NOT envelope.
class WaveformCompTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit WaveformCompTab(QWidget *parent = nullptr);
    void reset() override;
    void setLiftAngle(double degrees);

public slots:
    void onMeasurement(const Measurement &m) override;
    void replotAll() override;

private:
    struct BeatWave {
        QVector<double> xs;
        QVector<double> hpfYs;
        double aSamplePos = 0.0;
        double cMs = 0.0;
        double tAcMs = 0.0;
        bool   hasC = false;
    };

    struct PendingBeat {
        double aPos = -1.0;
    };

    struct BeatWindow {
        QLabel      *title = nullptr;
        QCustomPlot *plot = nullptr;
        QCPGraph    *hpfGraph = nullptr;
        QCPItemLine *zeroLine = nullptr;
        QCPItemLine *aMarker = nullptr;
        QCPItemLine *cMarker = nullptr;
        QCPItemText *tAcLabel = nullptr;
        QCPAxis     *degAxis = nullptr;
        QList<QCPItemLine *> msGrid;
    };

    QLabel      *mValuesLabel = nullptr;
    QLabel      *mTacLabel = nullptr;
    QList<BeatWindow> mWindows;

    static constexpr int    kMaxBeats    = 20;
    static constexpr int    kBeatPlots   = 3;
    static constexpr double kPreMs       = 2.0;
    static constexpr double kPostMs      = 20.0;
    static constexpr double kWindowMs    = kPreMs + kPostMs;
    static constexpr double kBufSeconds  = 1.0;

    QList<BeatWave>    mBeats;
    QList<PendingBeat> mPending;
    QVector<float>     mHpfBuf;
    double             mBufStartAbs = 0.0;

    int    mSps = 48000;
    int    mBph = 0;
    double mLiftAngle = 52.0;

    void appendBuffer(const Measurement &m);
    void fulfillPending();
    bool extractBeatWindow(double aPos, BeatWave &out) const;
    void pushBeat(const BeatWave &beat);
    void updateTacStats();
    void redrawPlots();
    void stylePlot(BeatWindow &win);
    void updateDegreeAxis(BeatWindow &win);
    void updatePlotGuides(BeatWindow &beatWindow, const BeatWave *beat, double yMin, double yMax);
};
