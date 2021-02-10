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
    Q_UNUSED(inEditor);
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
#if (defined(Q_OS_MAC64) || defined(Q_OS_MACOS) || defined(Q_OS_MACX))
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    if (currentScreen) {
        devicePixelRatio = currentScreen->devicePixelRatio();
    }
#endif
    PinWidget* w = new PinWidget(m_pixmap);
    const int m = w->margin() * devicePixelRatio;
    QRect adjusted_pos = m_geometry + QMargins(m, m, m, m);
    w->setGeometry(adjusted_pos);
#if (defined(Q_OS_MAC64) || defined(Q_OS_MACOS) || defined(Q_OS_MACX))
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
    emit requestAction(REQ_CAPTURE_DONE_OK);
    m_geometry = context.selection;
    m_geometry.setTopLeft(m_geometry.topLeft() + context.widgetOffset);
    m_pixmap = context.selectedScreenshotArea();
    emit requestAction(REQ_ADD_EXTERNAL_WIDGETS);
}
