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
        TOPLEFT_SIDE,
        BOTTOMLEFT_SIDE,
        TOPRIGHT_SIDE,
        BOTTOMRIGHT_SIDE,
        TOP_SIDE,
        BOTTOM_SIDE,
        RIGHT_SIDE,
        LEFT_SIDE,
        NO_SIDE,
    };

    explicit SelectionWidget(const QColor& c, QWidget* parent = nullptr);

    SideType getMouseSide(const QPoint& point) const;
    QVector<QRect> handlerAreas();

    void setGeometryAnimated(const QRect& r);
    void saveGeometry();
    QRect savedGeometry();
    QRect captureGeomtry();
    void setCaptureGeometry(QRect rect);
    void SetScale(float v);
    QRect captureToWidgetRect(QRect rect);

protected:
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void moveEvent(QMoveEvent*);

signals:
    void animationEnded();

public slots:
    void updateColor(const QColor& c);

private:
    void updateAreas();

    QPropertyAnimation* m_animation;

    QColor m_color;
    QPoint m_areaOffset;
    QPoint m_handleOffset;
    QRect m_geometryBackup;
    QRect m_captureGeometry;
    QTransform transform;

    // naming convention for handles
    // T top, B bottom, R Right, L left
    // 2 letters: a corner
    // 1 letter: the handle on the middle of the corresponding side
    QRect m_TLHandle, m_TRHandle, m_BLHandle, m_BRHandle;
    QRect m_LHandle, m_THandle, m_RHandle, m_BHandle;

    QRect m_TLArea, m_TRArea, m_BLArea, m_BRArea;
    QRect m_LArea, m_TArea, m_RArea, m_BArea;
};
