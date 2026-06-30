#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QStackedWidget>
#include <QPainter>
#include <QMouseEvent>
#include <algorithm>

// Lightweight thumbnail strip bar shown below the Scope 1 main plot.
// Each strip renders one captured beat waveform; clicking selects it.
class BeatStripBar : public QWidget {
    Q_OBJECT
public:
    static constexpr int kStripW = 60;
    static constexpr int kStripH = 52;
    static constexpr int kGap    = 4;
    static constexpr int kPad    = 4;

    explicit BeatStripBar(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedHeight(kStripH + kPad * 2);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setCursor(Qt::PointingHandCursor);
    }

    void setStrips(const QList<QVector<double>> &strips, int selected) {
        mStrips   = strips;
        mSelected = selected;
        update();
    }

signals:
    void beatSelected(int index);

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor(30, 30, 30));

        for (int i = 0; i < mStrips.size(); i++) {
            int x = kPad + i * (kStripW + kGap);
            QRect r(x, kPad, kStripW, kStripH);

            p.fillRect(r, i == mSelected ? QColor(55, 55, 75) : QColor(42, 42, 42));
            p.setPen(i == mSelected ? QPen(QColor(255, 220, 50), 1.5)
                                    : QPen(QColor(80, 80, 80), 1));
            p.drawRect(r);

            const QVector<double> &ys = mStrips[i];
            if (ys.isEmpty()) continue;
            double ymax = *std::max_element(ys.begin(), ys.end());
            if (ymax < 1e-9) continue;

            p.setPen(QPen(QColor(240, 200, 60), 1));
            QPolygonF poly;
            poly.reserve(ys.size());
            for (int k = 0; k < ys.size(); k++) {
                double px = r.left() + 2 + (double)k / ys.size() * (kStripW - 4);
                double py = r.bottom() - 2 - (ys[k] / ymax) * (kStripH - 4);
                poly << QPointF(px, py);
            }
            p.drawPolyline(poly);

            // Beat index label
            p.setPen(QColor(160, 160, 160));
            p.setFont(QFont("sans", 7));
            p.drawText(r.adjusted(2, 2, -2, -2), Qt::AlignTop | Qt::AlignLeft,
                       i == 0 ? "now" : QString("-%1").arg(i));
        }
    }

    void mousePressEvent(QMouseEvent *ev) override {
        int i = (qRound(ev->position().x()) - kPad) / (kStripW + kGap);
        if (i >= 0 && i < mStrips.size())
            emit beatSelected(i);
    }

private:
    QList<QVector<double>> mStrips;
    int mSelected = 0;
};

// Graph 7: Beat-Noise Scope Display — Scope 1 & Scope 2
// (Witschi Chronoscope X1 G3 manual p.19, project plan Figure 11).
//
// Scope 1: waveform of the alternating tick/tock beat noises with a
//   selectable time range (20 / 200 / 400 ms), |signal| display, A (green)
//   and C (red) markers, lift angle readout, a selector to bring one of
//   the last 10 captured beats back for enlarged inspection, and a strip
//   bar beneath the main plot showing thumbnail waveforms of recent beats.
//
// Scope 2: tic and tac noises on two horizontal axes over a fixed 20 ms
//   range, with a Σ control that averages up to 50 tic and 50 tac noises to
//   reduce random noise; the average amplitude of each trace is reported.
class BeatNoiseScopeTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit BeatNoiseScopeTab(QWidget *parent = nullptr);
    void reset() override;
    void setLiftAngle(double deg);

    int capturedBeats() const { return mBeats.size(); }
    QCustomPlot *scope1Plot() const { return mPlot1; }

public slots:
    void onMeasurement(const Measurement &m) override;
    void replotAll() override;

private:
    struct Beat {
        QVector<double> ys;       // |processed pcm| window
        double startAbs = 0;      // absolute sample index of ys[0]
        double aPos = -1, cPos = -1;
        bool   isTic = true;
    };

    void appendSamples(const Measurement &m);
    void fulfillPending();
    void redrawScope1();
    void redrawScope2();
    int  rangeSamples() const;

    // Controls
    QComboBox *mViewCombo;
    QComboBox *mRangeCombo;
    QComboBox *mBeatCombo;
    QCheckBox *mAvgCheck;
    QLabel    *mInfoLabel;

    // Scope 1 plot (single rect) / Scope 2 plot (two stacked rects)
    QStackedWidget *mStack;
    QCustomPlot    *mPlot1;
    QCustomPlot    *mPlot2;
    QCPGraph       *mTicGraph2, *mTocGraph2;
    QCPItemLine    *mAMarker, *mCMarker;
    BeatStripBar   *mStripBar;

    // Rolling |processed| buffer (~1 s)
    QVector<double> mBuf;
    double          mBufStartAbs = 0;
    int             mSps = 48000;

    QList<Beat> mPending;       // waiting for enough samples
    QList<Beat> mBeats;         // fulfilled, newest first (max 10)
    int         mParity = 0;

    // Scope 2 averaging state (20 ms windows)
    QVector<double> mTicSum, mTocSum;
    int mTicCount = 0, mTocCount = 0;
    static constexpr int kAvgCycle = 50;

    double mLiftAngle = 52.0;
};
