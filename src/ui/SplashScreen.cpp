#include "SplashScreen.h"
#include "AppInfo.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QtMath>
#include <cmath>

namespace {

constexpr int kWidth  = 760;
constexpr int kHeight = 460;

constexpr QColor kVoidTop(4, 7, 14);
constexpr QColor kVoidBottom(2, 4, 10);
constexpr QColor kCardFill(10, 16, 28, 235);
constexpr QColor kCardEdge(72, 118, 168, 140);
constexpr QColor kAccentCyan(77, 232, 255);
constexpr QColor kAccentViolet(123, 97, 255);
constexpr QColor kAccentGreen(74, 222, 128);
constexpr QColor kAccentBlue(99, 140, 255);
constexpr QColor kTextPrimary(236, 242, 250);
constexpr QColor kTextMuted(143, 163, 184);
constexpr QColor kLgRed(228, 0, 43);

qreal easeOutCubic(qreal t)
{
    const qreal u = 1.0 - t;
    return 1.0 - u * u * u;
}

qreal easeInOutCubic(qreal t)
{
    return t < 0.5 ? 4.0 * t * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 3.0) / 2.0;
}

qreal easeOutExpo(qreal t)
{
    return t >= 1.0 ? 1.0 : 1.0 - std::pow(2.0, -10.0 * t);
}

qreal clamp01(qreal v) { return qBound(0.0, v, 1.0); }

QColor withAlpha(const QColor &c, int alpha)
{
    QColor out(c);
    out.setAlpha(qBound(0, alpha, 255));
    return out;
}

void drawGlowRect(QPainter &p, const QRectF &rect, const QColor &color, int layers)
{
    for (int i = layers; i >= 1; --i) {
        const qreal pad = i * 1.6;
        p.setPen(QPen(withAlpha(color, color.alpha() / (i + 1)), 1.2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect.adjusted(-pad, -pad, pad, pad), 14, 14);
    }
}

// Beat positions (normalized 0-1 in the waveform) — interleaved A and C events
struct BeatEvent { qreal xNorm; bool isA; };
const BeatEvent kBeats[] = {
    {0.06f, true},  {0.19f, false}, {0.33f, true},  {0.46f, false},
    {0.60f, true},  {0.73f, false}, {0.87f, true},  {0.97f, false},
};
constexpr int kBeatCount = 8;

} // namespace

SplashScreen::SplashScreen(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(kWidth, kHeight);
    setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);

    m_lgLogo = QPixmap(QStringLiteral(":/images/lg_electronics_logo.png"));

    connect(&m_timer, &QTimer::timeout, this, &SplashScreen::onTick);
}

void SplashScreen::start()
{
    m_skipRequested = false;
    m_clock.start();
    m_timer.start(16);
    show();
    raise();
    activateWindow();
    setFocus();
}

bool SplashScreen::isAnimationComplete() const
{
    return m_skipRequested || m_clock.elapsed() >= kDurationMs;
}

void SplashScreen::requestSkip()
{
    m_skipRequested = true;
    update();
}

void SplashScreen::onTick()
{
    update();
    if (isAnimationComplete())
        m_timer.stop();
}

qreal SplashScreen::animationProgress() const
{
    if (m_skipRequested) return 1.0;
    return clamp01(qreal(m_clock.elapsed()) / qreal(kDurationMs));
}

QRectF SplashScreen::contentCard() const
{
    return QRectF(28, 28, width() - 56, height() - 56);
}

QFont SplashScreen::hudFont(int pixelSize, bool bold) const
{
    QFont f(QStringLiteral("Consolas"));
    if (!f.exactMatch()) f = QFont(QStringLiteral("Courier New"));
    f.setPixelSize(pixelSize);
    f.setBold(bold);
    f.setLetterSpacing(QFont::AbsoluteSpacing, 1.2);
    return f;
}

QFont SplashScreen::uiFont(int pixelSize, QFont::Weight weight) const
{
    QFont f(QStringLiteral("Segoe UI"));
    if (!f.exactMatch()) f = font();
    f.setPixelSize(pixelSize);
    f.setWeight(weight);
    return f;
}

void SplashScreen::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const qreal progress = animationProgress();
    const QRectF card = contentCard();

    drawBackdrop(p, progress);
    drawHudCard(p, progress);
    drawCornerBrackets(p, card, progress);
    drawScopePanel(p, card, progress);
    drawBranding(p, card, progress);
    drawHudReadout(p, card, progress);
    drawStatusBar(p, card, progress);
}

