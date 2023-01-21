// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class QPropertyAnimation;

class SelectionWidget : public QWidget
{
    Q_OBJECT
public:
    enum SideType
    {
        NO_SIDE = 0,
        TOP_SIDE = 0b0001,
        BOTTOM_SIDE = 0b0010,
        RIGHT_SIDE = 0b0100,
        LEFT_SIDE = 0b1000,
        TOPLEFT_SIDE = TOP_SIDE | LEFT_SIDE,
        BOTTOMLEFT_SIDE = BOTTOM_SIDE | LEFT_SIDE,
        TOPRIGHT_SIDE = TOP_SIDE | RIGHT_SIDE,
        BOTTOMRIGHT_SIDE = BOTTOM_SIDE | RIGHT_SIDE,
        CENTER = 0b10000,
    };

    explicit SelectionWidget(QColor c, QWidget* parent = nullptr);

    SideType getMouseSide(const QPoint& mousePos) const;
    QVector<QRect> handlerAreas();

    void setIgnoreMouse(bool ignore);
    void setIdleCentralCursor(const QCursor& cursor);

    void setGeometryAnimated(const QRect& r);
    void setGeometry(const QRect& r);
    QRect geometry() const;
    QRect fullGeometry() const;

    QRect rect() const;

protected:
    bool eventFilter(QObject*, QEvent*) override;
    void parentMousePressEvent(QMouseEvent* e);
    void parentMouseReleaseEvent(QMouseEvent* e);
    void parentMouseMoveEvent(QMouseEvent* e);

    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void moveEvent(QMoveEvent*) override;

    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

signals:
    void animationEnded();
    void geometryChanged();
    void geometrySettled();
    void visibilityChanged();

public slots:
    void updateColor(const QColor& c);

    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();

    void resizeLeft();
    void resizeRight();
    void resizeUp();
    void resizeDown();

private:
    void updateAreas();
    void updateCursor();
    void setGeometryByKeyboard(const QRect& r);

    QPropertyAnimation* m_animation;

    QColor m_color;
    QPoint m_areaOffset;
    QPoint m_handleOffset;

    QPoint m_dragStartPos;
    SideType m_activeSide;
    QCursor m_idleCentralCursor;
    bool m_ignoreMouse;
    bool m_mouseStartMove;

    // naming convention for handles
    // T top, B bottom, R Right, L left
    // 2 letters: a corner
    // 1 letter: the handle on the middle of the corresponding side
    QRect m_TLHandle, m_TRHandle, m_BLHandle, m_BRHandle;
    QRect m_LHandle, m_THandle, m_RHandle, m_BHandle;

    QRect m_TLArea, m_TRArea, m_BLArea, m_BRArea;
    QRect m_LArea, m_TArea, m_RArea, m_BArea;
};
