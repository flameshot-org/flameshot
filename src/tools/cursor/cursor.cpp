#include "cursor.h"
#include "colorutils.h"

#include <QPainter>
#include <QCursor>
#include <abstractlogger.h>

CursorTool::CursorTool(QObject* parent)
  : CaptureTool(parent)
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
    AbstractLogger::info() << __PRETTY_FUNCTION__;
    return {};
}

QRect CursorTool::boundingRect() const
{
    AbstractLogger::info() << __PRETTY_FUNCTION__;
    return QRect(QCursor::pos(), QPoint(24, 24));
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
    CaptureTool::copyParams(from, to);
}

QString CursorTool::description() const
{
    return tr("Includes the mouse cursor into the screenshot");
}

CaptureTool* CursorTool::copy(QObject* parent)
{
    auto* tool = new CursorTool(parent);
    copyParams(this, tool);
    return tool;
}

void CursorTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(painter)
    Q_UNUSED(pixmap)
    AbstractLogger::info() << __PRETTY_FUNCTION__;
}

void CursorTool::paintMousePreview(QPainter& painter,
                                   const CaptureContext& context)
{
    AbstractLogger::info() << __PRETTY_FUNCTION__;
}

void CursorTool::drawStart(const CaptureContext& context)
{
    Q_UNUSED(context);
    AbstractLogger::info() << __PRETTY_FUNCTION__;
}

bool CursorTool::closeOnButtonPressed() const
{
    AbstractLogger::info() << __PRETTY_FUNCTION__;
    return false;
}

bool CursorTool::isSelectable() const
{
    return true;
}

bool CursorTool::showMousePreview() const
{
    return false;
}

void CursorTool::drawEnd(const QPoint & p)
{
    AbstractLogger::info() << __PRETTY_FUNCTION__;
}

void CursorTool::drawMove(const QPoint & p)
{
    AbstractLogger::info() << __PRETTY_FUNCTION__;
}

void CursorTool::pressed(CaptureContext & context)
{
    AbstractLogger::info() << __PRETTY_FUNCTION__;
}

void CursorTool::onColorChanged(const QColor & c)
{
    AbstractLogger::info() << __PRETTY_FUNCTION__;
}

void CursorTool::onSizeChanged(int size)
{
    AbstractLogger::info() << __PRETTY_FUNCTION__;
}
