#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QLabel>
#include <QList>
#include <vector>

// Graph 13: Scope Function with Multiple Filter Views — F0..F3 (Figure 19).
//
// Four stacked time-pass panels showing the same PCM block through each
// filter stage simultaneously for side-by-side comparison.
class FilterScopeTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit FilterScopeTab(QWidget *parent = nullptr);
    void reset() override;

public slots:
    void onMeasurement(const Measurement &m) override;

private:
    struct FilterStages {
        QVector<double> f0;
        QVector<double> f1;
        QVector<double> f2;
        QVector<double> f3;
    };

    struct FilterPanel {
        QLabel      *title = nullptr;
        QCustomPlot *plot = nullptr;
        QCPGraph    *posGraph = nullptr;
        QCPGraph    *negGraph = nullptr;
        QList<QCPItemLine *> markers;
    };

    static constexpr int kFilterPanels = 4;

    QLabel              *mBlockLabel = nullptr;
    QList<FilterPanel>   mPanels;

    Measurement mLatest;
    bool        mHaveData = false;

    std::vector<float> mHpfRing;
    uint64_t           mRingStartTick  = 0;
    int                mSampleRate     = 48000;
    int                mBph            = 28800;
    uint64_t           mLastASamplePos = 0;
    bool               mHaveLastA      = false;
    uint64_t           mCycleStartTick = 0;

    int beatPeriodSamples() const;
    double beatPeriodMs() const;
    void appendToRing(const Measurement &m);
    QVector<float> extractBeatCyclePcm();

    static FilterStages computeFilterStages(const QVector<float> &pcm);
    void stylePanel(FilterPanel &panel, bool showXLabel);
    void drawPanel(FilterPanel &panel, int mode,
                   const QVector<double> &xs, const QVector<double> &ys,
                   const Measurement &m, double cycleMs);
    void redraw();
};