void SplashScreen::drawBackdrop(QPainter &p, qreal progress) const
{
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, kVoidTop);
    bg.setColorAt(1.0, kVoidBottom);
    p.fillRect(rect(), bg);

    // Pulsing radial halo
    const qreal pulse = 0.5 + 0.5 * std::sin(progress * 14.0);
    QRadialGradient halo(width() * 0.5, height() * 0.44, width() * (0.58 + 0.06 * pulse));
    halo.setColorAt(0.0, withAlpha(kAccentCyan, int(18 + 10 * pulse)));
    halo.setColorAt(0.35, withAlpha(kAccentViolet, int(8 + 6 * pulse)));
    halo.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.fillRect(rect(), halo);

    // Expanding ring animation (beat-synced)
    if (progress > 0.15) {
        const qreal ringT = std::fmod(progress * 3.5, 1.0);
        const qreal ringR = width() * 0.18 * ringT;
        const int ringAlpha = int(60 * (1.0 - ringT));
        QRadialGradient ring(width() * 0.5, height() * 0.44, ringR);
        ring.setColorAt(0.85, QColor(0, 0, 0, 0));
        ring.setColorAt(0.95, withAlpha(kAccentCyan, ringAlpha));
        ring.setColorAt(1.0, QColor(0, 0, 0, 0));
        p.fillRect(rect(), ring);
    }

    // Vignette
    QRadialGradient vignette(width() * 0.5, height() * 0.5, width() * 0.72);
    vignette.setColorAt(0.55, QColor(0, 0, 0, 0));
    vignette.setColorAt(1.0, QColor(0, 0, 0, 200));
    p.fillRect(rect(), vignette);

    // Horizontal scan lines
    const qreal scanOffset = std::fmod(progress * height() * 1.4, 6.0);
    p.setPen(QPen(QColor(255, 255, 255, 6), 1.0));
    for (qreal y = -scanOffset; y < height(); y += 6.0)
        p.drawLine(QPointF(0, y), QPointF(width(), y));

    // Outer border pulse
    const qreal borderPulse = 0.5 + 0.5 * std::sin(progress * 18.0);
    p.setPen(QPen(withAlpha(kAccentCyan, int(18 + 28 * borderPulse)), 1.0));
    p.drawRoundedRect(contentCard().adjusted(-1, -1, 1, 1), 16, 16);
}

void SplashScreen::drawHudCard(QPainter &p, qreal progress) const
{
    const QRectF card = contentCard();
    const qreal reveal = easeOutCubic(clamp01(progress / 0.35));

    QPainterPath cardPath;
    cardPath.addRoundedRect(card, 14, 14);

    QLinearGradient glass(card.topLeft(), card.bottomRight());
    glass.setColorAt(0.0, withAlpha(QColor(18, 28, 46), int(240 * reveal)));
    glass.setColorAt(0.55, withAlpha(kCardFill, int(235 * reveal)));
    glass.setColorAt(1.0, withAlpha(QColor(8, 12, 22), int(250 * reveal)));

    p.setOpacity(reveal);
    drawGlowRect(p, card, withAlpha(kAccentCyan, 55), 3);
    p.setPen(QPen(withAlpha(kCardEdge, 180), 1.2));
    p.setBrush(glass);
    p.drawPath(cardPath);

    QLinearGradient topSheen(card.left(), card.top(), card.left(), card.top() + 80);
    topSheen.setColorAt(0.0, QColor(255, 255, 255, 18));
    topSheen.setColorAt(1.0, QColor(255, 255, 255, 0));
    p.setPen(Qt::NoPen);
    p.setBrush(topSheen);
    p.drawPath(cardPath);

    p.setOpacity(1.0);
}

void SplashScreen::drawCornerBrackets(QPainter &p, const QRectF &card, qreal progress) const
{
    const qreal t = easeOutExpo(clamp01((progress - 0.05) / 0.30));
    if (t <= 0.0) return;

    const qreal len = 22.0 * t;
    const qreal inset = 10.0;
    const QRectF inner = card.adjusted(inset, inset, -inset, -inset);

    auto drawBracket = [&](qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3) {
        QPainterPath path;
        path.moveTo(x1, y1);
        path.lineTo(x2, y2);
        path.lineTo(x3, y3);
        p.drawPath(path);
    };

    p.setPen(QPen(withAlpha(kAccentCyan, int(170 * t)), 1.6, Qt::SolidLine, Qt::SquareCap));
    drawBracket(inner.left(), inner.top() + len, inner.left(), inner.top(), inner.left() + len, inner.top());
    drawBracket(inner.right() - len, inner.top(), inner.right(), inner.top(), inner.right(), inner.top() + len);
    drawBracket(inner.left(), inner.bottom() - len, inner.left(), inner.bottom(), inner.left() + len, inner.bottom());
    drawBracket(inner.right() - len, inner.bottom(), inner.right(), inner.bottom(), inner.right(), inner.bottom() - len);
}

