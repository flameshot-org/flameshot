// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "sizeindicatortool.h"
#include <QPainter>

SizeIndicatorTool::SizeIndicatorTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool SizeIndicatorTool::closeOnButtonPressed() const
{
    return false;
}

QIcon SizeIndicatorTool::icon(const QColor& background, bool inEditor) const
{
    return inEditor ? QIcon()
                    : QIcon(iconPath(background) + "size_indicator.svg");
}
QString SizeIndicatorTool::name() const
{
    return tr("Selection Size Indicator");
}

CaptureTool::Type SizeIndicatorTool::type() const
{
    return CaptureTool::TYPE_SELECTIONINDICATOR;
}

QString SizeIndicatorTool::description() const
{
    return tr("Show X and Y dimensions of the selection");
}

CaptureTool* SizeIndicatorTool::copy(QObject* parent)
{
    return new SizeIndicatorTool(parent);
}

void SizeIndicatorTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
