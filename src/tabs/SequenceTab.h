#pragma once
#include "BaseGraphTab.h"
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QDateTime>

class RadarChartTab;   // embedded on the right (forward decl — avoids include cycle)

// Graph 6: Multi-Position Sequence Display
// (Witschi Chronoscope X1 G3 manual p.15, project plan Figure 10).
//
// Captures Rate / Beat error / Amplitude for up to 10 watch test positions
// (NIHS 95-10/ISO 3158: CH, CB, 9H, 6H, 3H, 12H, ...) into one sequence table,
// plus the summary rows
//   X  = mean of all captured positions
//   D  = max - min across captured positions
// and a vertical-vs-horizontal comparison to reveal balance-wheel unbalance.
class SequenceTab : public BaseGraphTab
{
    Q_OBJECT
public:
    // One captured row, indexed 1:1 with positions(). valid=false until captured.
    struct PositionReading {
        bool   valid = false;
        double rate  = 0.0;   // s/d
        double beat  = 0.0;   // ms
        double amp   = 0.0;   // degrees
    };

    explicit SequenceTab(QWidget *parent = nullptr);
    void reset() override;

    static QStringList positions();          // standard position order
    QString activePosition() const { return mActivePosition; }
    void captureCurrent();                   // record latest averages at active position

    // Per-position captured readings (for the Radar/Polar view). Index matches
    // positions(); entries with valid==false have not been captured yet.
    QVector<PositionReading> capturedReadings() const;

    RadarChartTab *radar() const { return mRadar; }   // embedded radar (right pane)

public slots:
    void onMeasurement(const Measurement &m) override;
    void setActivePosition(const QString &pos);

public:
    // Auto H↔V position detection (demo): driven by MainWindow from amplitude.
    void selectPosition(const QString &pos);   // programmatic combo selection
    bool autoPosition() const;                 // is the "Auto H↔V" checkbox on?

signals:
    void positionChanged(const QString &pos); // notify MainWindow for Results label
    void sequenceUpdated();                    // captured/cleared — Radar view rebuilds

private:
    void recomputeSummary();
    int  rowOfPosition(const QString &pos) const;

    QComboBox     *mPositionCombo;
    QCheckBox     *mAutoPosCheck = nullptr;
    QLabel        *mHeaderLabel;
    QPushButton   *mCaptureButton;
    QTableWidget  *mTable;
    RadarChartTab *mRadar = nullptr;   // right pane of the split view

    Measurement   mLatest;
    bool          mHaveLatest     = false;
    QString       mActivePosition = "CH";
    QDateTime     mCapturedAt[10];
};
