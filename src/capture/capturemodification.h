// Copyright 2017 Alejandro Sirgo Rica
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

#ifndef CAPTURECHANGE_H
#define CAPTURECHANGE_H

#include "capturebutton.h"
#include <QObject>
#include <QVector>

class CaptureButton;
class QPoint;

class CaptureModification : public QObject {
    Q_OBJECT
public:
    CaptureModification(QObject *parent = nullptr) = delete;
    CaptureModification(
            const CaptureButton::ButtonType,
            const QPoint &,
            const QColor &,
            QObject *parent = nullptr
            );
    QColor getColor() const;
    QVector<QPoint> getPoints() const;
    CaptureTool* getTool() const;
    CaptureButton::ButtonType getType() const;
    void addPoint(const QPoint);

protected:
    QColor m_color;
    CaptureButton::ButtonType m_type;
    QVector<QPoint> m_coords;
    CaptureTool *m_tool;

private:

};

#endif // CAPTURECHANGE_H
