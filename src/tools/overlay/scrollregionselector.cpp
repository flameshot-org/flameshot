#include "scrollregionselector.h"

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPen>
#include <QPainterPath>

ScrollRegionSelector::ScrollRegionSelector(QWidget* parent)
  : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::Tool |
                   Qt::WindowStaysOnTopHint);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::CrossCursor);
}

void ScrollRegionSelector::setScreenshot(const QImage& image)
{
    m_screenshot = image;
    update();
}

bool ScrollRegionSelector::hasScreenshot() const
{
    return !m_screenshot.isNull();
}

QRect ScrollRegionSelector::currentRect() const
{
    return QRect(m_start, m_end).normalized();
}

void ScrollRegionSelector::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRect sel = currentRect();

    if (!m_screenshot.isNull()) {
        // Modo Wayland: se dibuja la captura y se sombrea fuera de la selección
        p.drawImage(rect(), m_screenshot);

        if (!sel.isNull()) {
            QPainterPath full;
            full.addRect(rect());

            QPainterPath hole;
            hole.addRect(sel);

            p.fillPath(full.subtracted(hole), QColor(0, 0, 0, 110));
            p.fillRect(sel, QColor(255, 0, 0, 30));
        }
    } else {
        // Modo Xorg/X11: NO oscurecer fondo, dejar totalmente transparente
        p.fillRect(rect(), Qt::transparent);

        if (!sel.isNull()) {
            p.fillRect(sel, QColor(255, 0, 0, 20));
        }
    }

    if (!sel.isNull()) {
        QPen pen(QColor(255, 0, 0), 2);
        p.setPen(pen);
        p.drawRect(sel.adjusted(0, 0, -1, -1));
    }
}

void ScrollRegionSelector::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_start = event->position().toPoint();
#else
        m_start = event->pos();
#endif
        m_end = m_start;
        m_selecting = true;
        update();
    }
}

void ScrollRegionSelector::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_selecting) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_end = event->position().toPoint();
#else
    m_end = event->pos();
#endif
    update();
}

void ScrollRegionSelector::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_selecting) {
        m_selecting = false;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_end = event->position().toPoint();
#else
        m_end = event->pos();
#endif

        emit selectionFinished(currentRect());
    }
}

void ScrollRegionSelector::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        emit selectionCanceled();
        return;
    }

    QWidget::keyPressEvent(event);
}
