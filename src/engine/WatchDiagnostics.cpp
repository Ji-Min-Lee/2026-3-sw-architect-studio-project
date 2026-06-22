#include "WatchDiagnostics.h"
#include <cmath>

namespace {

AxisStatus classifyRate(double rate, WatchType watchType)
{
    const double excellentMax = (watchType == WatchType::Women) ? 25.0 : 15.0;
    const double goodMax      = (watchType == WatchType::Women) ? 20.0 : 10.0;

    if (rate >= -5.0 && rate <= excellentMax)
        return AxisStatus::PassExcellent;
    if (rate >= -10.0 && rate <= goodMax)
        return AxisStatus::PassGood;
    return AxisStatus::Fail;
}

AxisStatus classifyAmplitude(double amplitude)
{
    if (amplitude >= 270.0)
        return AxisStatus::PassExcellent;
    if (amplitude >= 220.0)
        return AxisStatus::PassGood;
    return AxisStatus::Fail;
}

AxisStatus classifyBeatError(double beatError)
{
    if (beatError <= 0.5)
        return AxisStatus::PassExcellent;
    if (beatError <= 0.8)
        return AxisStatus::PassGood;
    return AxisStatus::Fail;
}

bool atLeastGood(AxisStatus status)
{
    return status == AxisStatus::PassGood || status == AxisStatus::PassExcellent;
}

bool isExcellent(AxisStatus status)
{
    return status == AxisStatus::PassExcellent;
}

QString valueOrDash(const std::optional<double> &value, int precision, const QString &suffix)
{
    if (!value)
        return QStringLiteral("—");
    return QString::number(*value, 'f', precision) + suffix;
}

} // namespace

