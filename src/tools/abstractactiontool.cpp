// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "abstractactiontool.h"

AbstractActionTool::AbstractActionTool(QObject* parent)
  : CaptureTool(parent)
{}

bool AbstractActionTool::isValid() const
{
    return true;
}

bool AbstractActionTool::isSelectable() const
{
    return false;
}

bool AbstractActionTool::showMousePreview() const
{
    return false;
}

void AbstractActionTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(painter)
    Q_UNUSED(pixmap)
}

void AbstractActionTool::paintMousePreview(QPainter& painter,
                                           const CaptureContext& context)
{
    Q_UNUSED(painter)
    Q_UNUSED(context)
}

void AbstractActionTool::drawEnd(const QPoint& p)
{
    Q_UNUSED(p)
}

void AbstractActionTool::drawMove(const QPoint& p)
{
    Q_UNUSED(p)
}

void AbstractActionTool::drawStart(const CaptureContext& context)
{
    Q_UNUSED(context)
}

void AbstractActionTool::colorChanged(const QColor& c)
{
    Q_UNUSED(c)
}

void AbstractActionTool::thicknessChanged(int th)
{
    Q_UNUSED(th)
}
