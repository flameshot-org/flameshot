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

class AbstractPathTool : public CaptureTool {
    Q_OBJECT
public:
    explicit AbstractPathTool(QObject *parent = nullptr);

    bool isValid() const override;
    bool closeOnButtonPressed() const override;
    bool isSelectable() const override;
    bool showMousePreview() const override;

    void undo(QPixmap &pixmap) override;

public slots:
    void drawEnd(const QPoint &p) override;
    void drawMove(const QPoint &p) override;
    void colorChanged(const QColor &c) override;
    void thicknessChanged(const int th) override;

protected:
    void updateBackup(const QPixmap &pixmap);
    void addPoint(const QPoint &point);

    QPixmap m_pixmapBackup;
    QRect m_backupArea;
    QColor m_color;
    QVector<QPoint> m_points;
    int m_thickness;
    // use m_padding to extend the area of the backup
    int m_padding;
};
