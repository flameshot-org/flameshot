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

class QPropertyAnimation;

class SelectionWidget : public QWidget
{
    Q_OBJECT
public:
    enum SideType {
        TOPLEFT_SIDE,
        BOTTONLEFT_SIDE,
        TOPRIGHT_SIDE,
        BOTTONRIGHT_SIDE,
        TOP_SIDE,
        BOTTON_SIDE,
        RIGHT_SIDE,
        LEFT_SIDE,
        NO_SIDE,
    };

    explicit SelectionWidget(const QColor &c, QWidget *parent = nullptr);

    SideType getMouseSide(const QPoint &point) const;
    QVector<QRect> handlerAreas();

    void setGeometryAnimated(const QRect &r);
    void saveGeometry();
    QRect savedGeometry();

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void moveEvent(QMoveEvent *);

signals:
    void animationEnded();

public slots:
    void updateColor(const QColor &c);

private:
    void updateAreas();

    QPropertyAnimation *m_animation;

    QColor m_color;
    QPoint m_areaOffset;
    QPoint m_handleOffset;
    QRect m_geometryBackup;

    // naming convention for handles
    // T top, B bottom, R Right, L left
    // 2 letters: a corner
    // 1 letter: the handle on the middle of the corresponding side
    QRect m_TLHandle, m_TRHandle, m_BLHandle, m_BRHandle;
    QRect m_LHandle, m_THandle, m_RHandle, m_BHandle;

    QRect m_TLArea, m_TRArea, m_BLArea, m_BRArea;
    QRect m_LArea, m_TArea, m_RArea, m_BArea;
};
