// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "accepttool.h"
#include "src/utils/screenshotsaver.h"
#include <QApplication>
#include <QPainter>
#include <QStyle>
#if defined(Q_OS_MACOS)
#include "src/widgets/capture/capturewidget.h"
#include <QApplication>
#include <QWidget>
#endif

AcceptTool::AcceptTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool AcceptTool::closeOnButtonPressed() const
{
    return true;
}

QIcon AcceptTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    // TODO add new icon
    return qApp->style()->standardIcon(QStyle::SP_DialogApplyButton);
}

QString AcceptTool::name() const
{
    return tr("Accept");
}

CaptureTool::Type AcceptTool::type() const
{
    return CaptureTool::TYPE_ACCEPT;
}

QString AcceptTool::description() const
{
    // TODO better message
    return tr("Accept the capture");
}

CaptureTool* AcceptTool::copy(QObject* parent)
{
    return new AcceptTool(parent);
}

void AcceptTool::pressed(const CaptureContext&)
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
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_CLOSE_GUI);
}
