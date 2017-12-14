#ifndef TERMINALLAUNCHER_H
#define TERMINALLAUNCHER_H

#include <QObject>

struct TerminalApp {
	QString name;
	QString arg;
};

class TerminalLauncher : public QObject
{
	Q_OBJECT
public:
	explicit TerminalLauncher(QObject *parent = nullptr);

	static bool launchDetached(const QString &command);
private:
	static TerminalApp getPreferedTerminal();
};

#endif // TERMINALLAUNCHER_H
