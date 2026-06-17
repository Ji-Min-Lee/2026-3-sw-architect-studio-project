#include "SoundImageWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>

SoundImageWidget::SoundImageWidget(QWidget *parent)
    : QWidget{parent}
{
    setMouseTracking(true);
}

void  SoundImageWidget::CreateImage(void)
{
    image=new QImage(size(), QImage::Format_ARGB32);
}

QImage * SoundImageWidget::GetImage(void)
{
    return image;
}

void  SoundImageWidget::DrawImage(void)
{
    update();
    return;
    image->fill(Qt::black); // Clear screen

    // 2. Manipulate raw pixels (Fast method)
    int width = image->width();
    int height = image->height();
    for (int y = 0; y < height; ++y) {
        // Get pointer to current line
        QRgb *line = reinterpret_cast<QRgb*>(image->scanLine(y));
        for (int x = 0; x < width; ++x) {
            // Example: Create a gradient effect
            line[x] = qRgba(x % 255, y % 255, (x+y) % 255, 255);
        }
    }

}

void SoundImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!image || height() <= 0) return;

    // Grid lines are at y-fraction 0.0 (A event boundary) and 0.5 (C event half-period).
    // Use a ±4% band around each line to trigger the tooltip.
    double yFrac = static_cast<double>(event->pos().y()) / height();

    QString tip;
    if (yFrac < 0.04 || yFrac > 0.96) {
        tip = "<b style='color:#0044ee'>● C event boundary (half-period)</b><br>"
              "Expected position of the complementary (C) impulse.<br>"
              "Vertical spread indicates beat error or amplitude variation.";
    } else if (yFrac > 0.46 && yFrac < 0.54) {
        tip = "<b style='color:#00cc00'>● A event boundary</b><br>"
              "Expected position of the tick/toc (A) impulse.<br>"
              "Events clustering here indicate stable beat detection.";
    }

    if (!tip.isEmpty())
        QToolTip::showText(event->globalPos(), tip, this);
    else
        QToolTip::hideText();
}

void SoundImageWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QRect targetRect = rect();
    painter.drawImage(targetRect, *image);

    // Legend overlay — top-right corner
    const int margin = 8;
    const int lineH  = 18;
    const int swatchW = 14;
    const int swatchH = 10;
    const int padX = 8, padY = 6;

    struct Entry { QColor color; bool isLine; QString label; };
    const Entry entries[] = {
        { QColor(0, 200, 0),  false, "A event" },
        { QColor(0, 0, 220),  false, "C event" },
        { QColor(0, 200, 0),  true,  "A event boundary" },
        { QColor(0, 0, 200),  true,  "C event boundary (half-period)" },
    };
    const int nEntries = static_cast<int>(sizeof(entries) / sizeof(entries[0]));

    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    QFontMetrics fm(font);

    int maxTextW = 0;
    for (const auto &e : entries)
        maxTextW = qMax(maxTextW, fm.horizontalAdvance(e.label));

    int boxW = padX * 2 + swatchW + 6 + maxTextW;
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
        painter.setBrush(e.color);
        if (e.isLine) {
            painter.fillRect(ex, ey - 1, swatchW, 2, e.color);
        } else {
            painter.drawEllipse(ex + 2, ey - swatchH / 2, swatchH, swatchH);
        }

        painter.setPen(Qt::white);
        painter.drawText(ex + swatchW + 6, ey + fm.ascent() / 2 - 1, e.label);
    }
}