void SplashScreen::drawScopePanel(QPainter &p, const QRectF &card, qreal progress) const
{
    const qreal expandPhase = clamp01((progress - 0.08) / 0.38);
    const qreal eased = easeOutCubic(expandPhase);

    const QRectF scopeArea(card.left() + 34, card.top() + 48, card.width() - 68, 130);

    const qreal startW = 36.0, startH = 10.0;
    const qreal panelW = startW + (scopeArea.width()  - startW) * eased;
    const qreal panelH = startH + (scopeArea.height() - startH) * eased;

    const QRectF panel(
        scopeArea.center().x() - panelW * 0.5,
        scopeArea.center().y() - panelH * 0.5,
        panelW, panelH);

    QPainterPath panelPath;
    panelPath.addRoundedRect(panel, 6, 6);

    QLinearGradient panelFill(panel.topLeft(), panel.bottomLeft());
    panelFill.setColorAt(0.0, QColor(5, 9, 16, 250));
    panelFill.setColorAt(1.0, QColor(3, 6, 12, 255));

    p.setPen(QPen(withAlpha(kAccentCyan, int(70 + 110 * eased)), 1.0));
    p.setBrush(panelFill);
    p.drawPath(panelPath);

    if (eased > 0.12) {
        const qreal gridAlpha = 28 * eased;
        p.setPen(QPen(withAlpha(kAccentCyan, int(gridAlpha)), 1.0, Qt::DotLine));
        for (int i = 1; i < 4; ++i) {
            const qreal y = panel.top() + panel.height() * qreal(i) / 4.0;
            p.drawLine(QPointF(panel.left() + 8, y), QPointF(panel.right() - 8, y));
        }
        p.drawLine(QPointF(panel.center().x(), panel.top() + 6),
                   QPointF(panel.center().x(), panel.bottom() - 6));
    }

    if (expandPhase > 0.10) {
        p.save();
        p.setClipPath(panelPath);
        drawWaveform(p, panel, progress);
        drawEventMarkers(p, panel, progress);
        p.restore();
    }

    if (eased > 0.20) {
        const qreal labelAlpha = clamp01((eased - 0.20) / 0.35);
        p.setFont(hudFont(9, true));
        p.setPen(withAlpha(kAccentCyan, int(200 * labelAlpha)));
        p.drawText(panel.adjusted(10, 7, -10, -7), Qt::AlignTop | Qt::AlignLeft,
                   QStringLiteral("WATCH AUDIO"));

        const qreal pulse = 0.45 + 0.55 * std::sin(progress * 20.0);
        p.setPen(Qt::NoPen);
        p.setBrush(withAlpha(kAccentGreen, int(220 * labelAlpha * pulse)));
        p.drawEllipse(QPointF(panel.right() - 16, panel.top() + 13), 3.5, 3.5);
    }
}

