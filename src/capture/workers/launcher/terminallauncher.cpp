#include "terminallauncher.h"
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QProcessEnvironment>

namespace {
	static const TerminalApp terminalApps[] = {
		{ "xterm",				 "-e" },
		{ "x-terminal-emulator", "-e" },
		{ "aterm",				 "-e" },
		{ "Eterm",				 "-e" },
		{ "rxvt",				 "-e" },
		{ "urxvt",				 "-e" },
		{ "xfce4-terminal",		 "-x" },
		{ "konsole",			 "-e" },
		{ "gnome-terminal",		 "--" },
		{ "terminator",			 "-e" },
		{ "terminology",		 "-e" },
		{ "tilix",				 "-e" },
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
