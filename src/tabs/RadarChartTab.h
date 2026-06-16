#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include "SequenceTab.h"
#include <QComboBox>
#include <QLabel>

// Graph 14 (bonus): Radar / Polar chart of multi-position watch readings
// (inspired by the Witschi Chronoscope X1 "Polar display mode", project plan
// Test Positions section).
//
// Reads the per-position readings captured in SequenceTab and draws them as a
// polar polygon — one spoke per watch position (CH / 12H / 3H / 6H / 9H / CB …),
// radius = the selected metric (Rate / Amplitude / Beat error). The acceptable
// tolerance band is overlaid as reference rings and out-of-tolerance positions
// are highlighted, so positional variance (e.g. balance-wheel poising issues)
// is visible at a glance, and a verdict line summarises overall watch health.
//
// Event-driven: it does NOT process per-block Measurements (onMeasurement is a
// no-op) — it rebuilds only when SequenceTab emits sequenceUpdated(), and only
// while visible (R1 Lazy Rendering), so it adds no real-time cost.
class RadarChartTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit RadarChartTab(SequenceTab *sequence, QWidget *parent = nullptr);
    void reset() override;

    QCustomPlot        *plot() const { return mPlot; }
    QCPPolarGraph      *dataGraph() const { return mData; }
    QString             verdictText() const;   // for tests / inspection

public slots:
    void onMeasurement(const Measurement &) override {}  // no-op — event-driven
    void rebuild();                                      // on sequenceUpdated()

protected:
    void showEvent(QShowEvent *e) override;              // lazy: render when shown

private:
    enum Metric { Rate = 0, Amplitude = 1, BeatError = 2 };

    Metric currentMetric() const;
    void   configureRadialForMetric(Metric m);
    bool   isOutOfTolerance(Metric m, double v) const;
    double metricValue(Metric m, const SequenceTab::PositionReading &r) const;
    void   toleranceBounds(Metric m, double &lo, double &hi, bool &hasLo, bool &hasHi) const;

    SequenceTab        *mSeq;
    QCustomPlot        *mPlot;
    QCPPolarAxisAngular*mAngular  = nullptr;
    QCPPolarAxisRadial *mRadial   = nullptr;
    QCPPolarGraph      *mData     = nullptr;   // measured polygon
    QCPPolarGraph      *mBad      = nullptr;   // out-of-tolerance markers
    QCPPolarGraph      *mRefLo    = nullptr;   // tolerance ring (lower)
    QCPPolarGraph      *mRefHi    = nullptr;   // tolerance ring (upper)
    QComboBox          *mMetricCombo = nullptr;
    QLabel             *mVerdictLabel = nullptr;

    QString             mVerdict;
    bool                mDirty = true;         // needs rebuild when next shown
};
