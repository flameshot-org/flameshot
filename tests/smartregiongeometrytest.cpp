// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "widgets/capture/smartregiongeometry.h"

#include <QTest>

using namespace SmartRegionGeometry;

class SmartRegionGeometryTest : public QObject
{
    Q_OBJECT

private slots:
    void spanningWindowUsesOnlyThePointerScreen();
    void hitTestingDoesNotUseTheBoundingUnion();
    void fractionalScaleAndNegativeOrigin();
    void logicalGapsAndInvalidMappingsAreRejected();
    void roundedPointStaysOnItsScreen();
};

void SmartRegionGeometryTest::spanningWindowUsesOnlyThePointerScreen()
{
    // Windows preserves monitor origins in physical pixels. Qt halves the
    // first screen's size at 200%, leaving a gap before the 100% screen.
    const QList<ScreenMapping> mappings{
        { QRect(0, 0, 3840, 2160), QRect(0, 0, 1920, 1080) },
        { QRect(3840, 0, 1920, 1080), QRect(3840, 0, 1920, 1080) },
    };
    const QRect window(3500, 100, 800, 800);
    const ScreenMapping* primary = screenAt(QPoint(1800, 100), mappings);
    const ScreenMapping* secondary = screenAt(QPoint(3900, 200), mappings);
    QVERIFY(primary);
    QVERIFY(secondary);

    QCOMPARE(toLogicalRect(window, *primary), QRect(1750, 50, 170, 400));
    QCOMPARE(toLogicalRect(window, *secondary), QRect(3840, 100, 460, 800));

    // The previous union took y=50 from the 200% screen, adding 50 pixels of
    // unrelated content above the window on the 100% capture screen.
    const QRect previousUnion =
      toLogicalRect(window, *primary).united(toLogicalRect(window, *secondary));
    QCOMPARE(previousUnion.intersected(secondary->logicalGeometry).top(), 50);
    QCOMPARE(toLogicalRect(window, *secondary).top(), 100);
}

void SmartRegionGeometryTest::hitTestingDoesNotUseTheBoundingUnion()
{
    const QList<ScreenMapping> mappings{
        { QRect(0, 0, 3840, 2160), QRect(0, 0, 1920, 1080) },
        { QRect(3840, 0, 1920, 1080), QRect(3840, 0, 1920, 1080) },
    };
    const QRect window(3500, 100, 800, 800);
    const QPoint pointer(4000, 75);
    const ScreenMapping* screen = screenAt(pointer, mappings);
    QVERIFY(screen);

    const QRect previousUnion =
      toLogicalRect(window, mappings.at(0))
        .united(toLogicalRect(window, mappings.at(1)));
    QVERIFY(previousUnion.contains(pointer));
    QVERIFY(!window.contains(toNativePoint(pointer, *screen)));
    QVERIFY(!toLogicalRect(window, *screen).contains(pointer));
}

void SmartRegionGeometryTest::fractionalScaleAndNegativeOrigin()
{
    const ScreenMapping mapping{
        QRect(-2560, -1440, 2560, 1440),
        QRect(-2560, -1440, 2048, 1152),
    };
    QCOMPARE(toLogicalRect(QRect(-2500, -1300, 500, 500), mapping),
             QRect(-2512, -1328, 400, 400));
    QCOMPARE(toNativePoint(QPoint(-2400, -1200), mapping),
             QPoint(-2360, -1140));
    QVERIFY(toLogicalRect(QRect(10, 10, 100, 100), mapping).isEmpty());
}

void SmartRegionGeometryTest::logicalGapsAndInvalidMappingsAreRejected()
{
    const QList<ScreenMapping> mappings{
        { QRect(0, 0, 3840, 2160), QRect(0, 0, 1920, 1080) },
        { QRect(3840, 0, 1920, 1080), QRect(3840, 0, 1920, 1080) },
        { QRect(), QRect(2000, 0, 100, 100) },
    };
    QVERIFY(!screenAt(QPoint(2500, 100), mappings));
    QVERIFY(!screenAt(QPoint(2050, 50), mappings));
    QVERIFY(toLogicalRect(QRect(10, 10, 20, 20), {}).isEmpty());
}

void SmartRegionGeometryTest::roundedPointStaysOnItsScreen()
{
    // A custom Qt scale factor below 1 can round the final logical pixel past
    // the native screen edge. It must not hit the adjacent monitor instead.
    const ScreenMapping mapping{ QRect(100, 200, 4, 4), QRect(100, 200, 8, 8) };
    QCOMPARE(toNativePoint(QPoint(107, 207), mapping), QPoint(103, 203));
}

QTEST_GUILESS_MAIN(SmartRegionGeometryTest)

#include "smartregiongeometrytest.moc"
