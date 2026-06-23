// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class QLabel;
class QVBoxLayout;
class QGestureEvent;
class QPinchGesture;
class QGraphicsDropShadowEffect;
class QCloseEvent;

class PinWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PinWidget(const QPixmap& pixmap,
                       const QRect& geometry,
                       QWidget* parent = nullptr,
                       qreal zoom = 1.0,
                       qreal opacity = 1.0);

signals:
    void dismissed(const QPixmap& pixmap,
                   const QRect& geometry,
                   qreal zoom,
                   qreal opacity,
                   const QString& screenName);

protected:
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;
    void closeEvent(QCloseEvent*) override;

    bool event(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    bool gestureEvent(QGestureEvent* event);
    bool scrollEvent(QWheelEvent* e);
    void pinchTriggered(QPinchGesture*);
    void closePin();

    void rotateLeft();
    void rotateRight();

    void increaseOpacity();
    void decreaseOpacity();

    QPixmap m_pixmap;
    QVBoxLayout* m_layout;
    QLabel* m_label;
    QGraphicsDropShadowEffect* m_shadowEffect;
    QColor m_baseColor, m_hoverColor;

    bool m_expanding{ false };
    qreal m_scaleFactor;
    qreal m_opacity;
    unsigned int m_rotateFactor{ 0 };
    qreal m_currentStepScaleFactor{ 1 };
    bool m_sizeChanged;

private slots:
    void showContextMenu(const QPoint& pos);
    void copyToClipboard();
    void saveToFile();
};
