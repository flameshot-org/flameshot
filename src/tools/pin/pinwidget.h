// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class QLabel;
class QVBoxLayout;
class QGraphicsDropShadowEffect;

class PinWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PinWidget(const QPixmap& pixmap,
                       const QRect& geometry,
                       QWidget* parent = nullptr);

protected:
    void wheelEvent(QWheelEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    void setScaledPixmapToLabel(const QSize& newSize,
                                const qreal scale,
                                const bool expanding);

    QPixmap m_pixmap;
    QVBoxLayout* m_layout;
    QLabel* m_label;
    QPoint m_dragStart;
    qreal m_offsetX, m_offsetY;
    QGraphicsDropShadowEffect* m_shadowEffect;
    QColor m_baseColor, m_hoverColor;
};
