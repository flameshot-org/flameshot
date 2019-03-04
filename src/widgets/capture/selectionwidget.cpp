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

#include "selectionwidget.h"
#include "src/utils/globalvalues.h"
#include <QPainter>
#include <QPropertyAnimation>

SelectionWidget::SelectionWidget(const QColor &c, QWidget *parent) :
    QWidget(parent), m_color(c)
{
    m_animation = new QPropertyAnimation(this, "geometry", this);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->setDuration(200);
    connect(m_animation, &QPropertyAnimation::finished,
            this, &SelectionWidget::animationEnded);

    setAttribute(Qt::WA_TransparentForMouseEvents);
    int sideVal = GlobalValues::buttonBaseSize() * 0.6;
    int handleSide = sideVal / 2;
    const QRect areaRect(0, 0, sideVal, sideVal);
    const QRect handleRect(0, 0, handleSide, handleSide);
    m_TLHandle = m_TRHandle = m_BLHandle = m_BRHandle =
            m_LHandle = m_THandle = m_RHandle = m_BHandle= handleRect;
    m_TLArea = m_TRArea = m_BLArea = m_BRArea = areaRect;

    m_areaOffset = QPoint(-sideVal/2, -sideVal/2);
    m_handleOffset = QPoint(-handleSide/2, -handleSide/2);
}

SelectionWidget::SideType SelectionWidget::getMouseSide(const QPoint &point) const {
    if (m_TLArea.contains(point)) {
        return TOPLEFT_SIDE;
    } else if (m_TRArea.contains(point)) {
        return TOPRIGHT_SIDE;
    } else if (m_BLArea.contains(point)) {
        return BOTTONLEFT_SIDE;
    } else if (m_BRArea.contains(point)) {
        return BOTTONRIGHT_SIDE;
    } else if (m_LArea.contains(point)) {
        return LEFT_SIDE;
    } else if (m_TArea.contains(point)) {
        return TOP_SIDE;
    } else if (m_RArea.contains(point)) {
        return RIGHT_SIDE;
    } else if (m_BArea.contains(point)) {
        return  BOTTON_SIDE;
    } else {
        return NO_SIDE;
    }
}

QVector<QRect> SelectionWidget::handlerAreas() {
    QVector<QRect> areas;
    areas << m_TLHandle << m_TRHandle << m_BLHandle << m_BRHandle
      <<m_LHandle << m_THandle << m_RHandle << m_BHandle;
    return areas;
}

void SelectionWidget::setGeometryAnimated(const QRect &r) {
    if (isVisible()) {
        m_animation->setStartValue(geometry());
        m_animation->setEndValue(r);
        m_animation->start();
    }
}

void SelectionWidget::saveGeometry() {
    m_geometryBackup = geometry();
}

QRect SelectionWidget::savedGeometry() {
    return m_geometryBackup;
}

void SelectionWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setPen(m_color);
    p.drawRect(rect() + QMargins(0, 0, -1, -1));
}

void SelectionWidget::resizeEvent(QResizeEvent *) {
    updateAreas();
}

void SelectionWidget::moveEvent(QMoveEvent *) {
    updateAreas();
}

void SelectionWidget::updateColor(const QColor &c) {
    m_color = c;
}

void SelectionWidget::updateAreas() {
    QRect r = rect();
    m_TLArea.moveTo(m_areaOffset + pos());
    m_TRArea.moveTo(r.topRight() + m_areaOffset + pos());
    m_BLArea.moveTo(r.bottomLeft() + m_areaOffset + pos());
    m_BRArea.moveTo(r.bottomRight() + m_areaOffset + pos());

    m_LArea = QRect(m_TLArea.bottomLeft(), m_BLArea.topRight());
    m_TArea = QRect(m_TLArea.topRight(), m_TRArea.bottomLeft());
    m_RArea = QRect(m_TRArea.bottomLeft(), m_BRArea.topRight());
    m_BArea = QRect(m_BLArea.topRight(), m_BRArea.bottomLeft());

    m_TLHandle.moveTo(m_TLArea.center() + m_handleOffset);
    m_BLHandle.moveTo(m_BLArea.center() + m_handleOffset);
    m_TRHandle.moveTo(m_TRArea.center() + m_handleOffset);
    m_BRHandle.moveTo(m_BRArea.center() + m_handleOffset);
    m_LHandle.moveTo(m_LArea.center() + m_handleOffset);
    m_THandle.moveTo(m_TArea.center() + m_handleOffset);
    m_RHandle.moveTo(m_RArea.center() + m_handleOffset);
    m_BHandle.moveTo(m_BArea.center() + m_handleOffset);
}
