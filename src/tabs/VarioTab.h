#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QLabel>
#include <cmath>

// Graph 5: Vario Display — long-term stability statistics of Rate and Amplitude
// (Witschi Chronoscope X1 G3 manual p.15, project plan Figure 9).
//
// For each of Rate and Amplitude this view continuously updates
//   Min / X̄ (mean) / σ (std-dev) / Max and the elapsed measurement time,
// and renders a horizontal value scale with
//   - green band  : acceptable range
//   - blue arrows : measured min / max
//   - red arrow   : mean
class VarioTab : public BaseGraphTab
{
    Q_OBJECT
public:
    struct Stats {
        quint64 n = 0;
        double  min = 0, max = 0, sum = 0, sumSq = 0;
        double  mean()  const { return n ? sum / n : 0.0; }
        double  sigma() const {
            if (n < 2) return 0.0;
            double v = (sumSq - sum * sum / n) / (n - 1);
            return v > 0 ? std::sqrt(v) : 0.0;
        }
        void add(double v) {
            if (!n) { min = max = v; }
            min = qMin(min, v); max = qMax(max, v);
            sum += v; sumSq += v * v; n++;
        }
    };

    explicit VarioTab(QWidget *parent = nullptr);
    void reset() override;

    QCustomPlot *plot() const { return mRateScale.plot; } // rate scale plot
    const Stats &rateStats() const { return mRate; }
    const Stats &ampStats()  const { return mAmp; }
    double elapsedSec() const { return mElapsedSec; }

public slots:
    void onMeasurement(const Measurement &m) override;

private:
    struct Scale {
        QCustomPlot *plot      = nullptr;
        QCPItemLine *minArrow  = nullptr;
        QCPItemLine *maxArrow  = nullptr;
        QCPItemLine *meanArrow = nullptr;
        QLabel      *label     = nullptr;
    };
    Scale makeScale(double lo, double hi, double bandLo, double bandHi);
    void  updateScale(Scale &s, const Stats &st, const QString &name,
                      const QString &unit, int decimals);
    static QCPItemLine *makeArrow(QCustomPlot *p, const QColor &c, double width);

    QLabel *mElapsedLabel;
    Scale   mRateScale, mAmpScale;
    Stats   mRate, mAmp;
    double  mElapsedSec = 0.0;
};
