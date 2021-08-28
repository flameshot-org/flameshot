// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "inverttool.h"
#include "src/utils/screenshotsaver.h"
#include <QPainter>
#include <QPixmap>
#include <QImage>
#if defined(Q_OS_MACOS)
#include "src/widgets/capture/capturewidget.h"
#include <QApplication>
#include <QWidget>
#endif

InvertTool::InvertTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool InvertTool::closeOnButtonPressed() const
{
    return true;
}

QIcon InvertTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "invert.svg");
}

QString InvertTool::name() const
{
    return tr("Invert");
}

ToolType InvertTool::type() const
{
    return ToolType::INVERT;
}

QString InvertTool::description() const
{
    return tr("Save the inverted capture");
}

CaptureTool* InvertTool::copy(QObject* parent)
{
    return new InvertTool(parent);
}

void InvertTool::pressed(const CaptureContext& context)
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

    QPixmap inverted = context.selectedScreenshotArea();
    QImage img = inverted.toImage();
    img.invertPixels();
    inverted.convertFromImage(img);

    if (context.savePath.isEmpty()) {
        emit requestAction(REQ_HIDE_GUI);
        bool ok = ScreenshotSaver().saveToFilesystemGUI(
          inverted);
        if (ok) {
            emit requestAction(REQ_CAPTURE_DONE_OK);
        }
    } else {
        bool ok = ScreenshotSaver().saveToFilesystem(
          inverted, context.savePath);
        if (ok) {
            emit requestAction(REQ_CAPTURE_DONE_OK);
        }
    }
}
