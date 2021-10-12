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

#include "sizeincreasetool.h"
#include <QPainter>

SizeIncreaseTool::SizeIncreaseTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool SizeIncreaseTool::closeOnButtonPressed() const
{
    return false;
}

QIcon SizeIncreaseTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "plus.svg");
}
QString SizeIncreaseTool::name() const
{
    return tr("Increase Tool Size");
}

CaptureTool::Type SizeIncreaseTool::type() const
{
    return CaptureTool::TYPE_SIZEINCREASE;
}

QString SizeIncreaseTool::description() const
{
    return tr("Increase the size of the other tools");
}

CaptureTool* SizeIncreaseTool::copy(QObject* parent)
{
    return new SizeIncreaseTool(parent);
}

void SizeIncreaseTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
    emit requestAction(REQ_INCREASE_TOOL_SIZE);
}
