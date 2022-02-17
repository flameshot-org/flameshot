// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "globalvalues.h"
#include <QApplication>
#include <QFontMetrics>

int GlobalValues::buttonBaseSize()
{
    return QApplication::fontMetrics().lineSpacing() * 2.2;
}

QString GlobalValues::versionInfo()
{
    return QStringLiteral("Flameshot " APP_VERSION " (" FLAMESHOT_GIT_HASH ")"
                          "\nCompiled with Qt " QT_VERSION_STR);
}

QString GlobalValues::iconPath()
{
#if USE_MONOCHROME_ICON
    return QString(":img/app/flameshot.monochrome.svg");
#else
    return { ":img/app/flameshot.svg" };
#endif
}

QString GlobalValues::iconPathPNG()
{
#if USE_MONOCHROME_ICON
    return QString(":img/app/flameshot.monochrome.png");
#else
    return { ":img/app/flameshot.png" };
#endif
}