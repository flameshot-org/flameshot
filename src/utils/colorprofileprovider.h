// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QColorSpace>
#include <QImage>

class QPixmap;
class QScreen;

/**
 * Resolves and applies the display's ICC color profile so that saved
 * screenshots are tagged the same way the native macOS screenshot tool tags
 * them (see issue #4341). Without the tag, color-managed apps assume sRGB and
 * wide-gamut captures look desaturated.
 *
 * This is currently a macOS-only fix: on other platforms forScreen() returns an
 * invalid QColorSpace and tagging is a no-op, leaving behavior unchanged.
 */
class ColorProfileProvider
{
public:
    /**
     * The color space of the given screen's display, or an invalid QColorSpace
     * when it cannot be determined (or the platform is not supported).
     */
    static QColorSpace forScreen(const QScreen* screen);

    /**
     * Pure fallback chain, exposed for testing: a valid platform profile is
     * used as-is; otherwise Display P3 for wide-gamut displays; otherwise an
     * invalid color space (i.e. leave untagged, sRGB being the universal
     * default).
     */
    static QColorSpace resolve(const QColorSpace& platformProfile,
                               bool wideGamut);

    /**
     * Returns the pixmap as a QImage, tagged with @p colorSpace when it is
     * valid (otherwise untagged). Tagging only labels the pixels; it does not
     * convert them, so it is cheap and lossless.
     */
    static QImage tagged(const QPixmap& pixmap, const QColorSpace& colorSpace);
};
