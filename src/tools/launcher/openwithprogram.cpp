// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "openwithprogram.h"

#if defined(Q_OS_WIN)
#include "src/utils/filenamehandler.h"
#include <QDir>
#include <QMessageBox>
#include <windows.h>
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif
#include <Shlobj.h>

#pragma comment(lib, "Shell32.lib")
#else
#include "src/tools/launcher/applauncherwidget.h"
#endif

void showOpenWithMenu(const QPixmap& capture)
{
#if defined(Q_OS_WIN)
    QString tempFile =
      FileNameHandler().properScreenshotPath(QDir::tempPath(), "png");
    bool ok = capture.save(tempFile);
    if (!ok) {
        QMessageBox::about(nullptr,
                           QObject::tr("Error"),
                           QObject::tr("Unable to write in") +
                             QDir::tempPath());
        return;
    }

    OPENASINFO info;
    auto wStringFile = tempFile.replace("/", "\\").toStdWString();
    info.pcszFile = wStringFile.c_str();
    info.pcszClass = nullptr;
    info.oaifInFlags = OAIF_ALLOW_REGISTRATION | OAIF_EXEC;
    SHOpenWithDialog(nullptr, &info);
#else
    auto* w = new AppLauncherWidget(capture);
    w->show();
#endif
}
