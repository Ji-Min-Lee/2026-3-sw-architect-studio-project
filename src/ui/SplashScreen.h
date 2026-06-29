#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>
#include <QElapsedTimer>
#include <QTimer>
#include <QPixmap>

class SplashScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SplashScreen(QWidget *parent = nullptr);

    void start();
    bool isAnimationComplete() const;
    void requestSkip();

    static constexpr int kDurationMs = 5000;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onTick();

private:
    qreal animationProgress() const;
    QRectF contentCard() const;

    void drawBackdrop(QPainter &p, qreal progress) const;
    void drawHudCard(QPainter &p, qreal progress) const;
    void drawCornerBrackets(QPainter &p, const QRectF &card, qreal progress) const;
    void drawScopePanel(QPainter &p, const QRectF &card, qreal progress) const;
    void drawWaveform(QPainter &p, const QRectF &panel, qreal progress) const;
    void drawEventMarkers(QPainter &p, const QRectF &panel, qreal progress) const;
    void drawBranding(QPainter &p, const QRectF &card, qreal progress) const;
    void drawHudReadout(QPainter &p, const QRectF &card, qreal progress) const;
    void drawStatusBar(QPainter &p, const QRectF &card, qreal progress) const;

    QFont hudFont(int pixelSize, bool bold = false) const;
    QFont uiFont(int pixelSize, QFont::Weight weight = QFont::Normal) const;

    QElapsedTimer m_clock;
    QTimer m_timer;
    QPixmap m_lgLogo;
    bool m_skipRequested = false;
};

#endif // SPLASHSCREEN_H
