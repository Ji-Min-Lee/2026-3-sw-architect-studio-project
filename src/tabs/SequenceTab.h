#pragma once
#include "BaseGraphTab.h"
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>

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
    explicit SequenceTab(QWidget *parent = nullptr);
    void reset() override;

    static QStringList positions();          // standard position order
    QString activePosition() const { return mActivePosition; }
    void captureCurrent();                   // record latest averages at active position

public slots:
    void onMeasurement(const Measurement &m) override;
    void setActivePosition(const QString &pos);

private:
    void recomputeSummary();
    int  rowOfPosition(const QString &pos) const;

    QLabel       *mHeaderLabel;
    QLabel       *mUnbalanceLabel;
    QPushButton  *mCaptureButton;
    QTableWidget *mTable;

    Measurement   mLatest;
    bool          mHaveLatest = false;
    QString       mActivePosition = "CH";
};
