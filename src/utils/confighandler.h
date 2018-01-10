// Copyright 2017 Alejandro Sirgo Rica
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

#ifndef CONFIGHANDLER_H
#define CONFIGHANDLER_H

#include "src/capture/widget/capturebutton.h"
#include <QList>
#include <QSettings>

class ConfigHandler
{
public:
    explicit ConfigHandler();

    QList<CaptureButton::ButtonType> getButtons();
    void setButtons(const QList<CaptureButton::ButtonType> &);

    QString savePathValue();
    void setSavePath(const QString &);

    QColor uiMainColorValue();
    void setUIMainColor(const QColor &);

    QColor uiContrastColorValue();
    void setUIContrastColor(const QColor &);

    QColor drawColorValue();
    void setDrawColor(const QColor &);

    bool showHelpValue();
    void setShowHelp(const bool);

    bool desktopNotificationValue();
    void setDesktopNotification(const bool);

    QString filenamePatternValue();
    void setFilenamePattern(const QString &);

    bool disabledTrayIconValue();
    void setDisabledTrayIcon(const bool);

    int drawThicknessValue();
    void setdrawThickness(const int);

    bool keepOpenAppLauncherValue();
    void setKeepOpenAppLauncher(const bool);

    bool startupLaunchValue();
    void setStartupLaunch(const bool);

    int contrastOpacityValue();
    void setContrastOpacity(const int);

    void setDefaults();
    void setAllTheButtons();

    QString configFilePath() const;

private:
    QSettings m_settings;

    bool normalizeButtons(QList<int> &);

    QList<CaptureButton::ButtonType> fromIntToButton(const QList<int> &l);
    QList<int> fromButtonToInt(const QList<CaptureButton::ButtonType> &l);

};

#endif // CONFIGHANDLER_H
