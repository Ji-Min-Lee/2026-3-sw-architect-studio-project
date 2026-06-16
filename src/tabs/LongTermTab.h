#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QLabel>

// Graph 8: Long-Term Performance Graph (project plan Figure 14).
//
// Three stacked series recorded over an extended period:
//   rate (s/d), amplitude (°), beat error (ms)
// Each series shows its running average (dotted line) and a ±σ typical-
// variation band. To stay readable and efficient over many hours, the
// update granularity is reduced as elapsed time grows (points are
// aggregated into progressively larger buckets).
class LongTermTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit LongTermTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
    void replotAll() override;
private:
    struct Series {
        QCPGraph    *graph    = nullptr;
        QCPItemLine *meanLine = nullptr;
        QCPItemRect *band     = nullptr;
        QCPAxisRect *rect     = nullptr;
        double sum = 0, sumSq = 0; quint64 n = 0;
        // bucket aggregation
        double bucketSum = 0; int bucketN = 0;
        void addRunning(double v) { sum += v; sumSq += v * v; n++; }
        double mean()  const { return n ? sum / n : 0.0; }
        double sigma() const {
            if (n < 2) return 0.0;
            double var = (sumSq - sum * sum / n) / (n - 1);
            return var > 0 ? std::sqrt(var) : 0.0;
        }
    };
    Series makeSeries(int row, const QString &name, const QColor &c,
                      bool firstUsesDefaultRect);
    void   addTolLine(Series &series, double yVal);
    void   addPoint(Series &series, double timeSec, double value);
    void   updateOverlay(Series &series);

    QCustomPlot *mPlot;
    QLabel      *mSummaryLabel = nullptr;
    Series mRate, mAmp, mBeat;
    double mTimeElapsed = 0.0;
    int    mBucketSize  = 1;     // measurements per plotted point
};
