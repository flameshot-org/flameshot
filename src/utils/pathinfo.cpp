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

#include "pathinfo.h"
#include <QApplication>
#include <QFileInfo>
#include <QDir>

const QString PathInfo::whiteIconPath() {
    return ":/img/buttonIconsWhite/";
}

const QString PathInfo::blackIconPath() {
    return ":/img/buttonIconsBlack/";
}

const QString PathInfo::whiteConfigIconPath() {
    return ":/img/configWhite/";
}

const QString PathInfo::blackConfigIconPath() {
    return ":/img/configBlack/";
}


QStringList PathInfo::translationsPaths() {
    QString binaryPath = QFileInfo(qApp->applicationDirPath())
            .absoluteFilePath();
    QString trPath = QDir::toNativeSeparators(binaryPath + "/translations") ;
#if defined(Q_OS_LINUX)
    return QStringList()
            << QString(APP_PREFIX) + "/share/flameshot/translations"
            << trPath
            << "/usr/share/flameshot/translations"
            << "/usr/local/share/flameshot/translations";
#elif defined(Q_OS_WIN)
    return QStringList()
            << trPath;
#endif
}
