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


#include "openwithprogram.h"

#if defined(Q_OS_WIN)
#include "src/utils/filenamehandler.h"
#include <QDir>
#include <QMessageBox>
#include <windows.h>
#include <Shlobj.h>

#pragma comment(lib, "Shell32.lib")
#else
#include "src/capture/workers/launcher/applauncherwidget.h"
#endif

void showOpenWithMenu(const QPixmap &capture) {
#if defined(Q_OS_WIN)
    QString tempFile =
            FileNameHandler().generateAbsolutePath(QDir::tempPath()) + ".png";
    bool ok = capture.save(tempFile);
    if (!ok) {
        QMessageBox::about(nullptr, QObject::tr("Error"),
                           QObject::tr("Unable to write in") + QDir::tempPath());
        return;
    }

    OPENASINFO info;
    auto wStringFile = tempFile.replace("/", "\\").toStdWString();
    info.pcszFile = wStringFile.c_str();
    info.pcszClass = nullptr;
    info.oaifInFlags = OAIF_ALLOW_REGISTRATION | OAIF_EXEC;
    SHOpenWithDialog(nullptr, &info);
#else
    auto w = new AppLauncherWidget(capture);
    w->show();
#endif
}
