#pragma once
#include <QWidget>
#include "Measurement.h"

// MVC: Passive View interface  |  Observer: ConcreteObserver
//
// All 11 graph tabs implement this interface.
// Adding a new graph = NewTab.h + NewTab.cpp + 3 lines in MainWindow.cpp (AP-3).
class BaseGraphTab : public QWidget
{
    Q_OBJECT
public:
    explicit BaseGraphTab(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual void reset() = 0;

public slots:
    virtual void onMeasurement(const Measurement &m) = 0;
};
