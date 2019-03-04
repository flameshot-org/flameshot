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

class AbstractActionTool : public CaptureTool {
    Q_OBJECT
public:
    explicit AbstractActionTool(QObject *parent = nullptr);

    bool isValid() const override;
    bool isSelectable() const override;
    bool showMousePreview() const override;

    void undo(QPixmap &pixmap) override;
    void process(QPainter &painter, const QPixmap &pixmap, bool recordUndo = false) override;
    void paintMousePreview(QPainter &painter, const CaptureContext &context) override;

public slots:
    void drawEnd(const QPoint &p) override;
    void drawMove(const QPoint &p) override;
    void drawStart(const CaptureContext &context) override;
    void colorChanged(const QColor &c) override;
    void thicknessChanged(const int th) override;
};
