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
