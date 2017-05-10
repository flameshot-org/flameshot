#ifndef CAPTURECHANGE_H
#define CAPTURECHANGE_H

#include "button.h"
#include <QVector>

class QPoint;

class CaptureModification {
public:
    CaptureModification();
    CaptureModification(const Button::Type, const QPoint);
    Button::Type getType() const;
    QColor getColor() const;
    QVector<QPoint> getPoints() const;
    void addPoint(const QPoint);
private:
    QVector<QPoint> m_coords;
    QColor m_color;
    Button::Type m_type;

};

#endif // CAPTURECHANGE_H
