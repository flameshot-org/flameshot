// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "cursor.h"
#include "colorutils.h"
#include <QPainter>

namespace
{
    static constexpr int PADDING_VALUE = 2;
    static constexpr int THICKNESS_OFFSET = 8;
}

CursorTool::CursorTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{}

QIcon CursorTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "mouse.svg");
}

QString CursorTool::info()
{
    return {};
}

bool CursorTool::isValid() const
{
    return true;
}

QRect CursorTool::mousePreviewRect(const CaptureContext& context) const
{
    int width = (context.toolSize + THICKNESS_OFFSET) * 2;
    QRect rect(0, 0, width, width);
    rect.moveCenter(context.mousePos);
    return rect;
}

#include <QCursor>

QRect CursorTool::boundingRect() const
{
    const int bubble_size = size() + THICKNESS_OFFSET + PADDING_VALUE;
    const QPoint first = points().first;
    return QRect(first.x(),
                 first.y(),
                 bubble_size * 2,
                 bubble_size * 2);
}

QString CursorTool::name() const
{
    return tr("Mouse Cursor");
}

CaptureTool::Type CursorTool::type() const
{
    return CaptureTool::TYPE_CURSOR;
}

void CursorTool::copyParams(const CursorTool* from,
                                 CursorTool* to)
{
    AbstractTwoPointTool::copyParams(from, to);
}

QString CursorTool::description() const
{
    return tr("Include the mouse cursor into the screenshot");
}

CaptureTool* CursorTool::copy(QObject* parent)
{
    auto* tool = new CursorTool(parent);
    copyParams(this, tool);
    return tool;
}

void CursorTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap);
    const QImage img(":/img/app/left_ptr.svg");
    painter.drawImage(boundingRect(), img);
}

void CursorTool::paintMousePreview(QPainter& painter,
                                   const CaptureContext& context)
{
    Q_UNUSED(painter);
    Q_UNUSED(context);
}

void CursorTool::drawStart(const CaptureContext& context)
{
    AbstractTwoPointTool::drawStart(context);
}

void CursorTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