void SplashScreen::drawWaveform(QPainter &p, const QRectF &panel, qreal progress) const
{
    const qreal wavePhase = clamp01((progress - 0.12) / 0.70);
    if (wavePhase <= 0.0) return;

    const int sampleCount = qMax(48, int(panel.width()));
    const qreal midY = panel.center().y();
    const qreal amp = panel.height() * 0.34 * easeInOutCubic(wavePhase);
    const qreal time = progress * 10.0;

    auto sampleAt = [&](qreal xNorm) -> qreal {
        const qreal x = xNorm * sampleCount;
        const qreal carrier  = std::sin((x * 0.14) - time * 4.8) * 0.20;
        const qreal harmonic = std::sin((x * 0.27) - time * 2.9) * 0.09;
        const qreal shimmer  = std::sin((x * 0.61) + time * 5.5) * 0.03;

        qreal tick = 0.0;
        const qreal beatSpacing = 34.0;
        const qreal beatPos = std::fmod(x + time * 30.0, beatSpacing);
        if (beatPos < 2.0)
            tick = std::exp(-beatPos * beatPos * 1.1) * 1.0;
        else if (beatPos > beatSpacing - 2.4)
            tick = -std::exp(-std::pow(beatSpacing - beatPos, 2) * 0.8) * 0.62;

        return (carrier + harmonic + shimmer + tick) * amp;
    };

    QPainterPath trace, fill;
    fill.moveTo(panel.left(), panel.bottom());
    for (int i = 0; i <= sampleCount; ++i) {
        const qreal xNorm = qreal(i) / qreal(sampleCount);
        const qreal x = panel.left() + xNorm * panel.width();
        const qreal y = midY - sampleAt(xNorm);
        if (i == 0) { trace.moveTo(x, y); fill.lineTo(x, y); }
        else         { trace.lineTo(x, y); fill.lineTo(x, y); }
    }
    fill.lineTo(panel.right(), panel.bottom());
    fill.closeSubpath();

    QLinearGradient fillGrad(panel.left(), midY, panel.left(), panel.bottom());
    fillGrad.setColorAt(0.0, withAlpha(kAccentCyan, int(36 * wavePhase)));
    fillGrad.setColorAt(1.0, withAlpha(kAccentCyan, 0));
    p.setPen(Qt::NoPen);
    p.setBrush(fillGrad);
    p.drawPath(fill);

    QLinearGradient stroke(panel.left(), midY, panel.right(), midY);
    stroke.setColorAt(0.0,  withAlpha(QColor(56,  189, 248), int(235 * wavePhase)));
    stroke.setColorAt(0.45, withAlpha(kAccentCyan,            int(235 * wavePhase)));
    stroke.setColorAt(1.0,  withAlpha(QColor(167, 139, 250), int(235 * wavePhase)));
    p.setPen(QPen(stroke, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    p.drawPath(trace);

    p.setPen(QPen(withAlpha(kAccentCyan, int(42 * wavePhase)), 1.0, Qt::DashLine));
    p.drawLine(QPointF(panel.left() + 8, midY), QPointF(panel.right() - 8, midY));
}

void SplashScreen::drawEventMarkers(QPainter &p, const QRectF &panel, qreal progress) const
{
    // Markers appear after waveform is visible, one by one
    const qreal markerPhase = clamp01((progress - 0.30) / 0.55);
    if (markerPhase <= 0.0) return;

    const qreal midY = panel.center().y();
    const qreal amp  = panel.height() * 0.34 * easeInOutCubic(clamp01((progress - 0.12) / 0.70));

    // How many markers are visible (they appear sequentially)
    const int visible = int(markerPhase * kBeatCount);

    for (int i = 0; i < visible && i < kBeatCount; ++i) {
        const BeatEvent &ev = kBeats[i];
        const qreal x = panel.left() + ev.xNorm * panel.width();

        // Approximate Y from waveform shape at this x (use simplified peak offset)
        const qreal peakY = midY - amp * (ev.isA ? 0.85 : -0.55);

        // Fade in individually
        const qreal evPhase = clamp01((markerPhase * kBeatCount) - i);
        const int   alpha   = int(220 * easeOutCubic(evPhase));

        const QColor color = ev.isA ? withAlpha(kAccentGreen, alpha)
                                    : withAlpha(kAccentBlue,  alpha);

        // Outer glow ring
        p.setPen(QPen(withAlpha(color, alpha / 4), 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(x, peakY), 6.0, 6.0);

        // Inner filled dot
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        p.drawEllipse(QPointF(x, peakY), 3.0, 3.0);

        // Short vertical line from marker to baseline
        p.setPen(QPen(withAlpha(color, alpha / 3), 1.0, Qt::DotLine));
        p.drawLine(QPointF(x, peakY + (ev.isA ? 3.5 : -3.5)), QPointF(x, midY));
    }
}

void SplashScreen::drawBranding(QPainter &p, const QRectF &card, qreal progress) const
{
    const qreal textPhase = clamp01((progress - 0.34) / 0.42);
    const qreal textAlpha = easeInOutCubic(textPhase);
    if (textAlpha <= 0.01) return;

    const int alpha = int(255 * textAlpha);
    const qreal textTop = card.top() + 204;

    p.setPen(withAlpha(kAccentCyan, int(120 * textAlpha)));
    p.setFont(hudFont(9, true));
    p.drawText(QRectF(card.left() + 34, textTop, card.width() * 0.56, 14),
               Qt::AlignLeft | Qt::AlignVCenter,
               QStringLiteral("MECHANICAL WATCH TIMING ANALYZER"));

    // Glitch effect on title — brief chromatic aberration at progress ~0.55–0.60
    const qreal glitchWindow = clamp01((progress - 0.54) / 0.03)
                             * (1.0 - clamp01((progress - 0.58) / 0.03));
    const int glitchOffset = int(4 * glitchWindow);

    QFont title = uiFont(36, QFont::DemiBold);
    title.setLetterSpacing(QFont::AbsoluteSpacing, 4.0);
    p.setFont(title);

    if (glitchOffset > 0) {
        // Red channel shifted left
        p.setPen(withAlpha(QColor(255, 60, 60), int(alpha * 0.5)));
        p.drawText(QRectF(card.left() + 34 - glitchOffset, textTop + 16, card.width() * 0.56, 44),
                   Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("TimeGrapher"));
        // Cyan channel shifted right
        p.setPen(withAlpha(QColor(60, 220, 255), int(alpha * 0.5)));
        p.drawText(QRectF(card.left() + 34 + glitchOffset, textTop + 16, card.width() * 0.56, 44),
                   Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("TimeGrapher"));
    }
    p.setPen(withAlpha(kTextPrimary, alpha));
    p.drawText(QRectF(card.left() + 34, textTop + 16, card.width() * 0.56, 44),
               Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("TimeGrapher"));

    // Accent separator line
    const qreal lineY = textTop + 66;
    QLinearGradient accentLine(card.left() + 34, lineY, card.left() + 200, lineY);
    accentLine.setColorAt(0.0, withAlpha(kLgRed, alpha));
    accentLine.setColorAt(0.55, withAlpha(kAccentCyan, int(alpha * 0.7)));
    accentLine.setColorAt(1.0, withAlpha(kAccentCyan, 0));
    p.setPen(QPen(accentLine, 2.0, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(card.left() + 34, lineY), QPointF(card.right() - 34, lineY));

    // LG logo
    const qreal logoY = textTop + 72;
    if (!m_lgLogo.isNull()) {
        p.setOpacity(textAlpha);
        const QPixmap scaled = m_lgLogo.scaledToHeight(20, Qt::SmoothTransformation);
        p.drawPixmap(QPointF(card.left() + 34, logoY), scaled);
        p.setOpacity(1.0);
    } else {
        p.setPen(withAlpha(kLgRed, alpha));
        p.setFont(uiFont(15, QFont::DemiBold));
        p.drawText(QRectF(card.left() + 34, logoY, card.width() * 0.56, 22),
                   Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("LG Electronics"));
    }

    p.setPen(withAlpha(kTextMuted, int(alpha * 0.95)));
    p.setFont(uiFont(12, QFont::Normal));
    p.drawText(QRectF(card.left() + 34, textTop + 100, card.width() * 0.56, 20),
               Qt::AlignLeft | Qt::AlignVCenter, AppInfo::programLabel());
}

void SplashScreen::drawHudReadout(QPainter &p, const QRectF &card, qreal progress) const
{
    // HUD data panel — appears in bottom-right, values count up
    const qreal hudPhase = clamp01((progress - 0.50) / 0.40);
    const qreal hudAlpha = easeOutCubic(hudPhase);
    if (hudAlpha <= 0.01) return;

    const qreal panelW = 200.0;
    const qreal panelH = 115.0;
    const QRectF panel(card.right() - 34 - panelW, card.bottom() - 54 - panelH,
                       panelW, panelH);

    // Panel background
    QPainterPath panelPath;
    panelPath.addRoundedRect(panel, 6, 6);
    QLinearGradient bg(panel.topLeft(), panel.bottomRight());
    bg.setColorAt(0.0, withAlpha(QColor(12, 22, 38), int(220 * hudAlpha)));
    bg.setColorAt(1.0, withAlpha(QColor(6, 12, 22),  int(230 * hudAlpha)));
    p.setPen(QPen(withAlpha(kAccentCyan, int(60 * hudAlpha)), 1.0));
    p.setBrush(bg);
    p.drawPath(panelPath);

    // Header
    p.setFont(hudFont(8, true));
    p.setPen(withAlpha(kAccentCyan, int(160 * hudAlpha)));
    p.drawText(panel.adjusted(10, 8, -10, -8), Qt::AlignTop | Qt::AlignLeft,
               QStringLiteral("LIVE ANALYSIS"));

    // Separator line
    const qreal sepY = panel.top() + 24;
    p.setPen(QPen(withAlpha(kAccentCyan, int(40 * hudAlpha)), 1.0));
    p.drawLine(QPointF(panel.left() + 8, sepY), QPointF(panel.right() - 8, sepY));

    // Data rows — values animate in via countup
    struct Row { const char *label; const char *unit; qreal finalVal; bool isRate; };
    const Row rows[] = {
        { "BPH",      "",      28800.0, false },
        { "RATE",     " s/d",     +0.1, true  },
        { "AMP",      "\xc2\xb0", 302.0, false },
        { "BEAT ERR", " ms",      0.3,  false },
    };

    p.setFont(hudFont(9));
    const qreal rowH = (panelH - 30) / 4.0;

    for (int i = 0; i < 4; ++i) {
        // Each row appears slightly after the previous
        const qreal rowPhase = clamp01((hudPhase - i * 0.12) / 0.30);
        if (rowPhase <= 0.0) continue;
        const int rowAlpha = int(200 * rowPhase * hudAlpha);

        const qreal y = sepY + 4 + i * rowH;
        const QRectF rowRect(panel.left() + 10, y, panelW - 20, rowH);

        p.setPen(withAlpha(kTextMuted, rowAlpha));
        p.drawText(rowRect, Qt::AlignLeft | Qt::AlignVCenter, rows[i].label);

        // Count-up animation
        const qreal val = rows[i].finalVal * easeOutCubic(rowPhase);
        QString valStr;
        if (i == 0)      valStr = QString::number(int(val));
        else if (i == 1) valStr = QString::asprintf("%+.1f", val);
        else if (i == 2) valStr = QString::number(int(val));
        else             valStr = QString::asprintf("%.2f", val);
        valStr += rows[i].unit;

        const QColor valColor = (i == 1 && val > 0) ? kAccentGreen :
                                (i == 1 && val < 0) ? QColor(255, 80, 80) :
                                kAccentCyan;
        p.setPen(withAlpha(valColor, rowAlpha));
        p.drawText(rowRect, Qt::AlignRight | Qt::AlignVCenter, valStr);
    }
}

void SplashScreen::drawStatusBar(QPainter &p, const QRectF &card, qreal progress) const
{
    const qreal barPhase = easeInOutCubic(clamp01((progress - 0.20) / 0.55));
    const qreal footerTop = card.bottom() - 54;
    const QRectF metaRow(card.left() + 34, footerTop, card.width() * 0.52, 16);
    const QRectF hintRow(card.left() + 34, card.bottom() - 30, card.width() - 68, 14);

    p.setFont(uiFont(11, QFont::Medium));
    p.setPen(withAlpha(kTextPrimary, int(200 * barPhase)));
    p.drawText(metaRow, Qt::AlignLeft | Qt::AlignVCenter, AppInfo::teamLabel());

    p.setFont(hudFont(9));
    p.setPen(withAlpha(kTextMuted, int(170 * barPhase)));
    p.drawText(metaRow, Qt::AlignRight | Qt::AlignVCenter, AppInfo::versionDetail());

    if (barPhase > 0.05) {
        const QRectF track(metaRow.left(), metaRow.bottom() + 6, metaRow.width(), 2);
        p.setPen(Qt::NoPen);
        p.setBrush(withAlpha(QColor(255, 255, 255, 18), int(255 * barPhase)));
        p.drawRoundedRect(track, 1, 1);

        const qreal loadT = m_skipRequested ? 1.0
            : clamp01(qreal(m_clock.elapsed()) / qreal(kDurationMs));
        QLinearGradient fill(track.topLeft(), track.topRight());
        fill.setColorAt(0.0, withAlpha(kAccentCyan, int(220 * barPhase)));
        fill.setColorAt(1.0, withAlpha(kAccentViolet, int(180 * barPhase)));
        p.setBrush(fill);
        p.drawRoundedRect(QRectF(track.left(), track.top(), track.width() * loadT, 2), 1, 1);
    }

    if (!isAnimationComplete()) {
        const qreal blink = 0.45 + 0.55 * std::sin(progress * 24.0);
        p.setPen(withAlpha(kTextMuted, int(130 * blink * barPhase)));
        p.drawText(hintRow, Qt::AlignRight | Qt::AlignVCenter,
                   QStringLiteral("PRESS ANY KEY"));
    }
}

void SplashScreen::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) requestSkip();
    QWidget::mousePressEvent(event);
}

void SplashScreen::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
    requestSkip();
}
