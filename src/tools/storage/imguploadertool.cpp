#include "imguploadertool.h"

ImgUploaderTool::ImgUploaderTool(QObject* parent)
  : AbstractActionTool(parent)
{}

void ImgUploaderTool::setCapture(const QPixmap& pixmap)
{
    m_capture = pixmap;
}

void ImgUploaderTool::pressed(const CaptureContext& context)
{
    m_capture = context.selectedScreenshotArea();
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_ADD_EXTERNAL_WIDGETS);
}

QString ImgUploaderTool::name() const
{
    return tr("Image uploader tool");
}

const QPixmap& ImgUploaderTool::capture()
{
    return m_capture;
}

QIcon ImgUploaderTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "cloud-upload.svg");
}

bool ImgUploaderTool::closeOnButtonPressed() const
{
    return true;
}

ToolType ImgUploaderTool::nameID() const
{
    return ToolType::UPLOAD;
}