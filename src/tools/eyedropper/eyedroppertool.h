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

#include "src/tools/capturetool.h"

class EyedropperWidget;

class EyedropperTool : public CaptureTool
{
  Q_OBJECT;
public:
  explicit EyedropperTool(QObject* parent = nullptr);
  ~EyedropperTool() override;

  bool isValid() const override;
  bool closeOnButtonPressed() const override;
  bool isSelectable() const override;
  bool showMousePreview() const override;

  QIcon icon(const QColor& background, bool inEditor) const override;
  QString name() const override;
  ToolType nameID() const override;
  QString description() const override;
  CaptureTool* copy(QObject* parent) override;
  void process(QPainter& painter, const QPixmap& pixmap, bool recordUndo) override;
  void paintMousePreview(QPainter& painter, const CaptureContext& context) override;
  void undo(QPixmap& pixmap) override;

public slots:
  void drawEnd(const QPoint& p) override;
  void drawMove(const QPoint& p) override;
  void drawStart(const CaptureContext& context) override;
  void pressed(const CaptureContext& context) override;
  void colorChanged(const QColor& c) override;
  void thicknessChanged(const int th) override;
  QWidget* widget() override;

private:
  EyedropperWidget* m_widget { nullptr };
};