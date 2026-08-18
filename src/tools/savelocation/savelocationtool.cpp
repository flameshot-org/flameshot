// SPDX-License-Identifier: GPL-3.0-or-later

#include "savelocationtool.h"
#include "utils/confighandler.h"

#include <QPainter>

SaveLocationTool::SaveLocationTool(int location, QObject* parent)
  : AbstractActionTool(parent)
  , m_location(location)
{}

bool SaveLocationTool::closeOnButtonPressed() const
{
    return true;
}

QIcon SaveLocationTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "content-save.svg");
}

QString SaveLocationTool::name() const
{
    return tr("Save to loc%1").arg(m_location);
}

QString SaveLocationTool::description() const
{
    return tr("Save to loc%1").arg(m_location);
}

CaptureTool::Type SaveLocationTool::type() const
{
    switch (m_location) {
        case 1:
            return CaptureTool::TYPE_SAVE_LOCATION_1;
        case 2:
            return CaptureTool::TYPE_SAVE_LOCATION_2;
        default:
            return CaptureTool::TYPE_SAVE_LOCATION_3;
    }
}

CaptureTool* SaveLocationTool::copy(QObject* parent)
{
    return new SaveLocationTool(m_location, parent);
}

void SaveLocationTool::pressed(CaptureContext& context)
{
    QString path;
    switch (m_location) {
        case 1:
            path = ConfigHandler().savePathLocation1();
            break;
        case 2:
            path = ConfigHandler().savePathLocation2();
            break;
        default:
            path = ConfigHandler().savePathLocation3();
            break;
    }

    emit requestAction(REQ_CLEAR_SELECTION);
    // An empty path falls back to the normal save-as dialog, same as
    // pressing Save with no configured location.
    context.request.addSaveTask(path);
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_CLOSE_GUI);
}
