#pragma once
#include <QWidget>
#include "Measurement.h"

// MVC: Passive View interface  |  Observer: ConcreteObserver
//
// All graph tabs implement this interface.
// Adding a new graph = NewTab.h + NewTab.cpp + 3 lines in MainWindow.cpp (AP-3).
//
// Pause contract: while paused, tabs keep accumulating incoming Measurement
// data but must not rescale axes or replot — the frozen view lets the user
// drag/zoom (cursor navigation) through recorded data without losing it.
class BaseGraphTab : public QWidget
{
    Q_OBJECT
public:
    explicit BaseGraphTab(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual void reset() = 0;

    void setPaused(bool p) { mPaused = p; }
    bool isPaused() const  { return mPaused; }

public slots:
    virtual void onMeasurement(const Measurement &m) = 0;

protected:
    bool mPaused = false;
};