DiagnosisResult WatchDiagnostics::Evaluate(const DiagnosisInput &input) const
{
    DiagnosisResult result;
    AxisBreakdown  &bd = result.breakdown;

    if (!input.metrics.rate)
        bd.rate = AxisStatus::Unknown;
    else
        bd.rate = classifyRate(*input.metrics.rate, input.watch_type);

    if (!input.metrics.amplitude)
        bd.amplitude = AxisStatus::Unknown;
    else
        bd.amplitude = classifyAmplitude(*input.metrics.amplitude);

    if (!input.metrics.beatError)
        bd.beatError = AxisStatus::Unknown;
    else
        bd.beatError = classifyBeatError(*input.metrics.beatError);

    if (bd.rate == AxisStatus::Unknown ||
        bd.amplitude == AxisStatus::Unknown ||
        bd.beatError == AxisStatus::Unknown)
    {
        result.level = DiagnosisLevel::Unknown;
        result.label = QStringLiteral("DIAGNOSIS: Unknown");
        return result;
    }

    if (isExcellent(bd.rate) && isExcellent(bd.amplitude) && isExcellent(bd.beatError))
    {
        result.level = DiagnosisLevel::Excellent;
        result.label = QStringLiteral("DIAGNOSIS: Excellent");
    }
    else if (atLeastGood(bd.rate) && atLeastGood(bd.amplitude) && atLeastGood(bd.beatError))
    {
        result.level = DiagnosisLevel::Good;
        result.label = QStringLiteral("DIAGNOSIS: Good");
    }
    else
    {
        result.level = DiagnosisLevel::NeedsService;
        result.label = QStringLiteral("DIAGNOSIS: Needs Service");
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

QString formatAxisStatus(AxisStatus status)
{
    switch (status) {
    case AxisStatus::PassExcellent: return QStringLiteral("Excellent");
    case AxisStatus::PassGood:      return QStringLiteral("Good");
    case AxisStatus::Fail:          return QStringLiteral("Fail");
    case AxisStatus::Unknown:       return QStringLiteral("Waiting");
    }
    return QStringLiteral("—");
}

QString formatDiagnosisTooltip(const DiagnosisResult &result,
                             const DiagnosisInput  &input)
{
    const QString watchType = (input.watch_type == WatchType::Women)
                                  ? QStringLiteral("Women")
                                  : QStringLiteral("Men");

    auto row = [&](const QString &axis, const QString &value, AxisStatus status) {
        QString mark;
        QString label = formatAxisStatus(status);
        QString color;
        switch (status) {
        case AxisStatus::PassExcellent:
            mark = QStringLiteral("✓"); color = QStringLiteral("#2ea04d"); break;
        case AxisStatus::PassGood:
            mark = QStringLiteral("✓"); color = QStringLiteral("#e0a000"); break;
        case AxisStatus::Fail:
            mark = QStringLiteral("✗"); color = QStringLiteral("#c03030"); break;
        case AxisStatus::Unknown:
            mark = QStringLiteral("…"); color = QStringLiteral("#888888"); break;
        }
        return QStringLiteral(
            "<tr>"
            "<td style='padding-right:10px;'><b>%1</b></td>"
            "<td align='right' style='padding-right:14px;font-family:monospace;'>%2</td>"
            "<td style='color:%3;'>%4 %5</td>"
            "</tr>")
            .arg(axis.toHtmlEscaped(), value.toHtmlEscaped(), color, mark, label);
    };

    QString html = QStringLiteral(
        "<html><body style='margin:0;'>"
        "<p style='margin:0 0 6px 0;'><b>Diagnosis breakdown</b> "
        "<span style='color:#666;'>(%1 watch)</span></p>"
        "<table cellspacing='0' cellpadding='2'>"
        "%2%3%4"
        "</table>")
        .arg(watchType.toHtmlEscaped(),
             row(QStringLiteral("Rate"),
                 valueOrDash(input.metrics.rate, 1, QStringLiteral(" s/d")),
                 result.breakdown.rate),
             row(QStringLiteral("Amplitude"),
                 valueOrDash(input.metrics.amplitude, 0, QStringLiteral("°")),
                 result.breakdown.amplitude),
             row(QStringLiteral("Beat Error"),
                 valueOrDash(input.metrics.beatError, 2, QStringLiteral(" ms")),
                 result.breakdown.beatError));

    if (result.level == DiagnosisLevel::Unknown) {
        QStringList waiting;
        if (result.breakdown.rate == AxisStatus::Unknown)
            waiting << QStringLiteral("Rate");
        if (result.breakdown.amplitude == AxisStatus::Unknown)
            waiting << QStringLiteral("Amplitude");
        if (result.breakdown.beatError == AxisStatus::Unknown)
            waiting << QStringLiteral("Beat Error");
        if (!waiting.isEmpty())
            html += QStringLiteral(
                "<p style='margin:8px 0 0 0;color:#666;'>Waiting for: %1</p>")
                        .arg(waiting.join(QStringLiteral(", ")).toHtmlEscaped());
    } else {
        QStringList fails;
        if (result.breakdown.rate == AxisStatus::Fail)
            fails << QStringLiteral("Rate");
        if (result.breakdown.amplitude == AxisStatus::Fail)
            fails << QStringLiteral("Amplitude");
        if (result.breakdown.beatError == AxisStatus::Fail)
            fails << QStringLiteral("Beat Error");
        if (!fails.isEmpty())
            html += QStringLiteral(
                "<p style='margin:8px 0 0 0;color:#c03030;'>Below Good band: %1</p>")
                        .arg(fails.join(QStringLiteral(", ")).toHtmlEscaped());
    }

    if (input.noSignal)
        html += QStringLiteral(
            "<p style='margin:8px 0 0 0;color:#c03030;'>"
            "No A-event for &gt;= 3 s — check mic / watch contact</p>");

    html += QStringLiteral("</body></html>");
    return html;
}

QString formatBreakdownTableHtml(const DiagnosisResult &result,
                                  const DiagnosisInput  &input)
{
    auto cellColor = [](AxisStatus status) -> QString {
        switch (status) {
        case AxisStatus::PassExcellent: return QStringLiteral("#2ea04d");
        case AxisStatus::PassGood:      return QStringLiteral("#e0a000");
        case AxisStatus::Fail:          return QStringLiteral("#c03030");
        case AxisStatus::Unknown:       return QStringLiteral("#888888");
        }
        return QStringLiteral("#888888");
    };

    auto bandLabel = [](AxisStatus status) -> QString {
        const QString text = formatAxisStatus(status);
        switch (status) {
        case AxisStatus::PassExcellent:
        case AxisStatus::PassGood:
            return QStringLiteral("✓ %1").arg(text);
        case AxisStatus::Fail:
            return QStringLiteral("✗ %1").arg(text);
        case AxisStatus::Unknown:
            return QStringLiteral("… %1").arg(text);
        }
        return text;
    };

    auto rowWithBand = [&](const QString &axis, const QString &value, AxisStatus status) {
        return QStringLiteral(
            "<tr style='color:#222;'>"
            "<td><b>%1</b></td>"
            "<td align='right' style='color:#111;font-family:monospace;'><b>%2</b></td>"
            "<td align='right' style='color:%3;'><b>%4</b></td>"
            "</tr>")
            .arg(axis, value, cellColor(status), bandLabel(status));
    };

    return QStringLiteral(
        "<html><body style='color:#222;margin:0;padding:0 0 4px 0;'>"
        "<table cellspacing='4' cellpadding='4' width='100%' style='color:#222;'>"
        "<tr style='color:#666;'><td><b>Axis</b></td><td align='right'><b>Value</b></td>"
        "<td align='right'><b>Band</b></td></tr>"
        "%1%2%3"
        "</table></body></html>")
        .arg(rowWithBand(QStringLiteral("Rate"),
                         valueOrDash(input.metrics.rate, 1, QStringLiteral(" s/d")),
                         result.breakdown.rate),
             rowWithBand(QStringLiteral("Amplitude"),
                         valueOrDash(input.metrics.amplitude, 0, QStringLiteral("°")),
                         result.breakdown.amplitude),
             rowWithBand(QStringLiteral("Beat Error"),
                         valueOrDash(input.metrics.beatError, 2, QStringLiteral(" ms")),
                         result.breakdown.beatError));
}

QString formatBreakdownCollapsedSummary(const DiagnosisResult &result,
                                        const DiagnosisInput  &input)
{
    Q_UNUSED(result);
    return QStringLiteral("Rate %1 · Amplitude %2 · Beat Error %3")
        .arg(valueOrDash(input.metrics.rate, 1, QStringLiteral(" s/d")),
             valueOrDash(input.metrics.amplitude, 0, QStringLiteral("°")),
             valueOrDash(input.metrics.beatError, 2, QStringLiteral(" ms")));
}
