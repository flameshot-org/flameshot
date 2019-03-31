// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

#include "capturetool.h"

class AbstractTwoPointTool : public CaptureTool {
    Q_OBJECT
public:
    explicit AbstractTwoPointTool(QObject *parent = nullptr);

    bool isValid() const override;
    bool closeOnButtonPressed() const override;
    bool isSelectable() const override;
    bool showMousePreview() const override;

    void undo(QPixmap &pixmap) override;

public slots:
    void drawEnd(const QPoint &p) override;
    void drawMove(const QPoint &p) override;
    void drawMoveWithAdjustment(const QPoint &p) override;
    void colorChanged(const QColor &c) override;
    void thicknessChanged(const int th) override;

protected:
    void updateBackup(const QPixmap &pixmap);
    QRect backupRect(const QRect &limits) const;

    QPixmap m_pixmapBackup;
    QPair<QPoint, QPoint> m_points;
    QColor m_color;
    int m_thickness;
    // use m_padding to extend the area of the backup
    int m_padding;

    bool m_supportsOrthogonalAdj = false;
    bool m_supportsDiagonalAdj = false;

private:
    QPoint adjustedVector(QPoint v) const;
};
