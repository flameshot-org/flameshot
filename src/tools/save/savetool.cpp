// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "savetool.h"
#include "src/utils/screenshotsaver.h"
#include <QApplication> // TODO rm
#include <QPainter>
#if defined(Q_OS_MACOS)
#include "src/widgets/capture/capturewidget.h"
#include <QApplication>
#include <QWidget>
#endif

SaveTool::SaveTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool SaveTool::closeOnButtonPressed() const
{
    return true;
}

QIcon SaveTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "content-save.svg");
}
QString SaveTool::name() const
{
    return tr("Save");
}

CaptureTool::Type SaveTool::type() const
{
    return CaptureTool::TYPE_SAVE;
}

QString SaveTool::description() const
{
    return tr("Save screenshot to a file");
}

CaptureTool* SaveTool::copy(QObject* parent)
{
    return new SaveTool(parent);
}

void SaveTool::pressed(CaptureContext& context)
{
    context.request()->addSaveTask();
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_CLOSE_GUI);
}
