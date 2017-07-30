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

#include "src/capture/capturebutton.h"
#include <QObject>
#include <QList>

class QSettings;

class ConfigHandler : public QObject
{
    Q_OBJECT
public:
    explicit ConfigHandler(QObject *parent = nullptr);

    QList<CaptureButton::ButtonType> getButtons();
    void setButtons(const QList<CaptureButton::ButtonType> &);

    QString getSavePath();
    void setSavePath(const QString &);

    QColor getUIMainColor();
    void setUIMainColor(const QColor &);

    QColor getUIContrastColor();
    void setUIContrastColor(const QColor &);

    QColor getDrawColor();
    void setDrawColor(const QColor &);

    bool getShowHelp();
    void setShowHelp(const bool);

    bool getDesktopNotification();
    void setDesktopNotification(const bool);

    QString getFilenamePattern();
    void setFilenamePattern(const QString &);

    bool getDisabledTrayIcon();
    void setDisabledTrayIcon(const bool);

    bool initiatedIsSet();
    void setInitiated();
    void setNotInitiated();
    void setDefaults();

    void setAllTheButtons();

private:
    QSettings *m_settings;

    bool normalizeButtons(QList<int> &);

    QList<CaptureButton::ButtonType> fromIntToButton(const QList<int> &l);
    QList<int> fromButtonToInt(const QList<CaptureButton::ButtonType> &l);

};

#endif // CONFIGHANDLER_H
