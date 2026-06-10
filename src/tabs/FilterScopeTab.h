#pragma once
#include "BaseGraphTab.h"
#include "qcustomplot.h"
#include <QComboBox>
#include <QLabel>

// Graph 13 (NEW): Scope Function with Multiple Filter Views — F0..F3
// (project plan Figure 19, after PC-RM4 / pascalchour chour_rm4).
//
//   F0 — signal as captured, mirrored around its average value
//   F1 — moving-average filter of F0 (smoothed envelope)
//   F2 — F1 with rising slopes emphasized, falling slopes attenuated
//        (makes T3, and to some extent T2, stand out)
//   F3 — upper portion of the signal only, rising-edge emphasis
// A/C event markers stay overlaid in every view so the user can compare how
// each filter changes the visibility of T1 / T2 / T3.
class FilterScopeTab : public BaseGraphTab
{
    Q_OBJECT
public:
    explicit FilterScopeTab(QWidget *parent = nullptr);
    void reset() override;
public slots:
    void onMeasurement(const Measurement &m) override;
private:
    QVector<double> applyFilter(const QVector<float> &raw) const;
    void redraw();

    QComboBox   *mFilterCombo;
    QLabel      *mHintLabel;
    QCustomPlot *mPlot;
    QList<QCPItemLine *> mMarkers;

    Measurement mLatest;
    bool        mHaveData = false;
};
