// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QList>
#include <QPoint>
#include <QRect>

#include <cmath>

namespace SmartRegionGeometry {
struct ScreenMapping
{
    QRect nativeGeometry;
    QRect logicalGeometry;
};

inline const ScreenMapping* screenAt(const QPoint& logicalPosition,
                                     const QList<ScreenMapping>& mappings)
{
    for (const ScreenMapping& mapping : mappings) {
        if (mapping.nativeGeometry.isValid() &&
            mapping.logicalGeometry.isValid() &&
            mapping.logicalGeometry.contains(logicalPosition)) {
            return &mapping;
        }
    }
    return nullptr;
}

inline QRect toLogicalRect(const QRect& nativeRect,
                           const ScreenMapping& mapping)
{
    if (!mapping.nativeGeometry.isValid() ||
        !mapping.logicalGeometry.isValid()) {
        return {};
    }
    const QRect nativePart = nativeRect.intersected(mapping.nativeGeometry);
    if (!nativePart.isValid()) {
        return {};
    }

    // Windows keeps screen origins in native pixels while Qt scales their
    // sizes. Map only the pointer's screen: uniting pieces from different DPI
    // domains would introduce nonexistent pixels into the selected region.
    const double xScale = static_cast<double>(mapping.logicalGeometry.width()) /
                          mapping.nativeGeometry.width();
    const double yScale =
      static_cast<double>(mapping.logicalGeometry.height()) /
      mapping.nativeGeometry.height();
    const int left = static_cast<int>(
      std::floor(mapping.logicalGeometry.x() +
                 (nativePart.x() - mapping.nativeGeometry.x()) * xScale));
    const int top = static_cast<int>(
      std::floor(mapping.logicalGeometry.y() +
                 (nativePart.y() - mapping.nativeGeometry.y()) * yScale));
    const int right = static_cast<int>(std::ceil(
      mapping.logicalGeometry.x() +
      (nativePart.x() + nativePart.width() - mapping.nativeGeometry.x()) *
        xScale));
    const int bottom = static_cast<int>(std::ceil(
      mapping.logicalGeometry.y() +
      (nativePart.y() + nativePart.height() - mapping.nativeGeometry.y()) *
        yScale));
    return QRect(left, top, right - left, bottom - top)
      .intersected(mapping.logicalGeometry);
}

inline QPoint toNativePoint(const QPoint& logicalPoint,
                            const ScreenMapping& mapping)
{
    if (!mapping.nativeGeometry.isValid() ||
        !mapping.logicalGeometry.isValid()) {
        return {};
    }

    const double xScale = static_cast<double>(mapping.nativeGeometry.width()) /
                          mapping.logicalGeometry.width();
    const double yScale = static_cast<double>(mapping.nativeGeometry.height()) /
                          mapping.logicalGeometry.height();
    const int x = mapping.nativeGeometry.x() +
                  static_cast<int>(std::lround(
                    (logicalPoint.x() - mapping.logicalGeometry.x()) * xScale));
    const int y = mapping.nativeGeometry.y() +
                  static_cast<int>(std::lround(
                    (logicalPoint.y() - mapping.logicalGeometry.y()) * yScale));
    return {
        qBound(
          mapping.nativeGeometry.left(), x, mapping.nativeGeometry.right()),
        qBound(mapping.nativeGeometry.top(), y, mapping.nativeGeometry.bottom())
    };
}
}
