// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Jeremy Borgman

#include "cacheutils.h"
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QRect>
#include <QStandardPaths>
#include <QString>

QString getCachePath()
{
    auto cachePath =
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (!QDir(cachePath).exists()) {
        QDir().mkpath(cachePath);
    }
    return cachePath;
}

void setLastRegion(QRect const& newRegion)
{
    auto cachePath = getCachePath() + "/region.txt";

    QFile file(cachePath);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out << newRegion;
        file.close();
    }
}

QRect getLastRegion()
{
    auto cachePath = getCachePath() + "/region.txt";
    QFile file(cachePath);

    QRect lastRegion;
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream input(&file);
        input >> lastRegion;
        file.close();
    } else {
        lastRegion = QRect(0, 0, 0, 0);
    }

    return lastRegion;
}