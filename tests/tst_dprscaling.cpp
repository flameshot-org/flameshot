// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/dprscaling.h"

#include <QTest>

using DprScaling::Fit;

class TestDprScaling : public QObject
{
    Q_OBJECT

private slots:
    void fitCrop_data();
    void fitCrop();
    void fitCrop_neverEnlarges();
    void fitCrop_degenerateLogicalSize();

    void toDevicePixels_data();
    void toDevicePixels();
    void toDevicePixels_wholeRatiosScaleOriginAndExtentDirectly();
    void toDevicePixels_adjacentRectsShareAnEdge();

    void toPixmapPoint_data();
    void toPixmapPoint();

    void toDevicePixelsLength_data();
    void toDevicePixelsLength();
};

void TestDprScaling::fitCrop_data()
{
    QTest::addColumn<QSize>("cropped");
    QTest::addColumn<QSize>("logical");
    QTest::addColumn<qreal>("reportedDpr");
    QTest::addColumn<QSize>("expectedSize");
    QTest::addColumn<qreal>("expectedDpr");

    // Measured on a real mixed-scale setup. The portal composites the whole
    // desktop at one uniform scale (1.5), so each monitor's crop arrives at
    // that scale regardless of its own.

    // DP-1: a 3840x2160 panel driven at 150%, which Qt reports as DPR 2. The
    // crop is already the panel's native pixels; enlarging it to 5120x2880 to
    // match the reported ratio would be pure invention.
    QTest::newRow("fractional scale keeps native pixels")
      << QSize(3840, 2160) << QSize(2560, 1440) << qreal(2.0)
      << QSize(3840, 2160) << qreal(1.5);

    // eDP-1: a 1920x1080 panel at 100%. The portal upscaled it into the
    // composite, so the crop holds more pixels than the panel has; dropping
    // back discards only what the portal invented.
    QTest::newRow("portal upscale is undone")
      << QSize(2880, 1620) << QSize(1920, 1080) << qreal(1.0)
      << QSize(1920, 1080) << qreal(1.0);

    // Composite scale and reported ratio agree: nothing to do.
    QTest::newRow("matching scale is untouched")
      << QSize(3840, 2160) << QSize(1920, 1080) << qreal(2.0)
      << QSize(3840, 2160) << qreal(2.0);

    QTest::newRow("unscaled display is untouched")
      << QSize(1920, 1080) << QSize(1920, 1080) << qreal(1.0)
      << QSize(1920, 1080) << qreal(1.0);

    // Over budget on one axis only still clamps both, so the crop keeps the
    // monitor's logical size rather than acquiring a new aspect ratio.
    QTest::newRow("single axis over budget clamps both")
      << QSize(2880, 1080) << QSize(1920, 1080) << qreal(1.0)
      << QSize(1920, 1080) << qreal(1.0);
}

void TestDprScaling::fitCrop()
{
    QFETCH(QSize, cropped);
    QFETCH(QSize, logical);
    QFETCH(qreal, reportedDpr);
    QFETCH(QSize, expectedSize);
    QFETCH(qreal, expectedDpr);

    const Fit fit = DprScaling::fitCrop(cropped, logical, reportedDpr);

    QCOMPARE(fit.size, expectedSize);
    QCOMPARE(fit.dpr, expectedDpr);
}

/// The invariant behind every row above: a crop is never scaled up, whatever
/// the reported ratio claims.
void TestDprScaling::fitCrop_neverEnlarges()
{
    for (int width = 800; width <= 4000; width += 400) {
        const QSize cropped(width, width / 2);
        const Fit fit =
          DprScaling::fitCrop(cropped, QSize(1920, 960), qreal(2.0));
        QVERIFY(fit.size.width() <= cropped.width());
        QVERIFY(fit.size.height() <= cropped.height());
    }
}

void TestDprScaling::fitCrop_degenerateLogicalSize()
{
    // A zero logical width would divide by zero when deriving the ratio.
    const Fit fit = DprScaling::fitCrop(QSize(0, 0), QSize(0, 0), qreal(2.0));
    QCOMPARE(fit.dpr, qreal(2.0));
}

void TestDprScaling::toDevicePixels_data()
{
    QTest::addColumn<QRect>("logical");
    QTest::addColumn<qreal>("dpr");
    QTest::addColumn<QRect>("expected");

    QTest::newRow("unscaled is identity")
      << QRect(100, 50, 300, 200) << qreal(1.0) << QRect(100, 50, 300, 200);

    QTest::newRow("whole ratio doubles everything")
      << QRect(100, 50, 300, 200) << qreal(2.0) << QRect(200, 100, 600, 400);

    // 301 * 1.5 = 451.5. Scaling the extent on its own truncates to 451, while
    // the far edge at 401 * 1.5 = 601.5 rounds to 602 -- so the rectangle must
    // be 452 wide to reach it.
    QTest::newRow("fractional ratio keeps the far edge")
      << QRect(100, 50, 301, 201) << qreal(1.5) << QRect(150, 75, 452, 302);

    QTest::newRow("origin at zero")
      << QRect(0, 0, 101, 101) << qreal(1.5) << QRect(0, 0, 152, 152);

    QTest::newRow("empty rect stays empty")
      << QRect(10, 10, 0, 0) << qreal(1.5) << QRect(15, 15, 0, 0);
}

