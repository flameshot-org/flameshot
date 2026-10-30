// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "applaunchertool.h"
#include "tools/launcher/applauncherwidget.h"
#include "utils/confighandler.h"
#include <QApplication>

AppLauncher::AppLauncher(QObject* parent)
  : AbstractActionTool(parent)
{}

bool AppLauncher::closeOnButtonPressed() const
{
    return true;
}

QIcon AppLauncher::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "open_with.svg");
}
QString AppLauncher::name() const
{
    return tr("App Launcher");
}

CaptureTool::Type AppLauncher::type() const
{
    return CaptureTool::TYPE_OPEN_APP;
}

QString AppLauncher::description() const
{
    if (!ConfigHandler().defaultApp().isEmpty()) {
        return tr("Open with default app (Alt+Click to choose app)");
    }
    return tr("Choose an app to open the capture");
}

QWidget* AppLauncher::widget()
{
    return new AppLauncherWidget(capture);
}

CaptureTool* AppLauncher::copy(QObject* parent)
{
    return new AppLauncher(parent);
}

void AppLauncher::pressed(CaptureContext& context)
{
    capture = context.selectedScreenshotArea();
    emit requestAction(REQ_CAPTURE_DONE_OK);

    QString defaultApp = ConfigHandler().defaultApp();
    bool altPressed = qApp->keyboardModifiers().testFlag(Qt::AltModifier);

    if (!defaultApp.isEmpty() && !altPressed) {
        AppLauncherWidget::launchWithCommand(
          capture, defaultApp, ConfigHandler().defaultAppInTerminal());
    } else {
        emit requestAction(REQ_ADD_EXTERNAL_WIDGETS);
    }
    emit requestAction(REQ_CLOSE_GUI);
}
