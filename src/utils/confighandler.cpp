// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

#include "confighandler.h"
#include <algorithm>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

ConfigHandler::ConfigHandler(){
    m_settings.setDefaultFormat(QSettings::IniFormat);
}

QVector<CaptureButton::ButtonType> ConfigHandler::getButtons() {
    QVector<CaptureButton::ButtonType> buttons;
    if (m_settings.contains("buttons")) {
        // TODO: remove toList in v1.0
        QVector<int> buttonsInt =
                m_settings.value("buttons").value<QList<int> >().toVector();
        bool modified = normalizeButtons(buttonsInt);
        if (modified) {
            m_settings.setValue("buttons", QVariant::fromValue(buttonsInt.toList()));
        }
        buttons = fromIntToButton(buttonsInt);
    } else {
        buttons = CaptureButton::getIterableButtonTypes();
    }
    return buttons;
}

void ConfigHandler::setButtons(const QVector<CaptureButton::ButtonType> &buttons) {
    QVector<int> l = fromButtonToInt(buttons);
    normalizeButtons(l);
    // TODO: remove toList in v1.0
    m_settings.setValue("buttons", QVariant::fromValue(l.toList()));
}

QVector<QColor> ConfigHandler::getUserColors() {
    QVector<QColor> colors;
    if (m_settings.contains("userColors")) {
        colors = m_settings.value("userColors").value<QVector<QColor> >();
    } else {
        colors = {
            Qt::darkRed,
            Qt::red,
            Qt::yellow,
            Qt::green,
            Qt::darkGreen,
            Qt::cyan,
            Qt::blue,
            Qt::magenta,
            Qt::darkMagenta,
        };
    }
    return colors;
}

void ConfigHandler::setUserColors(const QVector<QColor> &l) {
    m_settings.setValue("userColors", QVariant::fromValue(l));
}

QString ConfigHandler::savePathValue() {
    return m_settings.value("savePath").toString();
}

void ConfigHandler::setSavePath(const QString &savePath) {
    m_settings.setValue("savePath", savePath);
}

QColor ConfigHandler::uiMainColorValue() {
    QColor res = QColor(116, 0, 150);
    if (m_settings.contains("uiColor")) {
        res = m_settings.value("uiColor").value<QColor>();
    }
    return res;
}

void ConfigHandler::setUIMainColor(const QColor &c) {
    m_settings.setValue("uiColor", c);
}

QColor ConfigHandler::uiContrastColorValue() {
    QColor res = QColor(86, 0, 120);
    if (m_settings.contains("contastUiColor")) {
        res = m_settings.value("contastUiColor").value<QColor>();
    }
    return res;
}

void ConfigHandler::setUIContrastColor(const QColor &c) {
    m_settings.setValue("contastUiColor", c);
}

QColor ConfigHandler::drawColorValue() {
    QColor res(Qt::red);
    if (m_settings.contains("drawColor")) {
        res = m_settings.value("drawColor").value<QColor>();
    }
    return res;
}

void ConfigHandler::setDrawColor(const QColor &c) {
    m_settings.setValue("drawColor", c);
}

bool ConfigHandler::showHelpValue() {
    bool res = true;
    if (m_settings.contains("showHelp")) {
        res = m_settings.value("showHelp").toBool();
    }
    return res;
}

void ConfigHandler::setShowHelp(const bool showHelp) {
    m_settings.setValue("showHelp", showHelp);
}

bool ConfigHandler::desktopNotificationValue() {
    bool res = true;
    if (m_settings.contains("showDesktopNotification")) {
        res = m_settings.value("showDesktopNotification").toBool();
    }
    return res;
}

void ConfigHandler::setDesktopNotification(const bool showDesktopNotification) {
    m_settings.setValue("showDesktopNotification", showDesktopNotification);
}

QString ConfigHandler::filenamePatternValue() {
    return m_settings.value("filenamePattern").toString();
}

void ConfigHandler::setFilenamePattern(const QString &pattern) {
    return m_settings.setValue("filenamePattern", pattern);
}

bool ConfigHandler::disabledTrayIconValue() {
    bool res = false;
    if (m_settings.contains("disabledTrayIcon")) {
        res = m_settings.value("disabledTrayIcon").toBool();
    }
    return res;
}

