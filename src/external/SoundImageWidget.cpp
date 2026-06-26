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
    Q_UNUSED(event);
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

    struct Entry {
        QColor color;
        QColor color2;   // second swatch (invalid = none)
        QColor color3;
        bool isLine;
        QString label;
    };
    const Entry entries[] = {
        // A events: 3-tone confidence gradient
        { QColor(0,255,0), QColor(150,255,0), QColor(255,220,0), false, "A event (strong / medium / weak)" },
        // C events: 3-tone confidence gradient
        { QColor(0,0,255), QColor(0,150,255), QColor(0,220,255), false, "C event (strong / medium / weak)" },
    };
    const int nEntries = static_cast<int>(sizeof(entries) / sizeof(entries[0]));

    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    QFontMetrics fm(font);

    // Each multi-swatch row needs room for 3 dots; compute total swatch width
    const int dotD   = swatchH;          // dot diameter
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
            // Draw three dots for confidence gradient
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
}
