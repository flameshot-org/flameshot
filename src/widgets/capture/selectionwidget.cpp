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
  , m_activeSide(NO_SIDE)
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
    } else if (rect().contains(point)) {
        return CENTER;
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

// helper function
SelectionWidget::SideType getProperSide(SelectionWidget::SideType side,
                                        const QRect& r)
{
    using SideType = SelectionWidget::SideType;
    int intSide = side;
    if (r.right() < r.left()) {
        intSide ^= SideType::LEFT_SIDE;
        intSide ^= SideType::RIGHT_SIDE;
    }
    if (r.bottom() < r.top()) {
        intSide ^= SideType::TOP_SIDE;
        intSide ^= SideType::BOTTOM_SIDE;
    }

    return (SideType)intSide;
}

void SelectionWidget::setIgnoreMouse(bool ignore)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, ignore);
    if (ignore) {
        unsetCursor();
    } else {
        setCursor(Qt::ArrowCursor);
    }
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
            unsetCursor();
            return false;
        }
        auto* e = static_cast<QMouseEvent*>(event);
        parentMouseMoveEvent(e);
        return e->isAccepted();
    }
    return QWidget::eventFilter(obj, event);
}

void SelectionWidget::mousePressEvent(QMouseEvent* e)
{
    QPoint pos = mapToParent(e->pos());
    if (e->button() == Qt::LeftButton) {
        m_dragStartPos = pos;
        m_activeSide = getMouseSide(e->pos());
    }
}

void SelectionWidget::mouseReleaseEvent(QMouseEvent* e)
{
    m_activeSide = NO_SIDE;
    setCursor(Qt::ArrowCursor);
}

void SelectionWidget::parentMouseMoveEvent(QMouseEvent* e)
{
    if (m_activeSide == NO_SIDE) {
        e->ignore();
    }

    // Mouse position relative to CaptureWidget
    QPoint pos = e->pos();
    auto geom = geometry();

    SideType mouseSide = m_activeSide;
    if (!m_activeSide) {
        mouseSide = getMouseSide(mapFromParent(e->pos()));
    }
    switch (mouseSide) {
        case TOPLEFT_SIDE:
            setCursor(Qt::SizeFDiagCursor);
            if (m_activeSide)
                geom = QRect(pos, geom.bottomRight());
            break;
        case BOTTOMRIGHT_SIDE:
            setCursor(Qt::SizeFDiagCursor);
            if (m_activeSide)
                geom = QRect(geom.topLeft(), pos);
            break;
        case TOPRIGHT_SIDE:
            setCursor(Qt::SizeBDiagCursor);
            if (m_activeSide)
                geom = QRect(QPoint(geom.left(), pos.y()),
                             QPoint(pos.x(), geom.bottom()));
            break;
        case BOTTOMLEFT_SIDE:
            setCursor(Qt::SizeBDiagCursor);
            if (m_activeSide)
                geom = QRect(QPoint(pos.x(), geom.top()),
                             QPoint(geom.right(), pos.y()));
            break;
        case LEFT_SIDE:
            setCursor(Qt::SizeHorCursor);
            if (m_activeSide)
                geom = QRect(QPoint(pos.x(), geom.top()), geom.bottomRight());
            break;
        case RIGHT_SIDE:
            setCursor(Qt::SizeHorCursor);
            if (m_activeSide)
                geom = QRect(geom.topLeft(), QPoint(pos.x(), geom.bottom()));
            break;
        case TOP_SIDE:
            setCursor(Qt::SizeVerCursor);
            if (m_activeSide)
                geom = QRect(QPoint(geom.left(), pos.y()), geom.bottomRight());
            break;
        case BOTTOM_SIDE:
            setCursor(Qt::SizeVerCursor);
            if (m_activeSide)
                geom = QRect(geom.topLeft(), QPoint(geom.right(), pos.y()));
            break;
        default:
            if (m_activeSide) {
                setCursor(Qt::ClosedHandCursor);
                move(this->pos() + pos - m_dragStartPos);
                m_dragStartPos = pos;
                return;
            } else {
                setCursor(Qt::ArrowCursor);
                return;
            }
            break;
    }
    // finalize geometry change
    if (m_activeSide) {
        setGeometry(geom.normalized());
        m_activeSide = getProperSide(m_activeSide, geom);
    }
    m_dragStartPos = pos;
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
    m_TLArea.moveTo(r.topLeft() + m_areaOffset);
    m_TRArea.moveTo(r.topRight() + m_areaOffset);
    m_BLArea.moveTo(r.bottomLeft() + m_areaOffset);
    m_BRArea.moveTo(r.bottomRight() + m_areaOffset);

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
