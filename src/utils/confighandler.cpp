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
    if (m_settings.contains(QStringLiteral("buttons"))) {
        // TODO: remove toList in v1.0
        QVector<int> buttonsInt =
                m_settings.value(QStringLiteral("buttons")).value<QList<int> >().toVector();
        bool modified = normalizeButtons(buttonsInt);
        if (modified) {
            m_settings.setValue(QStringLiteral("buttons"), QVariant::fromValue(buttonsInt.toList()));
        }
        buttons = fromIntToButton(buttonsInt);
    } else {
        // Default tools
        buttons << CaptureButton::TYPE_PENCIL
                << CaptureButton::TYPE_DRAWER
                << CaptureButton::TYPE_ARROW
                << CaptureButton::TYPE_SELECTION
                << CaptureButton::TYPE_RECTANGLE
                << CaptureButton::TYPE_CIRCLE
                << CaptureButton::TYPE_MARKER
                << CaptureButton::TYPE_BLUR
                << CaptureButton::TYPE_SELECTIONINDICATOR
                << CaptureButton::TYPE_MOVESELECTION
                << CaptureButton::TYPE_UNDO
                << CaptureButton::TYPE_REDO
                << CaptureButton::TYPE_COPY
                << CaptureButton::TYPE_SAVE
                << CaptureButton::TYPE_EXIT
                << CaptureButton::TYPE_IMAGEUPLOADER
                << CaptureButton::TYPE_OPEN_APP
                << CaptureButton::TYPE_PIN
                << CaptureButton::TYPE_TEXT;
    }

    using bt = CaptureButton::ButtonType;
    std::sort(buttons.begin(), buttons.end(), [](bt a, bt b){
        return CaptureButton::getPriorityByButton(a) <
                CaptureButton::getPriorityByButton(b);
    });
    return buttons;
}

void ConfigHandler::setButtons(const QVector<CaptureButton::ButtonType> &buttons) {
    QVector<int> l = fromButtonToInt(buttons);
    normalizeButtons(l);
    // TODO: remove toList in v1.0
    m_settings.setValue(QStringLiteral("buttons"), QVariant::fromValue(l.toList()));
}

QVector<QColor> ConfigHandler::getUserColors() {
    QVector<QColor> colors;
    const QVector<QColor> &defaultColors = {
        Qt::darkRed,
        Qt::red,
        Qt::yellow,
        Qt::green,
        Qt::darkGreen,
        Qt::cyan,
        Qt::blue,
        Qt::magenta,
        Qt::darkMagenta
    };

    if (m_settings.contains(QStringLiteral("userColors"))) {
        for (const QString &hex : m_settings.value(QStringLiteral("userColors")).toStringList()) {
            if (QColor::isValidColor(hex)) {
                colors.append(QColor(hex));
            }
        }

        if (colors.isEmpty()) {
            colors = defaultColors;
        }
    } else {
        colors = defaultColors;
    }

    return colors;
}

void ConfigHandler::setUserColors(const QVector<QColor> &l) {
    QStringList hexColors;

    for (const QColor &color : l) {
        hexColors.append(color.name());
    }

    m_settings.setValue(QStringLiteral("userColors"), QVariant::fromValue(hexColors));
}

QString ConfigHandler::savePathValue() {
    return m_settings.value(QStringLiteral("savePath")).toString();
}

void ConfigHandler::setSavePath(const QString &savePath) {
    m_settings.setValue(QStringLiteral("savePath"), savePath);
}

QColor ConfigHandler::uiMainColorValue() {
    QColor res = QColor(116, 0, 150);

    if (m_settings.contains(QStringLiteral("uiColor"))) {
        QString hex = m_settings.value(QStringLiteral("uiColor")).toString();

        if (QColor::isValidColor(hex)) {
            res = QColor(hex);
        }
    }
    return res;
}

void ConfigHandler::setUIMainColor(const QColor &c) {
    m_settings.setValue(QStringLiteral("uiColor"), c.name());
}

QColor ConfigHandler::uiContrastColorValue() {
    QColor res = QColor(86, 0, 120);

    if (m_settings.contains(QStringLiteral("contastUiColor"))) {
        QString hex = m_settings.value(QStringLiteral("contastUiColor")).toString();

        if (QColor::isValidColor(hex)) {
            res = QColor(hex);
        }
    }

    return res;
}

void ConfigHandler::setUIContrastColor(const QColor &c) {
    m_settings.setValue(QStringLiteral("contastUiColor"), c.name());
}

QColor ConfigHandler::drawColorValue() {
    QColor res(Qt::red);

    if (m_settings.contains(QStringLiteral("drawColor"))) {
        QString hex = m_settings.value(QStringLiteral("drawColor")).toString();

        if (QColor::isValidColor(hex)) {
            res = QColor(hex);
        }
    }

    return res;
}

