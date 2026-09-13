// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "colorprofileprovider.h"

#include <QPixmap>
#include <QScreen>

#if defined(Q_OS_MACOS)
#include <CoreGraphics/CoreGraphics.h>

namespace {

// Match a Qt screen to a CoreGraphics display by comparing their top-left
// origin in the global (points) coordinate space. Falls back to the main
// display when no match is found.
CGDirectDisplayID displayForScreen(const QScreen* screen)
{
    if (screen != nullptr) {
        const QRect geom = screen->geometry();
        const uint32_t maxDisplays = 16;
        CGDirectDisplayID ids[maxDisplays];
        uint32_t count = 0;
        if (CGGetActiveDisplayList(maxDisplays, ids, &count) ==
            kCGErrorSuccess) {
            for (uint32_t i = 0; i < count; ++i) {
                const CGRect bounds = CGDisplayBounds(ids[i]);
                if (qRound(bounds.origin.x) == geom.x() &&
                    qRound(bounds.origin.y) == geom.y()) {
                    return ids[i];
                }
            }
        }
    }
    return CGMainDisplayID();
}

QColorSpace iccFromColorSpace(CGColorSpaceRef cg)
{
    if (cg == nullptr) {
        return {};
    }
    CFDataRef data = CGColorSpaceCopyICCData(cg);
    if (data == nullptr) {
        return {};
    }
    const QByteArray icc(reinterpret_cast<const char*>(CFDataGetBytePtr(data)),
                         static_cast<int>(CFDataGetLength(data)));
    CFRelease(data);
    return QColorSpace::fromIccProfile(icc);
}

} // namespace

QColorSpace ColorProfileProvider::forScreen(const QScreen* screen)
{
    const CGDirectDisplayID display = displayForScreen(screen);
    CGColorSpaceRef cg = CGDisplayCopyColorSpace(display);
    const QColorSpace platformProfile = iccFromColorSpace(cg);
    const bool wideGamut = (cg != nullptr) && CGColorSpaceIsWideGamutRGB(cg);
    if (cg != nullptr) {
        CGColorSpaceRelease(cg);
    }
    return resolve(platformProfile, wideGamut);
}

#else  // Non-macOS: not implemented yet, leave captures untagged as before.

QColorSpace ColorProfileProvider::forScreen(const QScreen* /*screen*/)
{
    return {};
}

#endif

QColorSpace ColorProfileProvider::resolve(const QColorSpace& platformProfile,
                                          bool wideGamut)
{
    if (platformProfile.isValid()) {
        return platformProfile;
    }
    if (wideGamut) {
        return QColorSpace(QColorSpace::DisplayP3);
    }
    return {};
}

QImage ColorProfileProvider::tagged(const QPixmap& pixmap,
                                    const QColorSpace& colorSpace)
{
    QImage image = pixmap.toImage();
    if (colorSpace.isValid()) {
        image.setColorSpace(colorSpace);
    }
    return image;
}
