// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <QWidget>
#include <QGraphicsDropShadowEffect>

class QVBoxLayout;
class QLabel;

class PinWidget : public QWidget {
    Q_OBJECT
public:
    explicit PinWidget(const QPixmap &pixmap, QWidget *parent = nullptr);

    int margin() const;

protected:
    void wheelEvent(QWheelEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

private:
    void setScaledPixmap(const QSize &size);

    QPixmap m_pixmap;
    QVBoxLayout *m_layout;
    QLabel *m_label;
    QPoint m_dragStart;
    qreal m_offsetX, m_offsetY;
    QGraphicsDropShadowEffect *m_shadowEffect;
    QColor m_baseColor, m_hoverColor;
};
