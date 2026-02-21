// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "src/utils/screengrabber_geometry.h"
#include <QTest>

class TestScreenGrabberGeometry : public QObject
{
    Q_OBJECT

private slots:

    // ---- Linux crop rect calculation ----

    void linuxCrop_singleMonitorAtOrigin()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
        };

        QRect crop = ScreenGeometry::calculateCropRectLinux(screens, 0, 1920, 1080);
        QCOMPARE(crop, QRect(0, 0, 1920, 1080));
    }

    void linuxCrop_twoSideBySideSameDpr_cropSecond()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 1920, 1080), 1.0 },
        };

        // Screenshot is full desktop: 3840x1080
        QRect crop = ScreenGeometry::calculateCropRectLinux(screens, 1, 3840, 1080);
        QCOMPARE(crop, QRect(1920, 0, 1920, 1080));
    }

    void linuxCrop_twoSideBySideSameDpr_cropFirst()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 1920, 1080), 1.0 },
        };

        QRect crop = ScreenGeometry::calculateCropRectLinux(screens, 0, 3840, 1080);
        QCOMPARE(crop, QRect(0, 0, 1920, 1080));
    }

    void linuxCrop_twoMonitorsDifferentDpr()
    {
        // 1080p at 1x and 4K at 2x (logical 1920x1080 each)
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 1920, 1080), 2.0 },
        };

        // Portal may return at some scale. Assume it returns 3840x1080
        // (matching the logical total)
        QRect crop = ScreenGeometry::calculateCropRectLinux(screens, 1, 3840, 1080);
        QCOMPARE(crop, QRect(1920, 0, 1920, 1080));
    }

    void linuxCrop_negativeCoordinates()
    {
        // Left monitor is at negative X
        QList<ScreenMetadata> screens = {
            { QRect(-1920, 0, 1920, 1080), 1.0 },
            { QRect(0, 0, 1920, 1080), 1.0 },
        };

        // Total logical: from -1920 to 1920 = 3840 wide
        QRect crop = ScreenGeometry::calculateCropRectLinux(screens, 0, 3840, 1080);
        // Left monitor: x offset = (-1920 - (-1920)) * 1.0 = 0
        QCOMPARE(crop, QRect(0, 0, 1920, 1080));

        QRect crop1 = ScreenGeometry::calculateCropRectLinux(screens, 1, 3840, 1080);
        // Right monitor: x offset = (0 - (-1920)) * 1.0 = 1920
        QCOMPARE(crop1, QRect(1920, 0, 1920, 1080));
    }

    void linuxCrop_verticallyStacked()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(0, 1080, 1920, 1080), 1.0 },
        };

        QRect crop = ScreenGeometry::calculateCropRectLinux(screens, 1, 1920, 2160);
        QCOMPARE(crop, QRect(0, 1080, 1920, 1080));
    }

    void linuxCrop_threeMonitorsMixedArrangement()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 2560, 1440), 1.0 },
            { QRect(4480, 0, 1920, 1080), 1.0 },
        };

        // Total: 6400x1440
        QRect crop = ScreenGeometry::calculateCropRectLinux(screens, 1, 6400, 1440);
        QCOMPARE(crop, QRect(1920, 0, 2560, 1440));
    }

    void linuxCrop_portalReturnsScaledImage()
    {
        // Single 4K monitor at 2x scaling: logical 1920x1080
        // Portal returns at 2x: 3840x2160 pixels
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 2.0 },
        };

        QRect crop = ScreenGeometry::calculateCropRectLinux(screens, 0, 3840, 2160);
        QCOMPARE(crop, QRect(0, 0, 3840, 2160));
    }

    void linuxCrop_invalidIndex()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
        };

        QRect crop = ScreenGeometry::calculateCropRectLinux(screens, 5, 1920, 1080);
        QVERIFY(crop.isEmpty());

        QRect cropNeg = ScreenGeometry::calculateCropRectLinux(screens, -1, 1920, 1080);
        QVERIFY(cropNeg.isEmpty());
    }

    // ---- Windows crop rect calculation ----

    void windowsCrop_singleMonitorHighDpi()
    {
        // 150% scaling on a 1920x1080 logical display
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.5 },
        };

        QRect crop = ScreenGeometry::calculateCropRectWindows(screens, 0);
        QCOMPARE(crop, QRect(0, 0, 2880, 1620));
    }

    void windowsCrop_twoMonitorsSameDpr()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 1920, 1080), 1.0 },
        };

        QRect crop0 = ScreenGeometry::calculateCropRectWindows(screens, 0);
        QCOMPARE(crop0, QRect(0, 0, 1920, 1080));

        QRect crop1 = ScreenGeometry::calculateCropRectWindows(screens, 1);
        QCOMPARE(crop1, QRect(1920, 0, 1920, 1080));
    }

    void windowsCrop_twoMonitorsMixedDpr_100_175()
    {
        // From PR #4498 test report: 1920x1080 at 100% + 3840x2160 at 175%
        // Logical: 1920x1080 + 2194x1234 (3840/1.75, 2160/1.75 rounded)
        // But Qt reports logical geometry for the 175% screen
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 2194, 1234), 1.75 },
        };

        QRect crop0 = ScreenGeometry::calculateCropRectWindows(screens, 0);
        // No screens to the left of screen 0
        QCOMPARE(crop0.x(), 0);
        QCOMPARE(crop0.y(), 0);
        QCOMPARE(crop0.width(), 1920);
        QCOMPARE(crop0.height(), 1080);

        QRect crop1 = ScreenGeometry::calculateCropRectWindows(screens, 1);
        // Screen 0 is to the left: physical width = 1920 * 1.0 = 1920
        QCOMPARE(crop1.x(), 1920);
        QCOMPARE(crop1.y(), 0);
        QCOMPARE(crop1.width(), qRound(2194 * 1.75));
        QCOMPARE(crop1.height(), qRound(1234 * 1.75));
    }

    void windowsCrop_monitorsVerticalOffset()
    {
        // Two monitors, second is below the first
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(0, 1080, 1920, 1080), 1.5 },
        };

        QRect crop1 = ScreenGeometry::calculateCropRectWindows(screens, 1);
        // Screen 0 is completely above: physical height = 1080 * 1.0 = 1080
        QCOMPARE(crop1.x(), 0);
        QCOMPARE(crop1.y(), 1080);
        QCOMPARE(crop1.width(), qRound(1920 * 1.5));
        QCOMPARE(crop1.height(), qRound(1080 * 1.5));
    }

    void windowsCrop_threeMonitorsMixedDpi()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 2560, 1440), 1.25 },
            { QRect(4480, 0, 1920, 1080), 1.5 },
        };

        QRect crop2 = ScreenGeometry::calculateCropRectWindows(screens, 2);
        // Screens 0 and 1 are to the left:
        // physical width 0 = 1920, physical width 1 = 2560*1.25 = 3200
        QCOMPARE(crop2.x(), 1920 + 3200);
        QCOMPARE(crop2.y(), 0);
        QCOMPARE(crop2.width(), qRound(1920 * 1.5));
        QCOMPARE(crop2.height(), qRound(1080 * 1.5));
    }

    void windowsCrop_portraitAndLandscape()
    {
        // Landscape 1920x1080, Portrait 1080x1920 to the right
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 1080, 1920), 1.0 },
        };

        QRect crop1 = ScreenGeometry::calculateCropRectWindows(screens, 1);
        QCOMPARE(crop1, QRect(1920, 0, 1080, 1920));
    }

    void windowsCrop_invalidIndex()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
        };

        QVERIFY(ScreenGeometry::calculateCropRectWindows(screens, 3).isEmpty());
        QVERIFY(ScreenGeometry::calculateCropRectWindows(screens, -1).isEmpty());
    }

    // ---- Desktop geometry ----

    void desktopGeometry_singleScreen()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
        };

        QRect geom = ScreenGeometry::calculateDesktopGeometry(screens, true);
        QCOMPARE(geom, QRect(0, 0, 1920, 1080));
    }

    void desktopGeometry_twoSideBySide()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 2560, 1440), 1.0 },
        };

        QRect geom = ScreenGeometry::calculateDesktopGeometry(screens, true);
        QCOMPARE(geom, QRect(0, 0, 4480, 1440));
    }

    void desktopGeometry_screensWithGap()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(2000, 0, 1920, 1080), 1.0 },
        };

        QRect geom = ScreenGeometry::calculateDesktopGeometry(screens, true);
        QCOMPARE(geom, QRect(0, 0, 3920, 1080));
    }

    void desktopGeometry_linuxDprAdjustment()
    {
        // On Linux, positions are divided by DPR
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 2.0 },
            { QRect(1920, 0, 1920, 1080), 2.0 },
        };

        QRect geomLinux = ScreenGeometry::calculateDesktopGeometry(screens, false);
        // Position 1920 / 2.0 = 960; width remains 1920
        // Screen 0: moveTo(0/2, 0/2) = (0,0) size 1920x1080
        // Screen 1: moveTo(1920/2, 0/2) = (960,0) size 1920x1080
        // United: (0,0) to (960+1920, 1080) = 2880x1080
        QCOMPARE(geomLinux, QRect(0, 0, 2880, 1080));

        QRect geomWin = ScreenGeometry::calculateDesktopGeometry(screens, true);
        QCOMPARE(geomWin, QRect(0, 0, 3840, 1080));
    }

    // ---- Monitor sorting ----

    void sortIndices_alreadySorted()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 1920, 1080), 1.0 },
        };

        QList<int> sorted = ScreenGeometry::sortScreenIndicesByPosition(screens);
        QCOMPARE(sorted, (QList<int>{ 0, 1 }));
    }

    void sortIndices_reversed()
    {
        QList<ScreenMetadata> screens = {
            { QRect(1920, 0, 1920, 1080), 1.0 },
            { QRect(0, 0, 1920, 1080), 1.0 },
        };

        QList<int> sorted = ScreenGeometry::sortScreenIndicesByPosition(screens);
        QCOMPARE(sorted, (QList<int>{ 1, 0 }));
    }

    void sortIndices_threeMonitorsArbitrary()
    {
        QList<ScreenMetadata> screens = {
            { QRect(3840, 0, 1920, 1080), 1.0 },
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 1920, 1080), 1.0 },
        };

        QList<int> sorted = ScreenGeometry::sortScreenIndicesByPosition(screens);
        QCOMPARE(sorted, (QList<int>{ 1, 2, 0 }));
    }

    // ---- Linux rescaling check ----

    void linuxRescaling_sameScale()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
        };

        QVERIFY(!ScreenGeometry::linuxCropNeedsRescaling(screens, 0, 1920));
    }

    void linuxRescaling_differentScale()
    {
        // Two monitors: 1920+1920=3840 logical. Screenshot is 5760 wide
        // scaleX = 5760/3840 = 1.5, but monitor 0 DPR is 1.0 -> needs rescale
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 1920, 1080), 2.0 },
        };

        QVERIFY(ScreenGeometry::linuxCropNeedsRescaling(screens, 0, 5760));
    }
};

QTEST_GUILESS_MAIN(TestScreenGrabberGeometry)
#include "test_screengrabber_geometry.moc"
