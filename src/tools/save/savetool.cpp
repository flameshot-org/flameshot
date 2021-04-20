// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "savetool.h"
#include "src/utils/screenshotsaver.h"
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

ToolType SaveTool::nameID() const
{
    return ToolType::SAVE;
}

QString SaveTool::description() const
{
    return tr("Save the capture");
}

CaptureTool* SaveTool::copy(QObject* parent)
{
    return new SaveTool(parent);
}

void SaveTool::pressed(const CaptureContext& context)
{
#if defined(Q_OS_MACOS)
    for (QWidget* widget : qApp->topLevelWidgets()) {
        QString className(widget->metaObject()->className());
        if (0 ==
            className.compare(CaptureWidget::staticMetaObject.className())) {
            widget->showNormal();
            widget->hide();
            break;
        }
    }
#endif
    emit requestAction(REQ_CLEAR_SELECTION);
    if (context.savePath.isEmpty()) {
        emit requestAction(REQ_HIDE_GUI);
        bool ok = ScreenshotSaver().saveToFilesystemGUI(
          context.selectedScreenshotArea());
        if (ok) {
            emit requestAction(REQ_CAPTURE_DONE_OK);
        }
    } else {
        bool ok = ScreenshotSaver().saveToFilesystem(
          context.selectedScreenshotArea(), context.savePath, "");
        if (ok) {
            emit requestAction(REQ_CAPTURE_DONE_OK);
        }
    }
}