void ConfigHandler::setDisabledTrayIcon(const bool disabledTrayIcon) {
    m_settings.setValue("disabledTrayIcon", disabledTrayIcon);
}

int ConfigHandler::drawThicknessValue() {
    int res = 0;
    if (m_settings.contains("drawThickness")) {
        res = m_settings.value("drawThickness").toInt();
    }
    return res;
}

void ConfigHandler::setdrawThickness(const int thickness) {
    m_settings.setValue("drawThickness", thickness);
}

bool ConfigHandler::keepOpenAppLauncherValue() {
    return m_settings.value("keepOpenAppLauncher").toBool();
}

void ConfigHandler::setKeepOpenAppLauncher(const bool keepOpen) {
    m_settings.setValue("keepOpenAppLauncher", keepOpen);
}

bool ConfigHandler::startupLaunchValue() {
    bool res = false;
#if defined(Q_OS_LINUX)
    QString path = QDir::homePath() + "/.config/autostart/Flameshot.desktop";
    res = QFile(path).exists();
#elif defined(Q_OS_WIN)
    QSettings bootUpSettings(
                "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                QSettings::NativeFormat);
    res = bootUpSettings.value("Flameshot").toString() ==
            QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
#endif
    return res;
}

void ConfigHandler::setStartupLaunch(const bool start) {
#if defined(Q_OS_LINUX)
    QString path = QDir::homePath() + "/.config/autostart/Flameshot.desktop";
    QFile file(path);
    if (start) {
        if (file.open(QIODevice::WriteOnly)) {
            QByteArray data("[Desktop Entry]\nName=flameshot\nIcon=flameshot"
                            "\nExec=flameshot\nTerminal=false\nType=Application"
                            "\nX-GNOME-Autostart-enabled=true\n");
            file.write(data);
        }
    } else {
        file.remove();
    }
#elif defined(Q_OS_WIN)
    QSettings bootUpSettings(
                "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                QSettings::NativeFormat);
    if (start) {
        QString app_path =
                QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        bootUpSettings.setValue("Flameshot", app_path);
    } else {
        bootUpSettings.remove("Flameshot");
    }
#endif
}

int ConfigHandler::contrastOpacityValue() {
    int opacity = 190;
    if (m_settings.contains("contrastOpacity")) {
        opacity = m_settings.value("contrastOpacity").toInt();
        opacity = qBound(0, opacity, 255);
    }
    return opacity;
}

void ConfigHandler::setContrastOpacity(const int transparency) {
    m_settings.setValue("contrastOpacity", transparency);
}

void ConfigHandler::setDefaults() {
    m_settings.clear();
}

void ConfigHandler::setAllTheButtons() {
    QVector<int> buttons;
    auto listTypes = CaptureButton::getIterableButtonTypes();
    for (const CaptureButton::ButtonType t: listTypes) {
        buttons << static_cast<int>(t);
    }
    // TODO: remove toList in v1.0
    m_settings.setValue("buttons", QVariant::fromValue(buttons.toList()));
}

QString ConfigHandler::configFilePath() const {
    return m_settings.fileName();
}

bool ConfigHandler::normalizeButtons(QVector<int> &buttons) {
    auto listTypes = CaptureButton::getIterableButtonTypes();
    QVector<int> listTypesInt;
    for(auto i: listTypes)
        listTypesInt << static_cast<int>(i);

    bool hasChanged = false;
    for (int i = 0; i < buttons.size(); i++) {
        if (!listTypesInt.contains(buttons.at(i))) {
            buttons.remove(i);
            hasChanged = true;
        }
    }
    std::sort(buttons.begin(), buttons.end(), [](int a, int b){
        return CaptureButton::getPriorityByButton((CaptureButton::ButtonType)a) <
                CaptureButton::getPriorityByButton((CaptureButton::ButtonType)b);
    });
    return hasChanged;
}

QVector<CaptureButton::ButtonType> ConfigHandler::fromIntToButton(
        const QVector<int> &l)
{
    QVector<CaptureButton::ButtonType> buttons;
    for (auto const i: l)
        buttons << static_cast<CaptureButton::ButtonType>(i);
    return buttons;
}

QVector<int> ConfigHandler::fromButtonToInt(
        const QVector<CaptureButton::ButtonType> &l)
{
    QVector<int> buttons;
    for (auto const i: l)
        buttons << static_cast<int>(i);
    return buttons;
}
