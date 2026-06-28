#include "SoundImageWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QToolTip>

SoundImageWidget::SoundImageWidget(QWidget *parent)
    : QWidget{parent}
{
    setMouseTracking(true);
}

void SoundImageWidget::CreateImage(void)
{
    image  = new QImage(size(), QImage::Format_ARGB32);
    mZoom  = 1.0f;
    mPanX  = 0.0f;
}

QImage * SoundImageWidget::GetImage(void)
{
    return image;
}

void SoundImageWidget::DrawImage(void)
{
    update();
}

// ── Zoom / pan helpers ────────────────────────────────────────

void SoundImageWidget::clampPan()
{
    if (!image) return;
    float maxPan = image->width() - image->width() / mZoom;
    if (maxPan < 0.0f) maxPan = 0.0f;
    if (mPanX < 0.0f)    mPanX = 0.0f;
    if (mPanX > maxPan)  mPanX = maxPan;
}

int SoundImageWidget::imageXFromWidgetX(int wx) const
{
    if (!image || width() == 0) return wx;
    float viewW = image->width() / mZoom;
    return static_cast<int>(mPanX + static_cast<float>(wx) / width() * viewW);
}

// ── Events ───────────────────────────────────────────────────

void SoundImageWidget::wheelEvent(QWheelEvent *event)
{
    if (!image) return;

    // Zoom toward cursor position
    float oldZoom = mZoom;
    float delta   = event->angleDelta().y() / 120.0f;
    mZoom *= (delta > 0) ? 1.25f : 0.8f;
    if (mZoom < kZoomMin) mZoom = kZoomMin;
    if (mZoom > kZoomMax) mZoom = kZoomMax;

    // Keep the image point under the cursor stationary
    float cursorImg = mPanX + static_cast<float>(event->position().x()) / width()
                      * (image->width() / oldZoom);
    mPanX = cursorImg - static_cast<float>(event->position().x()) / width()
            * (image->width() / mZoom);
    clampPan();
    update();

    // Show zoom level hint
    if (mZoom > 1.01f) {
        QToolTip::showText(event->globalPosition().toPoint(),
                           QString("Zoom %1×").arg(mZoom, 0, 'f', 1), this);
    } else {
        QToolTip::hideText();
    }
}

void SoundImageWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mDragStart    = event->pos();
        mDragPanStart = mPanX;
        mDragging     = false;
    }
}

void SoundImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        QToolTip::hideText();
        return;
    }
    int dx = event->pos().x() - mDragStart.x();
    if (!mDragging && std::abs(dx) > 4)
        mDragging = true;

    if (mDragging && image) {
        float viewW = image->width() / mZoom;
        mPanX = mDragPanStart - static_cast<float>(dx) / width() * viewW;
        clampPan();
        update();
    }
}

void SoundImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !mDragging)
        emit clicked(imageXFromWidgetX(event->pos().x()));
    mDragging = false;
}

// ── Paint ─────────────────────────────────────────────────────

void SoundImageWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);

    if (image) {
        QRect targetRect = rect();
        if (mZoom <= 1.01f) {
            painter.drawImage(targetRect, *image);
        } else {
            float viewW = image->width()  / mZoom;
            float viewH = image->height() / mZoom;
            // Center vertically (no vertical pan)
            float viewY = (image->height() - viewH) / 2.0f;
            QRectF srcRect(mPanX, viewY, viewW, viewH);
            painter.drawImage(targetRect, *image, srcRect);
        }
    }

    // Legend overlay — top-right corner
    const int margin = 8;
    const int lineH  = 18;
    const int swatchH = 10;
    const int padX = 8, padY = 6;

    struct Entry {
        QColor color;
        QColor color2;
        QColor color3;
        bool isLine;
        QString label;
    };
    const Entry entries[] = {
        { QColor(0,255,0), QColor(150,255,0), QColor(255,220,0), false, "A event (strong / medium / weak)" },
        { QColor(0,0,255), QColor(0,150,255), QColor(0,220,255), false, "C event (strong / medium / weak)" },
    };
    const int nEntries = static_cast<int>(sizeof(entries) / sizeof(entries[0]));

    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    QFontMetrics fm(font);

    const int dotD   = swatchH;
    const int dotGap = 3;
    const int multiSwatchW = dotD * 3 + dotGap * 2;
    const int textOff = multiSwatchW + 6;

    int maxTextW = 0;
    for (const auto &e : entries)
        maxTextW = qMax(maxTextW, fm.horizontalAdvance(e.label));

    int boxW = padX * 2 + textOff + maxTextW;
    int boxH = padY * 2 + nEntries * lineH;
    int boxX = width() - boxW - margin;
    int boxY = margin;

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 140));
    painter.drawRoundedRect(boxX, boxY, boxW, boxH, 4, 4);

    for (int i = 0; i < nEntries; ++i) {
        const auto &e = entries[i];
        int ey = boxY + padY + i * lineH + lineH / 2;
        int ex = boxX + padX;

        painter.setPen(Qt::NoPen);
        if (e.isLine) {
            painter.fillRect(ex, ey - 1, multiSwatchW, 2, e.color);
        } else {
            const QColor dots[3] = { e.color, e.color2, e.color3 };
            for (int d = 0; d < 3; ++d) {
                if (dots[d].isValid()) {
                    painter.setBrush(dots[d]);
                    painter.drawEllipse(ex + d * (dotD + dotGap), ey - dotD / 2, dotD, dotD);
                }
            }
        }

        painter.setPen(Qt::white);
        painter.drawText(ex + textOff, ey + fm.ascent() / 2 - 1, e.label);
    }

    // Zoom indicator (bottom-left)
    if (mZoom > 1.01f) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 140));
        QRect zRect(margin, height() - 24 - margin, 70, 24);
        painter.drawRoundedRect(zRect, 4, 4);
        painter.setPen(Qt::white);
        font.setPointSize(9);
        painter.setFont(font);
        painter.drawText(zRect, Qt::AlignCenter, QString("Zoom %1×").arg(mZoom, 0, 'f', 1));
    }
}
