#include "WatchDiagnostics.h"
#include <cmath>

DiagnosisResult WatchDiagnostics::Evaluate(const DiagnosisInput &input) const
{
    DiagnosisResult result;

    if (!input.rate_valid || !input.amplitude_valid || !input.beat_error_valid)
    {
        result.level = DiagnosisLevel::Unknown;
        result.label = "DIAGNOSIS: Unknown";
        return result;
    }

    const double rate           = input.rate_spd;
    const double amplitude      = input.amplitude_deg;
    const double beat_error     = input.beat_error_ms;

    /* Excellent rate band: Witschi Training Course p.15. Only the rate
     * column differs between watch types in that table (amplitude and
     * beat error are identical for every row), so only rate is
     * type-dependent here:
     *   Gent's watch  -5..+15 s/d
     *   Lady's watch  -5..+25 s/d
     * Good rate band is project-defined (not in Witschi); the Lady's
     * upper bound is widened by the same +10 s/d Witschi gives its
     * Excellent band, to keep the two tiers proportional per type. */
    const double excellentRateMax = (input.watch_type == WatchType::Women) ? 25.0 : 15.0;
    const double goodRateMax      = (input.watch_type == WatchType::Women) ? 20.0 : 10.0;

    /* Excellent band: amplitude threshold is the project's own "strong"
     * band (overview.md); Witschi's H/V split isn't modeled here since
     * we don't track test position. */
    if (rate >= -5.0 && rate <= excellentRateMax && amplitude >= 270.0 && beat_error <= 0.5)
    {
        result.level = DiagnosisLevel::Excellent;
        result.label = "DIAGNOSIS: Excellent";
    }
    /* Good band: project-defined middle tier (see docs/watch-diagnostics.md
     * for full source comparison). amplitude/beat_error chosen to sit
     * inside the "acceptable, but service coming" zone described by
     * independent collector/repair references rather than Witschi's
     * pass/fail-only table. */
    else if (rate >= -10.0 && rate <= goodRateMax && amplitude >= 220.0 && beat_error <= 0.8)
    {
        result.level = DiagnosisLevel::Good;
        result.label = "DIAGNOSIS: Good";
    }
    else
    {
        result.level = DiagnosisLevel::NeedsService;
        result.label = "DIAGNOSIS: Needs Service";
    }

    return result;
}

QColor DiagnosisColor(DiagnosisLevel level)
{
    switch (level)
    {
        case DiagnosisLevel::Excellent:    return QColor(0x2e, 0xa0, 0x4d);  /* green  */
        case DiagnosisLevel::Good:         return QColor(0xe0, 0xa8, 0x00);  /* amber  */
        case DiagnosisLevel::NeedsService: return QColor(0xc0, 0x30, 0x30);  /* red    */
        case DiagnosisLevel::Unknown:      return QColor(0x90, 0x90, 0x90);  /* gray   */
    }
    return QColor(0x90, 0x90, 0x90);
}
