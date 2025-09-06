#include "DesktopCapturer.h"

#include <QCursor>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QScreen>

DesktopCapturer::DesktopCapturer()
  : m_screenToDraw(nullptr)
{
    reset();
}

void DesktopCapturer::reset()
{
    m_geometry = QRect(0, 0, 0, 0);
    m_areas.clear();
}

QSize DesktopCapturer::screenSize() const
{
    return m_geometry.size();
}

QPoint DesktopCapturer::topLeft() const
{
    return m_geometry.topLeft();
}

QPoint DesktopCapturer::topLeftScaledToScreen() const
{
    return screenToDraw()->geometry().topLeft() /
           screenToDraw()->devicePixelRatio();
}

QRect DesktopCapturer::geometry()
{
    // Get Top Left and Bottom Right
    QPoint maxPoint(INT_MIN, INT_MIN);
    QPoint topLeft = QPoint(INT_MAX, INT_MAX);
    for (QScreen const* screen : QGuiApplication::screens()) {
        QRect geo = screen->geometry();
        int const width =
          static_cast<int>(geo.width() * screen->devicePixelRatio());
        int const height =
          static_cast<int>(geo.height() * screen->devicePixelRatio());
        int const maxX = width + geo.x();
        int const maxY = height + geo.y();

        // Get Top Left
        if (geo.x() < topLeft.x()) {
            topLeft.setX(geo.x());
        }
        if (geo.y() < topLeft.y()) {
            topLeft.setY(geo.y());
        }

        // Get Bottom Right
        if (maxX > maxPoint.x()) {
            maxPoint.setX(maxX);
        }
        if (maxY > maxPoint.y()) {
            maxPoint.setY(maxY);
        }
    }

    // Get Desktop size
    m_geometry.setX(topLeft.x());
    m_geometry.setY(topLeft.y());
    m_geometry.setWidth(maxPoint.x() - topLeft.x());
    m_geometry.setHeight(maxPoint.y() - topLeft.y());

    return m_geometry;
}

QPixmap DesktopCapturer::captureDesktopComposite()
{
    m_screenToDraw = lastScreen();
    qreal screenToDrawDpr = screenToDraw()->devicePixelRatio();

    // Calculate screen geometry
    geometry();

    // Create Desktop image
    QPixmap desktop(screenSize());
    desktop.fill(Qt::black);

    // Draw composite screenshot
    QPainter painter(&desktop);
    for (QScreen* screen : QGuiApplication::screens()) {
        QRect geo = screen->geometry();
        QPixmap pix = screen->grabWindow(0);

        // Composite screenshot should have pixel ratio 1 to draw all screen
        // with different ratios.
        pix.setDevicePixelRatio(1);

        // Calculate the offset of the current screen
        // from the top left corner of the composite screen
        geo.setX(geo.x() - topLeft().x());
        geo.setY(geo.y() - topLeft().y());

        painter.drawPixmap(geo.x(), geo.y(), pix);

        // Prepare areas
        // Everything, including location, should be in logical pixels
        // of the screen to draw a grabbed area (primary screen)
        QRect areaRect =
          QRect(static_cast<int>(geo.x() / screenToDrawDpr),
                static_cast<int>(geo.y() / screenToDrawDpr),
                static_cast<int>(pix.width() / screenToDrawDpr),
                static_cast<int>(pix.height() / screenToDrawDpr));
        m_areas.append(areaRect);
    }
    painter.end();

    // Set pixmap DevicePixelRatio of the screen where it should be drawn.
    desktop.setDevicePixelRatio(screenToDraw()->devicePixelRatio());

    return desktop;
}

QPixmap DesktopCapturer::captureDesktopAtCursorPos()
{
    // Active is where the mouse cursor is, it can be not an active screen
    QScreen* screen = screenAtCursorPos();
    QPixmap pix;
    if (screen == nullptr) {
        return pix;
    }
    pix = screen->grabWindow(0);
    m_geometry = screen->geometry();
    m_geometry.setWidth(
      static_cast<int>(m_geometry.width() * screen->devicePixelRatio()));
    m_geometry.setHeight(
      static_cast<int>(m_geometry.height() * screen->devicePixelRatio()));
    return pix;
}

QScreen* DesktopCapturer::screenAtCursorPos()
{
    // Get the current global position of the mouse cursor.
    // This position is in the virtual desktop coordinate system, which spans
    // all screens.
    const QPoint mousePos = QCursor::pos();
    m_screenToDraw = lastScreen();

    // Iterate through all screens available to the application.
    for (QScreen* screen : QGuiApplication::screens()) {
        // Get the screen's geometry in the virtual desktop coordinate system.
        // This is the rectangle that defines the screen's position and size.

        // Check if the screen's geometry contains the mouse cursor's position.
        if (screen->geometry().contains(mousePos)) {
            m_screenToDraw = screen;
            break;
        }
    }

    // Return nullptr if the cursor is not found on any screen.
    return m_screenToDraw;
}

QPixmap DesktopCapturer::captureDesktop(bool composite)
{
    QPixmap desktop;
    reset();
#ifdef Q_OS_MAC
    composite = false;
#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    // Wayland sessions cannot use composite
    // composite = false;
#endif
    if (composite) {
        desktop = captureDesktopComposite();
    } else {
        desktop = captureDesktopAtCursorPos();
    }
    return desktop;
}

QScreen* DesktopCapturer::screenToDraw() const
{
    return m_screenToDraw;
}

const QList<QRect>& DesktopCapturer::areas() const
{
    return m_areas;
}

QScreen* DesktopCapturer::lastScreen()
{
#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX))
    // At least in Gnome+XOrg, the last screen is actually the first screen
    // and all calculations are started from it, not from the PrimaryScreen.
    return QGuiApplication::screens().last();
#else
    return QGuiApplication::primaryScreen();
#endif
}
