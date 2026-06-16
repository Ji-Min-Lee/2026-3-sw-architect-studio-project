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
#include <cstdint>
#include "Logger.h"   // TG_NOW(), nowUs()
inline std::atomic<int>     g_replotCount{0};
inline std::atomic<int64_t> g_plotUs{0};     // accumulated replot() wall time (us) per beat
