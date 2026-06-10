#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QVector>

// Long-Term Performance Graph (Spec p.19, Figure 14)
//
// Three stacked plots: rate (s/day) / amplitude (°) / beat error (ms).
// AllTime policy: shows all accumulated data from session start.
//
// DownsamplingBuffer: time-bucket averaging that coarsens automatically
// as elapsed time grows, keeping the plot readable without unbounded memory.
//   elapsed < 10 min  → 1 s buckets
//   elapsed < 1 hr    → 10 s buckets
//   elapsed >= 1 hr   → 60 s buckets
class LongTermTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit LongTermTab(QWidget *parent = nullptr);
    void reset() override;
    QCustomPlot *ratePlot()  const { return mRatePlot; }
    QCustomPlot *ampPlot()   const { return mAmpPlot; }
    QCustomPlot *beatPlot()  const { return mBeatPlot; }
public slots:
    void onMeasurement(const Measurement &m) override;

private:
    // ── DownsamplingBuffer ────────────────────────────────────
    struct DownsampledSeries {
        QVector<double> xs, ys;
        double bucketSum   = 0.0;
        int    bucketN     = 0;
        double bucketStart = -1.0;

        static double interval(double elapsed) {
            if (elapsed < 600.0)  return 1.0;
            if (elapsed < 3600.0) return 10.0;
            return 60.0;
        }

        // Feed a new (t, v) sample. Returns true when a bucket is committed.
        bool feed(double t, double v) {
            double iv = interval(t);
            if (bucketStart < 0) bucketStart = t;

            if (t - bucketStart >= iv && bucketN > 0) {
                xs.append(bucketStart + iv / 2.0);
                ys.append(bucketSum / bucketN);
                bucketSum   = v;
                bucketN     = 1;
                bucketStart = t;
                return true;
            }
            bucketSum += v;
            bucketN++;
            return false;
        }

        void clear() {
            xs.clear(); ys.clear();
            bucketSum = 0.0; bucketN = 0; bucketStart = -1.0;
        }

        void applyTo(QCustomPlot *plot, int graphIdx) const {
            plot->graph(graphIdx)->setData(xs, ys, true);
        }
    };

    // ── plots ─────────────────────────────────────────────────
    QCustomPlot *mRatePlot  = nullptr;
    QCustomPlot *mAmpPlot   = nullptr;
    QCustomPlot *mBeatPlot  = nullptr;

    // ── series ────────────────────────────────────────────────
    DownsampledSeries mRateSeries;
    DownsampledSeries mAmpSeries;
    DownsampledSeries mBeatSeries;

    double mTimeElapsed = 0.0;
};
