#include "screencolorpickertool.h"
#include <qapplication.h>
#include <qclipboard.h>


ScreenColorPickerTool::ScreenColorPickerTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool ScreenColorPickerTool::closeOnButtonPressed() const
{
    return true;
}

QIcon ScreenColorPickerTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "content-copy.svg");
}
QString ScreenColorPickerTool::name() const
{
    return tr("ScreenColorPicker");
}

CaptureTool::Type ScreenColorPickerTool::type() const
{
    return CaptureTool::TYPE_COPY;
}

QString ScreenColorPickerTool::description() const
{
    return tr("Copy the color under your pointer to clipboard");
}

CaptureTool* ScreenColorPickerTool::copy(QObject* parent)
{
    return new ScreenColorPickerTool(parent);
}

void ScreenColorPickerTool::pressed(CaptureContext& context)
{
    QPixmap pixmap = context.screenshot;
    QPoint mousePoint = context.mousePos;

    QColor color = pixmap.toImage().pixelColor(mousePoint);
    QApplication::clipboard()->setText(color.name());
}

