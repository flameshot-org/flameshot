// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Manuel Martins & Contributors

#include "pinhistoryentry.h"

#include <QJsonObject>

QJsonObject PinHistoryEntry::toJson() const
{
    QJsonObject geom;
    geom.insert(QStringLiteral("x"), geometry.x());
    geom.insert(QStringLiteral("y"), geometry.y());
    geom.insert(QStringLiteral("w"), geometry.width());
    geom.insert(QStringLiteral("h"), geometry.height());

    QJsonObject obj;
    obj.insert(QStringLiteral("id"), id);
    obj.insert(QStringLiteral("pixmap"), pixmapFile);
    obj.insert(QStringLiteral("thumb"), thumbFile);
    obj.insert(QStringLiteral("createdAt"),
               createdAt.toUTC().toString(Qt::ISODate));
    obj.insert(QStringLiteral("dismissedAt"),
               dismissedAt.toUTC().toString(Qt::ISODate));
    obj.insert(QStringLiteral("geometry"), geom);
    obj.insert(QStringLiteral("screen"), screenName);
    obj.insert(QStringLiteral("zoom"), zoom);
    obj.insert(QStringLiteral("rotation"), rotation);
    obj.insert(QStringLiteral("opacity"), opacity);
    obj.insert(QStringLiteral("width"), width);
    obj.insert(QStringLiteral("height"), height);
    return obj;
}

PinHistoryEntry PinHistoryEntry::fromJson(const QJsonObject& obj)
{
    PinHistoryEntry e;
    e.id = obj.value(QStringLiteral("id")).toString();
    e.pixmapFile = obj.value(QStringLiteral("pixmap")).toString();
    e.thumbFile = obj.value(QStringLiteral("thumb")).toString();
    e.createdAt = QDateTime::fromString(
      obj.value(QStringLiteral("createdAt")).toString(), Qt::ISODate);
    e.dismissedAt = QDateTime::fromString(
      obj.value(QStringLiteral("dismissedAt")).toString(), Qt::ISODate);

    const QJsonObject geom = obj.value(QStringLiteral("geometry")).toObject();
    e.geometry = QRect(geom.value(QStringLiteral("x")).toInt(),
                       geom.value(QStringLiteral("y")).toInt(),
                       geom.value(QStringLiteral("w")).toInt(),
                       geom.value(QStringLiteral("h")).toInt());

    e.screenName = obj.value(QStringLiteral("screen")).toString();
    e.zoom = obj.value(QStringLiteral("zoom")).toDouble(1.0);
    e.rotation = obj.value(QStringLiteral("rotation")).toInt(0);
    e.opacity = obj.value(QStringLiteral("opacity")).toDouble(1.0);
    e.width = obj.value(QStringLiteral("width")).toInt();
    e.height = obj.value(QStringLiteral("height")).toInt();
    return e;
}
