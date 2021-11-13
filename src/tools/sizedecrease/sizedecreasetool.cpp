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

#include "sizedecreasetool.h"
#include <QPainter>

SizeDecreaseTool::SizeDecreaseTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool SizeDecreaseTool::closeOnButtonPressed() const
{
    return false;
}

QIcon SizeDecreaseTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "minus.svg");
}
QString SizeDecreaseTool::name() const
{
    return tr("Decrease Tool Size");
}

CaptureTool::Type SizeDecreaseTool::type() const
{
    return CaptureTool::TYPE_SIZEDECREASE;
}

QString SizeDecreaseTool::description() const
{
    return tr("Decrease the size of the other tools");
}

CaptureTool* SizeDecreaseTool::copy(QObject* parent)
{
    return new SizeDecreaseTool(parent);
}

void SizeDecreaseTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
    emit requestAction(REQ_DECREASE_TOOL_SIZE);
}
