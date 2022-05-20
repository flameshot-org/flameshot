// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Jeremy Borgman

#ifndef FLAMESHOT_CACHEUTILS_H
#define FLAMESHOT_CACHEUTILS_H

class QString;
class QRect;

QString getCachePath();
QRect getLastRegion();
void setLastRegion(QRect const& newRegion);

#endif // FLAMESHOT_CACHEUTILS_H