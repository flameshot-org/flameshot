// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Ouyang Chunhui & Contributor

#include "save-to-pdf.h"
#include "src/utils/screenshotsaver.h"
#include <QPainter>

SaveToPDFTool::SaveToPDFTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool SaveToPDFTool::closeOnButtonPressed() const
{
    return true;
}

QIcon SaveToPDFTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "save-to-pdf.svg");
}
QString SaveToPDFTool::name() const
{
    return tr("Save To PDF");
}

CaptureTool::Type SaveToPDFTool::type() const
{
    return CaptureTool::TYPE_PRINT;
}

QString SaveToPDFTool::description() const
{
    return tr("Save To PDF");
}

CaptureTool* SaveToPDFTool::copy(QObject* parent)
{
    return new SaveToPDFTool(parent);
}

void SaveToPDFTool::pressed(CaptureContext& context)
{
    emit requestAction(REQ_CLEAR_SELECTION);
    context.request.addTask(CaptureRequest::SAVE_TO_PDF);
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_CLOSE_GUI);
}
