#include "capturemodification.h"
#include <QSettings>
#include <QColor>

// CaptureModification is a single modification in the screenshot drawn
// by the user.

CaptureModification::CaptureModification(
        const Button::Type t, QPoint p) : m_type(t) {
    m_coords.append(p);
    if (m_type == Button::Type::circle || m_type == Button::Type::rectangle
         || m_type == Button::Type::arrow || m_type == Button::Type::line ||
            m_type == Button::Type::marker) {
        m_coords.append(p);
    }
    QSettings settings;
    m_color = settings.value("drawColor").value<QColor>();
}

CaptureModification::CaptureModification() {

}

Button::Type CaptureModification::getType() const {
    return m_type;
}

QColor CaptureModification::getColor() const {
    return m_color;
}

QVector<QPoint> CaptureModification::getPoints() const {
    return m_coords;
}

void CaptureModification::addPoint(const QPoint p) {
    if (m_type == Button::Type::circle || m_type == Button::Type::rectangle
         || m_type == Button::Type::arrow || m_type == Button::Type::line ||
            m_type == Button::Type::marker) {
        m_coords[1] = p;
    } else {
        m_coords.append(p);
    }
}
