// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "colorutils.h"

inline qreal getColorLuma(const QColor& c)
{
    return 0.30 * c.redF() + 0.59 * c.greenF() + 0.11 * c.blueF();
}

bool ColorUtils::colorIsDark(const QColor& c)
{
    // when luma <= 0.5, we considor it as a dark color
    return getColorLuma(c) <= 0.5;
}

QColor ColorUtils::contrastColor(const QColor& c)
{
    int change = colorIsDark(c) ? 30 : -45;

    return { qBound(0, c.red() + change, 255),
             qBound(0, c.green() + change, 255),
             qBound(0, c.blue() + change, 255) };
}
