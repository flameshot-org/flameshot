// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QColor>

namespace ColorUtils { // namespace

bool colorIsDark(const QColor& c);

QColor contrastColor(const QColor& c);

} // namespace
