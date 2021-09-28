// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "selectionwidget.h"
#include "capturetool.h"
#include "capturetoolbutton.h"
#include "src/utils/globalvalues.h"
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>

#define MARGIN (m_THandle.width())

SelectionWidget::SelectionWidget(const QColor& c, QWidget* parent)
  : QWidget(parent)
  , m_color(c)
  , m_draggingAround(false)
  , m_resizingSide(NO_SIDE)
{
    setMouseTracking(true);
    parent->installEventFilter(this);

    m_animation = new QPropertyAnimation(this, "geometry", this);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->setDuration(200);
    connect(m_animation,
            &QPropertyAnimation::finished,
            this,
            &SelectionWidget::animationEnded);

    // TODO setAttribute(Qt::WA_TransparentForMouseEvents);
    int sideVal = GlobalValues::buttonBaseSize() * 0.6;
    int handleSide = sideVal / 2;
    const QRect areaRect(0, 0, sideVal, sideVal);

    const QRect handleRect(0, 0, handleSide, handleSide);
    m_TLHandle = m_TRHandle = m_BLHandle = m_BRHandle = m_LHandle = m_THandle =
      m_RHandle = m_BHandle = handleRect;
    m_TLArea = m_TRArea = m_BLArea = m_BRArea = areaRect;

    m_areaOffset = QPoint(-sideVal / 2, -sideVal / 2);
    m_handleOffset = QPoint(-handleSide / 2, -handleSide / 2);
}

SelectionWidget::SideType SelectionWidget::getMouseSide(
  const QPoint& point) const
{
    if (m_TLArea.contains(point)) {
        return TOPLEFT_SIDE;
    } else if (m_TRArea.contains(point)) {
        return TOPRIGHT_SIDE;
    } else if (m_BLArea.contains(point)) {
        return BOTTOMLEFT_SIDE;
    } else if (m_BRArea.contains(point)) {
        return BOTTOMRIGHT_SIDE;
    } else if (m_LArea.contains(point)) {
        return LEFT_SIDE;
    } else if (m_TArea.contains(point)) {
        return TOP_SIDE;
    } else if (m_RArea.contains(point)) {
        return RIGHT_SIDE;
    } else if (m_BArea.contains(point)) {
        return BOTTOM_SIDE;
    } else {
        return NO_SIDE;
    }
}

QVector<QRect> SelectionWidget::handlerAreas()
{
    QVector<QRect> areas;
    areas << m_TLHandle << m_TRHandle << m_BLHandle << m_BRHandle << m_LHandle
          << m_THandle << m_RHandle << m_BHandle;
    return areas;
}

void SelectionWidget::setGeometryAnimated(const QRect& r)
{
    if (isVisible()) {
        m_animation->setStartValue(geometry());
        m_animation->setEndValue(r);
        m_animation->start();
    }
}

void SelectionWidget::setGeometry(const QRect& r)
{
    QWidget::setGeometry(r + QMargins(MARGIN, MARGIN, MARGIN, MARGIN));
    emit geometryChanged();
}

QRect SelectionWidget::geometry() const
{
    return QWidget::geometry() - QMargins(MARGIN, MARGIN, MARGIN, MARGIN);
}

QRect SelectionWidget::fullGeometry() const
{
    return QWidget::geometry();
}

void SelectionWidget::saveGeometry()
{
    m_geometryBackup = geometry();
}

QRect SelectionWidget::savedGeometry()
{
    return m_geometryBackup;
}

QRect SelectionWidget::rect() const
{
    return QWidget::rect() - QMargins(MARGIN, MARGIN, MARGIN, MARGIN);
}

bool SelectionWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseMove) {
        if (testAttribute(Qt::WA_TransparentForMouseEvents)) {
            return false;
        }
        auto* e = static_cast<QMouseEvent*>(event);
        if (!(m_draggingAround || fullGeometry().contains(e->pos()))) {
            return false;
        }
        QPoint localPos = mapFromParent(e->pos());
        auto mouseSide = getMouseSide(localPos);
        QPoint pos = e->pos();
        if (mouseSide == TOPLEFT_SIDE || m_resizingSide == TOPLEFT_SIDE) {
            setCursor(Qt::SizeFDiagCursor);
            if (e->buttons() == Qt::LeftButton) {
                //                auto r = fullGeometry();
                //                r.setTopLeft(pos - localPos);
                //                QWidget::setGeometry(r);
                //                m_resizingSide = mouseSide;
            }
        }
        switch (mouseSide) {
            case TOPLEFT_SIDE:
            case BOTTOMRIGHT_SIDE:
                break;
            case TOPRIGHT_SIDE:
            case BOTTOMLEFT_SIDE:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case LEFT_SIDE:
            case RIGHT_SIDE:
                setCursor(Qt::SizeHorCursor);
                break;
            case TOP_SIDE:
            case BOTTOM_SIDE:
                setCursor(Qt::SizeVerCursor);
                break;
            default:
                if (e->buttons() == Qt::LeftButton) {
                    move(this->pos() + pos - m_dragStartPos);
                    m_dragStartPos = pos;
                    qDebug() << "Delta: " << pos - m_dragStartPos;
                    qDebug() << "Parent: " << pos;
                    qDebug() << "dragStartPos: " << m_dragStartPos;
                    qDebug() << "pos(): " << this->pos();
                    setCursor(Qt::ClosedHandCursor);
                } else {
                    unsetCursor();
                }
                break;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void SelectionWidget::mousePressEvent(QMouseEvent* e)
{
    QPoint pos = mapToParent(e->pos());
    if (e->button() == Qt::LeftButton && fullGeometry().contains(pos)) {
        m_dragStartPos = pos;
        m_draggingAround = true;
        saveGeometry();
    }
}

void SelectionWidget::mouseReleaseEvent(QMouseEvent* e)
{
    m_draggingAround = false;
    unsetCursor();
}

void SelectionWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setPen(m_color);
    p.drawRect(rect() + QMargins(0, 0, -1, -1));
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(m_color);
    for (auto rect : handlerAreas()) {
        p.drawEllipse(rect);
    }
}

void SelectionWidget::resizeEvent(QResizeEvent*)
{
    updateAreas();
    emit geometryChanged();
}

void SelectionWidget::moveEvent(QMoveEvent*)
{
    updateAreas();
    emit geometryChanged();
}

void SelectionWidget::updateColor(const QColor& c)
{
    m_color = c;
}

void SelectionWidget::updateAreas()
{
    QRect r = rect();
    QPoint pos(MARGIN, MARGIN);
    pos = {};
    m_TLArea.moveTo(m_areaOffset + pos);
    m_TRArea.moveTo(r.topRight() + m_areaOffset + pos);
    m_BLArea.moveTo(r.bottomLeft() + m_areaOffset + pos);
    m_BRArea.moveTo(r.bottomRight() + m_areaOffset + pos);

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
