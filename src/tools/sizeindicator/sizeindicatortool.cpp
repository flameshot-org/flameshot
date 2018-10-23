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

#include "sizeindicatortool.h"
#include <QPainter>

SizeIndicatorTool::SizeIndicatorTool(QObject *parent) : AbstractActionTool(parent) {

}

bool SizeIndicatorTool::closeOnButtonPressed() const {
    return false;
}

QIcon SizeIndicatorTool::icon(const QColor &background, bool inEditor) const {
    return inEditor ? QIcon() :
                      QIcon(iconPath(background) + "size_indicator.svg");
}
QString SizeIndicatorTool::name() const {
    return tr("Selection Size Indicator");
}

QString SizeIndicatorTool::nameID() {
    return QLatin1String("");
}

QString SizeIndicatorTool::description() const {
    return tr("Show the dimensions of the selection (X Y)");
}

CaptureTool* SizeIndicatorTool::copy(QObject *parent) {
    return new SizeIndicatorTool(parent);
}

void SizeIndicatorTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}
