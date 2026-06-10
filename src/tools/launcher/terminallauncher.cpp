// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "terminallauncher.h"

#include <QDir>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>

namespace {
static const TerminalApp terminalApps[] = {
    { "x-terminal-emulator", "-e" },
    { "xfce4-terminal", "-x" },
    { "konsole", "-e" },
    { "gnome-terminal", "--" },
    { "terminator", "-e" },
    { "terminology", "-e" },
    { "tilix", "-e" },
    { "xterm", "-e" },
    { "aterm", "-e" },
    { "Eterm", "-e" },
    { "rxvt", "-e" },
    { "urxvt", "-e" },
};
}

TerminalLauncher::TerminalLauncher(QObject* parent)
  : QObject(parent)
{}

TerminalApp TerminalLauncher::getPreferedTerminal()
{
    TerminalApp res;
    for (const TerminalApp& app : terminalApps) {
        QString path = QStandardPaths::findExecutable(app.name);
        if (!path.isEmpty()) {
            res = app;
            break;
        }
    }
    return res;
}

bool TerminalLauncher::launchDetached(const QStringList& args)
{
    TerminalApp app = getPreferedTerminal();
    if (app.name.isEmpty())
        return false;
    QStringList termArgs = { app.arg };
    termArgs.append(args);
    return QProcess::startDetached(app.name, termArgs);
}
