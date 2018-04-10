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

#include "terminallauncher.h"
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QProcessEnvironment>

namespace {
    static const TerminalApp terminalApps[] = {
        { "x-terminal-emulator", "-e" },
        { "xfce4-terminal",         "-x" },
        { "konsole",             "-e" },
        { "gnome-terminal",         "--" },
        { "terminator",             "-e" },
        { "terminology",         "-e" },
        { "tilix",                 "-e" },
        { "xterm",                 "-e" },
        { "aterm",                 "-e" },
        { "Eterm",                 "-e" },
        { "rxvt",                 "-e" },
        { "urxvt",                 "-e" },
    };
}

TerminalLauncher::TerminalLauncher(QObject *parent) : QObject(parent) {
}

TerminalApp TerminalLauncher::getPreferedTerminal() {
    TerminalApp res;
    for (const TerminalApp &app : terminalApps) {
        QString path = QStandardPaths::findExecutable(app.name);
        if (!path.isEmpty()) {
            res = app;
            break;
        }
    }
    return res;
}

bool TerminalLauncher::launchDetached(const QString &command) {
    TerminalApp app = getPreferedTerminal();
    QString s = app.name + " " + app.arg + " " + command;
    return QProcess::startDetached(s);
}
