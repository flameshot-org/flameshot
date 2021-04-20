// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "pintool.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/tools/pin/pinwidget.h"
#include <QScreen>

PinTool::PinTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool PinTool::closeOnButtonPressed() const
{
    return true;
}

QIcon PinTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "pin.svg");
}
QString PinTool::name() const
{
    return tr("Pin Tool");
}

ToolType PinTool::nameID() const
{
    return ToolType::PIN;
}

QString PinTool::description() const
{
    return tr("Pin image on the desktop");
}

QWidget* PinTool::widget()
{
    qreal devicePixelRatio = 1;
#if defined(Q_OS_MACOS)
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    if (currentScreen) {
        devicePixelRatio = currentScreen->devicePixelRatio();
    }
#endif
    PinWidget* w = new PinWidget(m_pixmap);
    const int m = static_cast<int>(w->margin() * devicePixelRatio);
    QRect adjusted_pos = m_geometry + QMargins(m, m, m, m);
    w->setGeometry(adjusted_pos);
#if defined(Q_OS_MACOS)
    if (currentScreen) {
        QPoint topLeft = currentScreen->geometry().topLeft();
        adjusted_pos.setX((adjusted_pos.x() - topLeft.x()) / devicePixelRatio +
                          topLeft.x());

        adjusted_pos.setY((adjusted_pos.y() - topLeft.y()) / devicePixelRatio +
                          topLeft.y());
        adjusted_pos.setWidth(adjusted_pos.size().width() / devicePixelRatio);
        adjusted_pos.setHeight(adjusted_pos.size().height() / devicePixelRatio);
        w->resize(0, 0);
        w->move(adjusted_pos.x(), adjusted_pos.y());
    }
#endif
    return w;
}

CaptureTool* PinTool::copy(QObject* parent)
{
    return new PinTool(parent);
}

void PinTool::pressed(const CaptureContext& context)
{
    emit requestAction(REQ_CLEAR_SELECTION);
    emit requestAction(REQ_CAPTURE_DONE_OK);
    m_geometry = context.selection;
    m_geometry.setTopLeft(m_geometry.topLeft() + context.widgetOffset);
    m_pixmap = context.selectedScreenshotArea();
    emit requestAction(REQ_ADD_EXTERNAL_WIDGETS);
}
