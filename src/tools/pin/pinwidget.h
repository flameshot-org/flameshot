// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class QLabel;
class QVBoxLayout;
class QGestureEvent;
class QPinchGesture;
class QGraphicsDropShadowEffect;

class PinWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PinWidget(const QPixmap& pixmap,
                       const QRect& geometry,
                       QWidget* parent = nullptr);

protected:
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;

    bool event(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    bool gestureEvent(QGestureEvent* event);
    bool scrollEvent(QWheelEvent* e);
    void pinchTriggered(QPinchGesture*);

    QPixmap m_pixmap;
    QVBoxLayout* m_layout;
    QLabel* m_label;
    QPoint m_dragStart;
    qreal m_offsetX{}, m_offsetY{};
    QGraphicsDropShadowEffect* m_shadowEffect;
    QColor m_baseColor, m_hoverColor;

    bool m_expanding{ false };
    qreal m_scaleFactor{ 1 };
    qreal m_currentStepScaleFactor{ 1 };
    bool m_sizeChanged{ false };

private slots:
    void showContextMenu(const QPoint& pos);
    void copyToClipboard();
    void saveToFile();
};
