// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

#pragma once

#include "src/capture/widgets/capturebutton.h"
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
            const QPoint &initialPoint,
            const QColor &color,
            const int thickness,
            QObject *parent = nullptr
            );
    QColor color() const;
    QVector<QPoint> points() const;
    CaptureTool* tool() const;
    int thickness() const;
    CaptureButton::ButtonType buttonType() const;
    void addPoint(const QPoint);

protected:
    QColor m_color;
    CaptureButton::ButtonType m_type;
    QVector<QPoint> m_coords;
    CaptureTool *m_tool;
    int m_thickness;

};
