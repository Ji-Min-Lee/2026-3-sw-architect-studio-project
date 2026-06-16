#pragma once
#include <QWidget>
#include <QShowEvent>
#include <QTimer>
#include "Measurement.h"

// MVC: Passive View interface  |  Observer: ConcreteObserver
//
// All graph tabs implement this interface.
// Adding a new graph = NewTab.h + NewTab.cpp + 3 lines in MainWindow.cpp (AP-3).
//
// Pause contract: while paused, tabs keep accumulating incoming Measurement
// data but must not rescale axes or replot — the frozen view lets the user
// drag/zoom (cursor navigation) through recorded data without losing it.
//
// R1 Lazy Rendering contract:
//   onMeasurement() accumulates data always, but skips replot() when !isVisible().
//   showEvent() calls replotAll() so the tab catches up on the latest snapshot
//   when it becomes visible (e.g. user switches tab).
class BaseGraphTab : public QWidget
{
    Q_OBJECT
public:
    explicit BaseGraphTab(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual void reset() = 0;

    void setPaused(bool p) { mPaused = p; }
    bool isPaused() const  { return mPaused; }

    // R1: subclasses override to replot all their QCustomPlot instances.
    // Called by showEvent() to refresh the view when the tab becomes visible.
    virtual void replotAll() {}

public slots:
    virtual void onMeasurement(const Measurement &m) = 0;

protected:
    void showEvent(QShowEvent *e) override
    {
        QWidget::showEvent(e);
        // Defer replot to avoid blocking the show event — especially important
        // at startup when all tabs get showEvent simultaneously.
        if (!mPaused)
            QTimer::singleShot(0, this, &BaseGraphTab::replotAll);
    }

    bool mPaused = false;
};
