// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Manuel Martins & Contributors

#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QRect>
#include <QString>

struct PinHistoryEntry
{
    QString id;
    QString pixmapFile;
    QString thumbFile;
    QDateTime createdAt;
    QDateTime dismissedAt;
    QRect geometry;
    QString screenName;
    qreal zoom{ 1.0 };
    int rotation{ 0 };
    qreal opacity{ 1.0 };
    int width{ 0 };
    int height{ 0 };

    bool isValid() const { return !id.isEmpty() && !pixmapFile.isEmpty(); }

    QJsonObject toJson() const;
    static PinHistoryEntry fromJson(const QJsonObject& obj);
};
