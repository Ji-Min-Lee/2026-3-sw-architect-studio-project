#pragma once
// R1 Lazy Rendering — per-beat replot call counter.
//
// Tabs increment this whenever they actually call replot().
// MainWindow reads and resets it in onMeasurementReady() and passes the
// value to the Logger so the CSV shows replot_count per beat.
//
// Thread model: incremented on UI thread, read+reset on UI thread (both in
// onMeasurementReady). No cross-thread race.
//
#include <atomic>
inline std::atomic<int> g_replotCount{0};
