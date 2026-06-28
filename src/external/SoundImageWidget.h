#ifndef SOUNDIMAGEWIDGET_H
#define SOUNDIMAGEWIDGET_H

#include <QWidget>
#include <QPoint>

class SoundImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SoundImageWidget(QWidget *parent = nullptr);
    void CreateImage(void);
    void DrawImage(void);
    QImage * GetImage(void);
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    // Map widget x → image x (accounting for zoom/pan)
    int imageXFromWidgetX(int wx) const;

private:
    void clampPan();

    QImage *image = nullptr;

    float  mZoom   = 1.0f;   // 1x–8x
    float  mPanX   = 0.0f;   // image-pixel offset of left edge

    QPoint mDragStart;
    float  mDragPanStart = 0.0f;
    bool   mDragging     = false;

    static constexpr float kZoomMin = 1.0f;
    static constexpr float kZoomMax = 8.0f;

signals:
    void clicked(int imageX);
};

#endif // SOUNDIMAGEWIDGET_H
