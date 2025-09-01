#ifndef DESKTOP_CAPTURER_H
#define DESKTOP_CAPTURER_H

#include <QObject>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QScreen>

/**
 * @class DesktopCapturer
 * @brief Provides functionality to capture screenshots of the desktop.
 *
 * This class can capture either a composite screenshot of all screens
 * or a screenshot of the single screen where the mouse cursor is located.
 */
class DesktopCapturer : public QObject {
    Q_OBJECT

public:
    DesktopCapturer();
    ~DesktopCapturer() = default;

    /**
     * @brief Resets the internal geometry data.
     */
    void reset();

    /**
     * @brief Gets the size of the captured desktop area.
     * @return The size as a QSize object.
     */
    QSize screenSize() const;

    /**
     * @brief Gets the top-left coordinate of the captured desktop area.
     * @return The top-left point as a QPoint object.
     */
    QPoint topLeft() const;

    QPoint topLeftScaledToScreen() const;

    /**
     * @brief Calculates and returns the geometry of the entire virtual desktop.
     * @return The geometry as a QRect object.
     */
    QRect geometry();

    /**
     * @brief Captures a composite screenshot of all desktops.
     * @return A QPixmap containing the combined desktop image.
     */
    QPixmap captureDesktopComposite();

    /**
     * @brief Captures a screenshot of the desktop at the mouse cursor's position.
     * @return A QPixmap of the screen where the cursor is located.
     */
    QPixmap captureDesktopAtCursorPos();

    /**
     * @brief Main function to capture the desktop based on the composite flag.
     * @param composite If true, captures a composite screenshot; otherwise, captures the screen at the cursor.
     * @return The captured QPixmap.
     */
    QPixmap captureDesktop(bool composite = true);

    QScreen* screenToDraw() const;

    const QList<QRect>& areas() const;

private:
    /**
     * @brief Finds the QScreen instance where the mouse cursor is currently located.
     * @return A pointer to the QScreen object.
     */
    QScreen* screenAtCursorPos();

    QRect           m_geometry;
    QScreen*        m_screenToDraw;
    QVector<QRect>  m_areas;
};

#endif // DESKTOP_CAPTURER_H