void ConfigHandler::setDrawColor(const QColor &c) {
    m_settings.setValue(QStringLiteral("drawColor"), c.name());
}

bool ConfigHandler::showHelpValue() {
    bool res = true;
    if (m_settings.contains(QStringLiteral("showHelp"))) {
        res = m_settings.value(QStringLiteral("showHelp")).toBool();
    }
    return res;
}

void ConfigHandler::setShowHelp(const bool showHelp) {
    m_settings.setValue(QStringLiteral("showHelp"), showHelp);
}

bool ConfigHandler::desktopNotificationValue() {
    bool res = true;
    if (m_settings.contains(QStringLiteral("showDesktopNotification"))) {
        res = m_settings.value(QStringLiteral("showDesktopNotification")).toBool();
    }
    return res;
}

void ConfigHandler::setDesktopNotification(const bool showDesktopNotification) {
    m_settings.setValue(QStringLiteral("showDesktopNotification"), showDesktopNotification);
}

QString ConfigHandler::filenamePatternValue() {
    return m_settings.value(QStringLiteral("filenamePattern")).toString();
}

void ConfigHandler::setFilenamePattern(const QString &pattern) {
    return m_settings.setValue(QStringLiteral("filenamePattern"), pattern);
}

bool ConfigHandler::disabledTrayIconValue() {
    bool res = false;
    if (m_settings.contains(QStringLiteral("disabledTrayIcon"))) {
        res = m_settings.value(QStringLiteral("disabledTrayIcon")).toBool();
    }
    return res;
}

void ConfigHandler::setDisabledTrayIcon(const bool disabledTrayIcon) {
    m_settings.setValue(QStringLiteral("disabledTrayIcon"), disabledTrayIcon);
}

int ConfigHandler::drawThicknessValue() {
    int res = 0;
    if (m_settings.contains(QStringLiteral("drawThickness"))) {
        res = m_settings.value(QStringLiteral("drawThickness")).toInt();
    }
    return res;
}

void ConfigHandler::setdrawThickness(const int thickness) {
    m_settings.setValue(QStringLiteral("drawThickness"), thickness);
}

bool ConfigHandler::keepOpenAppLauncherValue() {
    return m_settings.value(QStringLiteral("keepOpenAppLauncher")).toBool();
}

void ConfigHandler::setKeepOpenAppLauncher(const bool keepOpen) {
    m_settings.setValue(QStringLiteral("keepOpenAppLauncher"), keepOpen);
}

bool ConfigHandler::startupLaunchValue() {
    bool res = false;

    if (m_settings.contains(QStringLiteral("startupLaunch"))) {
        res = m_settings.value(QStringLiteral("startupLaunch")).toBool();
    }

    if (res != verifyLaunchFile()) {
        setStartupLaunch(res);
    }

    return res;
}

bool ConfigHandler::verifyLaunchFile() {
    bool res = false;

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
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
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    QString path = QDir::homePath() + "/.config/autostart/";
    QDir autostartDir(path);
    if (!autostartDir.exists()) {
        autostartDir.mkpath(".");
    }

    QFile file(path + "Flameshot.desktop");
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
    m_settings.setValue(QStringLiteral("startupLaunch"), start);
}

int ConfigHandler::contrastOpacityValue() {
    int opacity = 190;
    if (m_settings.contains(QStringLiteral("contrastOpacity"))) {
        opacity = m_settings.value(QStringLiteral("contrastOpacity")).toInt();
        opacity = qBound(0, opacity, 255);
    }
    return opacity;
}

void ConfigHandler::setContrastOpacity(const int transparency) {
    m_settings.setValue(QStringLiteral("contrastOpacity"), transparency);
}

bool ConfigHandler::closeAfterScreenshotValue() {
    return m_settings.value(QStringLiteral("closeAfterScreenshot")).toBool();
}

void ConfigHandler::setCloseAfterScreenshot(const bool close) {
    m_settings.setValue(QStringLiteral("closeAfterScreenshot"), close);
}

bool ConfigHandler::saveAfterCopyValue() {
    return m_settings.value(QStringLiteral("saveAfterCopy")).toBool();
}

void ConfigHandler::setSaveAfterCopy(const bool save) {
    m_settings.setValue(QStringLiteral("saveAfterCopy"), save);
}

QString ConfigHandler::saveAfterCopyPathValue() {
    return m_settings.value(QStringLiteral("saveAfterCopyPath")).toString();
}

void ConfigHandler::setSaveAfterCopyPath(const QString &path) {
    m_settings.setValue(QStringLiteral("saveAfterCopyPath"), path);
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
    m_settings.setValue(QStringLiteral("buttons"), QVariant::fromValue(buttons.toList()));
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
