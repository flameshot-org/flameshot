// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "smartregiondetector.h"

// Each native backend defines these two functions. The build system adds
// exactly one backend source and FLAMESHOT_SMART_REGION_BACKEND, or none.
#if defined(FLAMESHOT_SMART_REGION_BACKEND)
bool nativeSmartRegionDetectorSupported();
std::unique_ptr<SmartRegionDetector> createNativeSmartRegionDetector();
#endif

namespace {
// Smaller regions are usually decorations or separators, not useful targets.
constexpr int MIN_REGION_SIDE = 4;
}

QList<QRect> buildRegionChain(const QList<QRect>& elementGeometries,
                              const QRect& windowGeometry,
                              const QPoint& globalPosition)
{
    const QRect window = windowGeometry.normalized();
    if (!window.isValid() || !window.contains(globalPosition)) {
        return {};
    }

    QList<QRect> chain;
    for (const QRect& element : elementGeometries) {
        const QRect region = element.normalized().intersected(window);
        if (region.width() < MIN_REGION_SIDE ||
            region.height() < MIN_REGION_SIDE ||
            !region.contains(globalPosition) || chain.contains(region)) {
            continue;
        }
        chain.append(region);
    }
    if (!chain.contains(window)) {
        chain.append(window);
    }
    return chain;
}

bool SmartRegionDetector::isSupported()
{
#if defined(FLAMESHOT_SMART_REGION_BACKEND)
    return nativeSmartRegionDetectorSupported();
#else
    return false;
#endif
}

std::unique_ptr<SmartRegionDetector> SmartRegionDetector::create()
{
#if defined(FLAMESHOT_SMART_REGION_BACKEND)
    if (nativeSmartRegionDetectorSupported()) {
        return createNativeSmartRegionDetector();
    }
#endif
    return nullptr;
}
