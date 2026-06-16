#pragma once

// Pure calculation functions extracted from MeasurementEngine.
// No Qt, no DSP dependencies — directly unit-testable.
namespace WatchMath {

// Beat error magnitude in milliseconds.
// t0, t1, t2: three consecutive A-event sample positions.
// fs: sample rate in Hz.
// Formula: |(t1-t0 - (t2-t1)| / 2 * 1000
double beatErrorMs(double t0, double t1, double t2, int fs);

// Amplitude in degrees using the exact trigonometric formula.
// aPos, cPos: A and C event sample positions (same beat packet).
// fs: sample rate in Hz.
// liftAngle: lift angle in degrees.
// bph: beats per hour.
// Returns < 0 if result is invalid (>= 360°) or input is degenerate.
double amplitudeDeg(double aPos, double cPos, int fs, double liftAngle, int bph);

// Escapement interval from A to C event in milliseconds.
// aPos, cPos: sample positions.
// fs: sample rate in Hz.
double escapementMs(double aPos, double cPos, int fs);

// Wrap value into [lo, hi) with modulo arithmetic.
double wrapInRange(double value, double lo, double hi);

// Normalize a rate error relative to the first observed error (zero-offset anchoring).
// firstError: instErrorMs of beat 0 (the anchor).
// currentError: instErrorMs of beat n.
// Returns currentError - firstError so beat 0 always plots at 0.
double applyZeroOffset(double firstError, double currentError);

// Instantaneous rate error in seconds for beat n.
// Formula (Part I): En = T_measured - (T_start + n * I_target)
// timeMeasured: absolute timestamp of beat n in seconds.
// tStart: timestamp of beat 0 in seconds.
// beatIndex: beat index (0, 1, 2, ...).
// bph: beats per hour (used to derive I_target = 3600 / BPH).
double instErrorSec(double timeMeasured, double tStart, int beatIndex, int bph);

// Nominal half-beat interval in seconds.
// I_target = 3600 / BPH
double halfBeatInterval(int bph);

// Rate in seconds/day from same-phase tic and tac periods.
// Formula (Part II): Rate = (rate_tic + rate_tac) / 2
//   rate_tic = 86400 * (T_nom / T_tic - 1)
//   rate_tac = 86400 * (T_nom / T_tac - 1)
//   T_nom = 7200 / BPH  (same-phase nominal period)
// tTicSec, tTacSec: measured same-phase periods in seconds.
// bph: beats per hour.
double rateSpdFromPhase(double tTicSec, double tTacSec, int bph);

} // namespace WatchMath