void TestDprScaling::toDevicePixels()
{
    QFETCH(QRect, logical);
    QFETCH(qreal, dpr);
    QFETCH(QRect, expected);

    QCOMPARE(DprScaling::toDevicePixels(logical, dpr), expected);
}

/// The compatibility guarantee: on displays that report a whole-number ratio
/// this must agree with scaling origin and extent directly, so nothing changes
/// where nothing was broken.
void TestDprScaling::toDevicePixels_wholeRatiosScaleOriginAndExtentDirectly()
{
    const QRect logical(37, 91, 613, 227);
    for (int dpr = 1; dpr <= 4; ++dpr) {
        const QRect expected(logical.left() * dpr,
                             logical.top() * dpr,
                             logical.width() * dpr,
                             logical.height() * dpr);
        QCOMPARE(DprScaling::toDevicePixels(logical, qreal(dpr)), expected);
    }
}

/// Why edges are scaled rather than extents: rectangles that touched in
/// logical space must still touch afterwards, with no seam and no overlap.
void TestDprScaling::toDevicePixels_adjacentRectsShareAnEdge()
{
    for (int split = 1; split < 200; ++split) {
        const QRect left(0, 0, split, 50);
        const QRect right(split, 0, 200 - split, 50);

        const QRect scaledLeft = DprScaling::toDevicePixels(left, qreal(1.5));
        const QRect scaledRight = DprScaling::toDevicePixels(right, qreal(1.5));

        QCOMPARE(scaledLeft.left() + scaledLeft.width(), scaledRight.left());
    }
}

void TestDprScaling::toPixmapPoint_data()
{
    QTest::addColumn<QPoint>("global");
    QTest::addColumn<QPoint>("origin");
    QTest::addColumn<qreal>("dpr");
    QTest::addColumn<QPoint>("expected");

    QTest::newRow("primary monitor unscaled")
      << QPoint(100, 200) << QPoint(0, 0) << qreal(1.0) << QPoint(100, 200);

    // DP-1 sits at x=1920 and its capture is 1.5x its logical size, so both the
    // offset and the ratio have to be applied.
    QTest::newRow("offset monitor at fractional ratio")
      << QPoint(2000, 100) << QPoint(1920, 0) << qreal(1.5) << QPoint(120, 150);

    // eDP-1 starts 360 below the top of the desktop.
    QTest::newRow("offset monitor unscaled")
      << QPoint(100, 800) << QPoint(0, 360) << qreal(1.0) << QPoint(100, 440);

    QTest::newRow("whole ratio")
      << QPoint(1930, 5) << QPoint(1920, 0) << qreal(2.0) << QPoint(20, 10);

    QTest::newRow("top-left corner maps to the origin")
      << QPoint(1920, 0) << QPoint(1920, 0) << qreal(1.5) << QPoint(0, 0);

    QTest::newRow("fractional ratio rounds")
      << QPoint(1921, 1) << QPoint(1920, 0) << qreal(1.5) << QPoint(2, 2);
}

void TestDprScaling::toPixmapPoint()
{
    QFETCH(QPoint, global);
    QFETCH(QPoint, origin);
    QFETCH(qreal, dpr);
    QFETCH(QPoint, expected);

    QCOMPARE(DprScaling::toPixmapPoint(global, origin, dpr), expected);
}

void TestDprScaling::toDevicePixelsLength_data()
{
    QTest::addColumn<qreal>("logical");
    QTest::addColumn<qreal>("dpr");
    QTest::addColumn<int>("expected");

    // The magnifier covers a count of logical pixels, so the number of
    // captured pixels behind it varies with each display's scale.
    QTest::newRow("unscaled") << qreal(7) << qreal(1.0) << 7;
    QTest::newRow("fractional rounds up") << qreal(7) << qreal(1.5) << 11;
    QTest::newRow("larger magnifier at fractional")
      << qreal(11) << qreal(1.5) << 17;
    QTest::newRow("whole ratio") << qreal(7) << qreal(2.0) << 14;
    QTest::newRow("zero stays zero") << qreal(0) << qreal(1.5) << 0;

    // Dividing the magnifier's span by a display's rendering scale leaves a
    // fraction of a logical pixel, which must survive to the conversion:
    // rounding it away first loses a captured pixel.
    QTest::newRow("fractional length is not truncated")
      << qreal(3.5) << qreal(2.0) << 7;
}

void TestDprScaling::toDevicePixelsLength()
{
    QFETCH(qreal, logical);
    QFETCH(qreal, dpr);
    QFETCH(int, expected);

    QCOMPARE(DprScaling::toDevicePixels(logical, dpr), expected);
}

QTEST_APPLESS_MAIN(TestDprScaling)
#include "tst_dprscaling.moc"
