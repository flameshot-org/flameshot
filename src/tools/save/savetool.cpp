// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

#include "savetool.h"
#include "src/utils/screenshotsaver.h"
#include <QPainter>
#if (defined(Q_OS_MAC64) || defined(Q_OS_MACOS) || defined(Q_OS_MACX))
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
    Q_UNUSED(inEditor);
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
#if (defined(Q_OS_MAC64) || defined(Q_OS_MACOS) || defined(Q_OS_MACX))
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
