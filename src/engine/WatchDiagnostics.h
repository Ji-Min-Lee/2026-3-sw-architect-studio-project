/* WatchDiagnostics.h - rule-based watch condition diagnosis.
 *
 * Consumes the Rate/Amplitude/Beat-Error measurements already computed
 * by MainWindow and produces a coarse condition label. This is step 1
 * of the AI feature track: a fixed-threshold stand-in for the model
 * that will later replace EvaluateThresholds() (see WatchClassifier).
 */
#ifndef WATCHDIAGNOSTICS_H
#define WATCHDIAGNOSTICS_H

#include <QString>
#include <QColor>

enum class DiagnosisLevel {
    Unknown,
    Excellent,
    Good,
    NeedsService
};

/* Witschi Training Course p.15 gives a per-watch-type rate band
 * (Gent's/Lady's/Chronometer/Chronograph); only Gent's and Lady's
 * are modeled here. Amplitude and beat-error bands are identical
 * across all four rows in that table, so WatchType only affects
 * the rate thresholds. */
enum class WatchType {
    Men,
    Women
};

/* Traffic-light color for a diagnosis level, for UI display
 * (e.g. QLabel background). Gray for Unknown (not enough data yet). */
QColor DiagnosisColor(DiagnosisLevel level);

struct DiagnosisInput {
    double    rate_spd;
    bool      rate_valid;
    double    amplitude_deg;
    bool      amplitude_valid;
    double    beat_error_ms;
    bool      beat_error_valid;
    WatchType watch_type = WatchType::Men;
};

struct DiagnosisResult {
    DiagnosisLevel level = DiagnosisLevel::Unknown;
    QString        label;
};

class WatchDiagnostics
{
public:
    DiagnosisResult Evaluate(const DiagnosisInput &input) const;
};

#endif /* WATCHDIAGNOSTICS_H */
