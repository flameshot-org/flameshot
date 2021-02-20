// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QObject>

struct TerminalApp
{
    QString name;
    QString arg;
};

class TerminalLauncher : public QObject
{
    Q_OBJECT
public:
    explicit TerminalLauncher(QObject* parent = nullptr);

    static bool launchDetached(const QString& command);

private:
    static TerminalApp getPreferedTerminal();
};
