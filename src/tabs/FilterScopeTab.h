#pragma once
#include "BaseGraphTab.h"
#include "Dsp.h"
#include "qcustomplot.h"
#include <QLabel>
#include <QList>

// Graph 13: Scope Function with Multiple Filter Views — F0..F3 (Figure 19).
//
// Four stacked panels showing the same beat window through each DSP stage.
// Window width = one beat period (3600/BPH s), triggered at the latest A event.
class FilterScopeTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit FilterScopeTab(QWidget *parent = nullptr);
    void reset() override;

public slots:
    void onMeasurement(const Measurement &m) override;
    void replotAll() override;

private:
    struct FilterStages {
        QVector<double> f0;
        QVector<double> f1;
        QVector<double> f2;
        QVector<double> f3;
        QVector<double> f3Threshold;
    };

    struct FilterPanel {
        QLabel      *title = nullptr;
        QCustomPlot *plot = nullptr;
        QCPGraph    *posGraph = nullptr;
        QCPGraph    *negGraph = nullptr;
        QList<QCPItemLine *> markers;
    };

    static constexpr int    kFilterPanels = 4;
    static constexpr double kBufSeconds   = 2.0;
    static constexpr int    kDefaultBph   = 28800;

    QLabel              *mBlockLabel = nullptr;
    QList<FilterPanel>   mPanels;

    // Rolling PCM history (absolute sample coordinates).
    QVector<float>   mRawBuf;
    QVector<float>   mHpfBuf;
    QVector<double>  mPcmBuf;
    QVector<double>  mThrBuf;
    double           mRawStartAbs = 0.0;
    double           mOutStartAbs = 0.0;
    int              mSps = 48000;
    int              mBph = kDefaultBph;

    // Latest captured beat window (A-triggered, one beat long).
    FilterStages          mBeatStages;
    QVector<AcousticEvent> mBeatEvents;
    double                mBeatAnchorAbs = 0.0;
    double                mLastAPos = -1.0;
    bool                  mHaveBeatWindow = false;

    // Fallback block display before the first beat is captured (unit tests).
    Measurement mLatest;
    bool        mHaveData = false;

    tg_hpf_t      mFallbackHpf{};
    tg_envelope_t mFallbackEnv{};
    int           mFallbackSps = 0;
    bool          mFallbackReady = false;

    int  beatSamples() const;
    void appendBuffers(const Measurement &m);
    void tryCaptureBeat(double aPos);
    void refreshBeatEvents(const Measurement &m);
    FilterStages buildStagesFromSlice(const QVector<float> &raw,
                                      const QVector<float> &hpf,
                                      const QVector<double> &pcm,
                                      const QVector<double> &threshold,
                                      int sampleRate) const;
    FilterStages buildFilterStages(const Measurement &m);
    void ensureFallbackDsp(int sampleRate);
    void stylePanel(FilterPanel &panel, bool showXLabel);
    void drawPanel(FilterPanel &panel, int mode,
                   const QVector<double> &xs, const QVector<double> &ys,
                   const QVector<double> *threshold,
                   const QVector<AcousticEvent> &events,
                   double anchorAbs, int sampleRate, bool showEventMarkers);
    void redraw();
};
