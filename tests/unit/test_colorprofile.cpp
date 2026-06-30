// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

// Unit tests for the color profile handling that fixes #4341 (screenshots
// saved without an ICC profile look desaturated on wide-gamut displays).
// These tests are platform-neutral: they exercise the pure helpers and the
// PNG/ICC round-trip, so they run headless on any platform without a real
// wide-gamut display.

#include "utils/colorprofileprovider.h"

#include <QBuffer>
#include <QColorSpace>
#include <QImage>
#include <QPixmap>
#include <QtTest>

class TestColorProfile : public QObject
{
    Q_OBJECT
private slots:
    void taggedAppliesValidColorSpace();
    void taggedIsNoOpForInvalidColorSpace();
    void resolveFallbackChain();
    void pngRoundTripEmbedsIccProfile();
    void untaggedPngHasNoIccProfile();
};

// tagged() must label the pixels with a valid color space.
void TestColorProfile::taggedAppliesValidColorSpace()
{
    QPixmap pixmap(8, 8);
    pixmap.fill(Qt::blue);
    // A freshly filled pixmap has no color space.
    QVERIFY(!pixmap.toImage().colorSpace().isValid());

    const QColorSpace p3(QColorSpace::DisplayP3);
    const QImage tagged = ColorProfileProvider::tagged(pixmap, p3);

    QVERIFY(tagged.colorSpace().isValid());
    QCOMPARE(tagged.colorSpace(), p3);
}

// An invalid color space must leave the image untagged (previous behavior).
void TestColorProfile::taggedIsNoOpForInvalidColorSpace()
{
    QPixmap pixmap(8, 8);
    pixmap.fill(Qt::green);

    const QImage tagged = ColorProfileProvider::tagged(pixmap, QColorSpace());

    QVERIFY(!tagged.colorSpace().isValid());
}

// Fallback chain: a valid platform profile is kept; otherwise Display P3 for
// wide-gamut displays; otherwise invalid (leave untagged / assume sRGB).
void TestColorProfile::resolveFallbackChain()
{
    const QColorSpace srgb(QColorSpace::SRgb);

    // Valid platform profile is returned unchanged, regardless of wide-gamut.
    QCOMPARE(ColorProfileProvider::resolve(srgb, false), srgb);
    QCOMPARE(ColorProfileProvider::resolve(srgb, true), srgb);

    // No platform profile + wide-gamut display -> Display P3.
    const QColorSpace wide =
      ColorProfileProvider::resolve(QColorSpace(), true);
    QVERIFY(wide.isValid());
    QCOMPARE(wide, QColorSpace(QColorSpace::DisplayP3));

    // No platform profile + non-wide-gamut display -> stay untagged.
    QVERIFY(!ColorProfileProvider::resolve(QColorSpace(), false).isValid());
}

// The core regression: a tagged image must keep a valid embedded ICC profile
// across a PNG save/load round-trip (proves Qt writes the iCCP chunk).
void TestColorProfile::pngRoundTripEmbedsIccProfile()
{
    QImage image(16, 16, QImage::Format_RGB32);
    image.fill(Qt::red);
    image.setColorSpace(QColorSpace(QColorSpace::DisplayP3));
    QVERIFY(image.colorSpace().isValid());

    QByteArray bytes;
    QBuffer buffer(&bytes);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(image.save(&buffer, "PNG"));
    buffer.close();

    QImage reloaded;
    QVERIFY(reloaded.loadFromData(bytes, "PNG"));
    QVERIFY(reloaded.colorSpace().isValid());
    QVERIFY(!reloaded.colorSpace().iccProfile().isEmpty());
}

// Sanity check documenting the bug: without a color space the PNG is untagged.
void TestColorProfile::untaggedPngHasNoIccProfile()
{
    QImage image(16, 16, QImage::Format_RGB32);
    image.fill(Qt::red);

    QByteArray bytes;
    QBuffer buffer(&bytes);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(image.save(&buffer, "PNG"));
    buffer.close();

    QImage reloaded;
    QVERIFY(reloaded.loadFromData(bytes, "PNG"));
    QVERIFY(!reloaded.colorSpace().isValid());
}

QTEST_MAIN(TestColorProfile)
#include "test_colorprofile.moc"
