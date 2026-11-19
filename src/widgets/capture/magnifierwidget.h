#pragma once

#include <QWidget>

class QPropertyAnimation;

class MagnifierWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MagnifierWidget(const QPixmap& p,
                             const QColor& c,
                             bool isSquare,
                             QWidget* parent = nullptr);

    void setCursorPos(const QPoint& pos);
    void renderAt(QPainter& painter,
                  const QPoint& cursorPos,
                  const QRect& bounds);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    const int m_magPixels = 8;
    const int m_magOffset = 16;
    const int magZoom = 10;
    const int m_pixels = 2 * m_magPixels + 1;
    qreal m_devicePixelRatio = 1.0;
    bool m_square;
    bool m_hasCursorPos = false;
    QPoint m_cursorPos;
    QColor m_color;
    QColor m_borderColor;
    QPixmap m_screenshot;
    QPixmap m_paddedScreenshot;
    void drawMagnifier(QPainter& painter,
                       const QPoint& cursorPos,
                       const QRect& bounds);
    void drawMagnifierCircle(QPainter& painter,
                             const QPoint& cursorPos,
                             const QRect& bounds);
};
