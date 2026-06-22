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
#include "Measurement.h"

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

enum class AxisStatus {
    Unknown,
    PassExcellent,
    PassGood,
    Fail
};

/* Traffic-light color for a diagnosis level, for UI display
 * (e.g. QLabel background). Gray for Unknown (not enough data yet). */
QColor DiagnosisColor(DiagnosisLevel level);

struct DiagnosisInput {
    WatchMetrics metrics;
    WatchType    watch_type = WatchType::Men;
    bool         noSignal   = false;
};

struct AxisBreakdown {
    AxisStatus rate      = AxisStatus::Unknown;
    AxisStatus amplitude = AxisStatus::Unknown;
    AxisStatus beatError = AxisStatus::Unknown;
};

struct DiagnosisResult {
    DiagnosisLevel level = DiagnosisLevel::Unknown;
    QString        label;
    AxisBreakdown  breakdown;
};

class WatchDiagnostics
{
public:
    DiagnosisResult Evaluate(const DiagnosisInput &input) const;
};

QString formatAxisStatus(AxisStatus status);
QString formatDiagnosisTooltip(const DiagnosisResult &result,
                               const DiagnosisInput  &input);
QString formatBreakdownTableHtml(const DiagnosisResult &result,
                                 const DiagnosisInput  &input);
QString formatBreakdownCollapsedSummary(const DiagnosisResult &result,
                                        const DiagnosisInput  &input);

#endif /* WATCHDIAGNOSTICS_H */
