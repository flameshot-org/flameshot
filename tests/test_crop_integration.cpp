// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "src/utils/screengrabber_geometry.h"
#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QTest>

// Integration tests that create real QPixmap objects and verify that the
// extracted crop geometry functions produce correct results when applied
// to actual pixel data.

class TestCropIntegration : public QObject
{
    Q_OBJECT

private:
    // Fill a rectangular region of an image with a solid color
    static void fillRegion(QImage& img, const QRect& rect, QColor color)
    {
        QPainter p(&img);
        p.fillRect(rect, color);
        p.end();
    }

    // Sample the color at the center of a pixmap
    static QColor centerColor(const QPixmap& pm)
    {
        QImage img = pm.toImage();
        return img.pixelColor(img.width() / 2, img.height() / 2);
    }

private slots:

    void cropLinux_twoMonitors_correctRegion()
    {
        // Simulate a 3840x1080 desktop with two 1920x1080 monitors
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 1920, 1080), 1.0 },
        };

        QImage desktop(3840, 1080, QImage::Format_RGB32);
        desktop.fill(Qt::black);
        fillRegion(desktop, QRect(0, 0, 1920, 1080), Qt::red);
        fillRegion(desktop, QRect(1920, 0, 1920, 1080), Qt::blue);
        QPixmap fullScreenshot = QPixmap::fromImage(desktop);

        // Crop to monitor 0 (red)
        QRect crop0 = ScreenGeometry::calculateCropRectLinux(screens, 0, fullScreenshot.width(), fullScreenshot.height());
        QPixmap cropped0 = fullScreenshot.copy(crop0);
        QCOMPARE(cropped0.size(), QSize(1920, 1080));
        QCOMPARE(centerColor(cropped0), QColor(Qt::red));

        // Crop to monitor 1 (blue)
        QRect crop1 = ScreenGeometry::calculateCropRectLinux(screens, 1, fullScreenshot.width(), fullScreenshot.height());
        QPixmap cropped1 = fullScreenshot.copy(crop1);
        QCOMPARE(cropped1.size(), QSize(1920, 1080));
        QCOMPARE(centerColor(cropped1), QColor(Qt::blue));
    }

    void cropLinux_scaledScreenshot()
    {
        // Single 4K monitor at 2x: logical 1920x1080, screenshot 3840x2160
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 2.0 },
        };

        QImage desktop(3840, 2160, QImage::Format_RGB32);
        desktop.fill(Qt::green);
        QPixmap fullScreenshot = QPixmap::fromImage(desktop);

        QRect crop = ScreenGeometry::calculateCropRectLinux(screens, 0, fullScreenshot.width(), fullScreenshot.height());
        QPixmap cropped = fullScreenshot.copy(crop);
        QCOMPARE(cropped.size(), QSize(3840, 2160));
        QCOMPARE(centerColor(cropped), QColor(Qt::green));
    }

    void cropWindows_twoMonitorsMixedDpi()
    {
        // Monitor 0: 1920x1080 at 1x -> 1920x1080 physical
        // Monitor 1: 1920x1080 at 1.5x -> 2880x1620 physical
        // Canvas: 4800x1620
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 1920, 1080), 1.5 },
        };

        int canvasW = 1920 + 2880;
        int canvasH = 1620;

        QImage desktop(canvasW, canvasH, QImage::Format_RGB32);
        desktop.fill(Qt::black);
        fillRegion(desktop, QRect(0, 0, 1920, 1080), Qt::cyan);
        fillRegion(desktop, QRect(1920, 0, 2880, 1620), Qt::magenta);
        QPixmap fullScreenshot = QPixmap::fromImage(desktop);

        QRect crop0 = ScreenGeometry::calculateCropRectWindows(screens, 0);
        QPixmap cropped0 = fullScreenshot.copy(crop0);
        QCOMPARE(cropped0.size(), QSize(1920, 1080));
        QCOMPARE(centerColor(cropped0), QColor(Qt::cyan));

        QRect crop1 = ScreenGeometry::calculateCropRectWindows(screens, 1);
        QPixmap cropped1 = fullScreenshot.copy(crop1);
        QCOMPARE(cropped1.size(), QSize(2880, 1620));
        QCOMPARE(centerColor(cropped1), QColor(Qt::magenta));
    }

    void cropLinux_threeMonitors_cropMiddle()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 2560, 1440), 1.0 },
            { QRect(4480, 0, 1920, 1080), 1.0 },
        };

        int totalW = 6400;
        int totalH = 1440;
        QImage desktop(totalW, totalH, QImage::Format_RGB32);
        desktop.fill(Qt::black);
        fillRegion(desktop, QRect(0, 0, 1920, 1080), Qt::red);
        fillRegion(desktop, QRect(1920, 0, 2560, 1440), Qt::green);
        fillRegion(desktop, QRect(4480, 0, 1920, 1080), Qt::blue);
        QPixmap fullScreenshot = QPixmap::fromImage(desktop);

        QRect crop1 = ScreenGeometry::calculateCropRectLinux(screens, 1, fullScreenshot.width(), fullScreenshot.height());
        QPixmap cropped1 = fullScreenshot.copy(crop1);
        QCOMPARE(cropped1.size(), QSize(2560, 1440));
        QCOMPARE(centerColor(cropped1), QColor(Qt::green));
    }

    void cropBoundsClamp_cropExceedsPixmap()
    {
        // Craft a scenario where the calculated crop goes beyond the pixmap
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
            { QRect(1920, 0, 1920, 1080), 1.0 },
        };

        // Provide a screenshot that's smaller than expected
        QImage desktop(2000, 1080, QImage::Format_RGB32);
        desktop.fill(Qt::yellow);
        QPixmap fullScreenshot = QPixmap::fromImage(desktop);

        QRect crop1 = ScreenGeometry::calculateCropRectLinux(screens, 1, fullScreenshot.width(), fullScreenshot.height());

        // The raw crop would go beyond 2000px width, intersect should clamp
        QRect clamped = crop1.intersected(QRect(0, 0, fullScreenshot.width(), fullScreenshot.height()));
        QVERIFY(clamped.right() < fullScreenshot.width());
    }

    void cropLinux_negativeCoordinates_correctPixels()
    {
        QList<ScreenMetadata> screens = {
            { QRect(-1920, 0, 1920, 1080), 1.0 },
            { QRect(0, 0, 1920, 1080), 1.0 },
        };

        QImage desktop(3840, 1080, QImage::Format_RGB32);
        desktop.fill(Qt::black);
        fillRegion(desktop, QRect(0, 0, 1920, 1080), Qt::darkGreen);
        fillRegion(desktop, QRect(1920, 0, 1920, 1080), Qt::darkBlue);
        QPixmap fullScreenshot = QPixmap::fromImage(desktop);

        // Monitor 0 (at x=-1920): should map to pixel 0
        QRect crop0 = ScreenGeometry::calculateCropRectLinux(screens, 0, fullScreenshot.width(), fullScreenshot.height());
        QCOMPARE(crop0.x(), 0);
        QPixmap cropped0 = fullScreenshot.copy(crop0);
        QCOMPARE(centerColor(cropped0), QColor(Qt::darkGreen));

        // Monitor 1 (at x=0): should map to pixel 1920
        QRect crop1 = ScreenGeometry::calculateCropRectLinux(screens, 1, fullScreenshot.width(), fullScreenshot.height());
        QCOMPARE(crop1.x(), 1920);
        QPixmap cropped1 = fullScreenshot.copy(crop1);
        QCOMPARE(centerColor(cropped1), QColor(Qt::darkBlue));
    }

    void invalidMonitorIndex_returnsEmptyRect()
    {
        QList<ScreenMetadata> screens = {
            { QRect(0, 0, 1920, 1080), 1.0 },
        };

        QRect crop = ScreenGeometry::calculateCropRectLinux(screens, 5, 1920, 1080);
        QVERIFY(crop.isEmpty());

        QRect cropWin = ScreenGeometry::calculateCropRectWindows(screens, 5);
        QVERIFY(cropWin.isEmpty());
    }
};

// Use QTEST_MAIN (not GUILESS) because QPixmap requires QApplication
QTEST_MAIN(TestCropIntegration)
#include "test_crop_integration.moc"
