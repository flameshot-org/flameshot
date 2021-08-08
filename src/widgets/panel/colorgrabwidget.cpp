
#include "colorgrabwidget.h"
#include "sidepanelwidget.h"

#include <QPainter>
#include <QKeyEvent>

ColorGrabWidget::ColorGrabWidget(QPixmap* p, QWidget *parent)
    : QWidget(parent)
    , m_pixmap(p)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::BypassWindowManagerHint |
                   Qt::FramelessWindowHint |
                   Qt::Tool);
    setMouseTracking(true);
}

void ColorGrabWidget::startGrabbing()
{
    m_ignoreFirstMouseRelease = true;
    show();
    grabMouse(Qt::CrossCursor);
    // FIXME: CaptureWidget shortcuts are still active
    grabKeyboard();
}

QColor ColorGrabWidget::color()
{
    return m_color;
}

void ColorGrabWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    // TODO
}

void ColorGrabWidget::hideEvent(QHideEvent*)
{
    releaseMouse();
    releaseKeyboard();
}

void ColorGrabWidget::mousePressEvent(QMouseEvent* e)
{
    // TODO
    QWidget::mousePressEvent(e);
}

void ColorGrabWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_ignoreFirstMouseRelease) {
        m_ignoreFirstMouseRelease = false;
        return;
    }
    emit colorGrabbed(getColorAtPoint(e->pos()));
    hide();
}

void ColorGrabWidget::mouseMoveEvent(QMouseEvent *e)
{
    m_color = getColorAtPoint(e->pos());
    emit colorUpdated(m_color);
    QWidget::mouseMoveEvent(e);
}

void ColorGrabWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        emit grabAborted();
        hide();
    } else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        emit colorGrabbed(m_color);
        hide();
    }
    e->accept();
}

QColor ColorGrabWidget::getColorAtPoint(const QPoint& p)
{
    if (m_pixmap) {
#if defined(Q_OS_MACOS)
        QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
        QPoint point = p;
        if (currentScreen) {
            point = QPoint((p.x() - currentScreen->geometry().x()) *
                             currentScreen->devicePixelRatio(),
                           (p.y() - currentScreen->geometry().y()) *
                             currentScreen->devicePixelRatio());
        }
        QPixmap pixel = m_pixmap->copy(QRect(point, point));
#else
        QPixmap pixel = m_pixmap->copy(QRect(p, p));
#endif
        return pixel.toImage().pixel(0, 0);
    }
    return QColor();
}
