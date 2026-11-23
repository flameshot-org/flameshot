// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QString>

// Opt-in diagnostics of input handling, for problems that only occur on
// particular hardware. Set FLAMESHOT_INPUT_LOG to a file path to enable it;
// each line starts with the milliseconds since the process started.
namespace InputTrace {
bool enabled();
void write(const QString& line);
}
