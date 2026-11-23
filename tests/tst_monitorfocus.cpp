// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/monitorfocus.h"

#include <QTest>

using MonitorFocus::ActiveMonitorTracker;

Q_DECLARE_METATYPE(std::optional<QPoint>)

// Geometry as Qt reports it on a real mixed-DPR two-monitor setup:
// index 0 = DP-1 (2560x1440 at +1920+0), index 1 = eDP-1 (1920x1080 at +0+360).
// The layout is L-shaped, so it has a genuine gap above eDP-1.
static QVector<QRect> layout()
{
    return { QRect(1920, 0, 2560, 1440), QRect(0, 360, 1920, 1080) };
}

class TestMonitorFocus : public QObject
{
    Q_OBJECT

private slots:
    void monitorIndexAt_data();
    void monitorIndexAt();
    void monitorIndexAt_emptyLayout();

    void monitorWithoutPicker_data();
    void monitorWithoutPicker();

    void tracker_startsUnset();
    void tracker_pointerAtSetsActive();
    void tracker_repeatedPointerAtIsNoOp();
    void tracker_ignoresUnknownAndOutOfRange();
    void tracker_pointerLeftDoesNotClearActive();
    void tracker_latchFreezesSwitching();
    void tracker_beginEditingWithIndexRecoversLostEnter();
    void tracker_beginEditingIsIdempotent();
    void tracker_shouldSwitchTo();
};

void TestMonitorFocus::monitorIndexAt_data()
{
    QTest::addColumn<QPoint>("pos");
    QTest::addColumn<int>("expected");

    QTest::newRow("inside DP-1") << QPoint(2000, 100) << 0;
    QTest::newRow("inside eDP-1") << QPoint(100, 800) << 1;
    // The L-shaped gap: above eDP-1 and left of DP-1, on no monitor at all.
    QTest::newRow("gap above eDP-1") << QPoint(100, 100) << -1;
    // Seam: eDP-1 spans x 0..1919, DP-1 starts at 1920.
    QTest::newRow("seam last px of eDP-1") << QPoint(1919, 800) << 1;
    QTest::newRow("seam first px of DP-1") << QPoint(1920, 800) << 0;
    // Bottom-right is exclusive.
    QTest::newRow("DP-1 last px") << QPoint(4479, 1439) << 0;
    QTest::newRow("just past DP-1") << QPoint(4480, 1439) << -1;
    QTest::newRow("far off desktop") << QPoint(99999, 99999) << -1;
    QTest::newRow("negative coords") << QPoint(-50, -50) << -1;
}

void TestMonitorFocus::monitorIndexAt()
{
    QFETCH(QPoint, pos);
    QFETCH(int, expected);
    QCOMPARE(MonitorFocus::monitorIndexAt(layout(), pos), expected);
}

void TestMonitorFocus::monitorIndexAt_emptyLayout()
{
    QCOMPARE(MonitorFocus::monitorIndexAt({}, QPoint(0, 0)), -1);
}

void TestMonitorFocus::monitorWithoutPicker_data()
{
    QTest::addColumn<bool>("followCursor");
    QTest::addColumn<std::optional<QPoint>>("cursorPos");
    QTest::addColumn<int>("expected");

    using Pos = std::optional<QPoint>;
    QTest::newRow("follow cursor, pointer on DP-1")
      << true << Pos(QPoint(2000, 100)) << 0;
    QTest::newRow("follow cursor, pointer on eDP-1")
      << true << Pos(QPoint(100, 800)) << 1;
    // Wayland: no global pointer position to go by.
    QTest::newRow("follow cursor, pointer unknown") << true << Pos() << -1;
    QTest::newRow("follow cursor, pointer in the gap")
      << true << Pos(QPoint(100, 100)) << -1;
    QTest::newRow("ask, pointer on DP-1")
      << false << Pos(QPoint(2000, 100)) << -1;
}

void TestMonitorFocus::monitorWithoutPicker()
{
    QFETCH(bool, followCursor);
    QFETCH(std::optional<QPoint>, cursorPos);
    QFETCH(int, expected);
    QCOMPARE(
      MonitorFocus::monitorWithoutPicker(followCursor, cursorPos, layout()),
      expected);
}

void TestMonitorFocus::tracker_startsUnset()
{
    ActiveMonitorTracker t(2);
    QCOMPARE(t.activeMonitor(), -1);
    QCOMPARE(t.monitorCount(), 2);
    QVERIFY(!t.isLatched());
}

void TestMonitorFocus::tracker_pointerAtSetsActive()
{
    ActiveMonitorTracker t(2);
    QVERIFY(t.pointerAt(0));
    QCOMPARE(t.activeMonitor(), 0);
    QVERIFY(t.pointerAt(1));
    QCOMPARE(t.activeMonitor(), 1);
}

void TestMonitorFocus::tracker_repeatedPointerAtIsNoOp()
{
    ActiveMonitorTracker t(2);
    QVERIFY(t.pointerAt(0));
    QVERIFY(!t.pointerAt(0)); // no change -> no switch
    QCOMPARE(t.activeMonitor(), 0);
}

void TestMonitorFocus::tracker_ignoresUnknownAndOutOfRange()
{
    ActiveMonitorTracker t(2);
    t.pointerAt(1);
    QVERIFY(!t.pointerAt(-1)); // "between monitors" is not a switch
    QCOMPARE(t.activeMonitor(), 1);
    QVERIFY(!t.pointerAt(7)); // out of range is ignored
    QCOMPARE(t.activeMonitor(), 1);
}

void TestMonitorFocus::tracker_pointerLeftDoesNotClearActive()
{
    // A crossing is Leave(old) then Enter(new). If Leave cleared the active
    // monitor there would be a frame with nothing armed.
    ActiveMonitorTracker t(2);
    t.pointerAt(1);
    QVERIFY(!t.pointerLeft(1));
    QCOMPARE(t.activeMonitor(), 1);
}

void TestMonitorFocus::tracker_latchFreezesSwitching()
{
    ActiveMonitorTracker t(2);
    t.pointerAt(1);
    t.beginEditing();
    QVERIFY(t.isLatched());
    QVERIFY(!t.pointerAt(0));
    QCOMPARE(t.activeMonitor(), 1);
    QVERIFY(!t.shouldSwitchTo(0));
}

void TestMonitorFocus::tracker_beginEditingWithIndexRecoversLostEnter()
{
    // The press tells us authoritatively where the pointer is, even if the
    // Enter that should have armed that monitor was dropped.
    ActiveMonitorTracker t(2);
    t.pointerAt(1);
    t.beginEditing(0);
    QVERIFY(t.isLatched());
    QCOMPARE(t.activeMonitor(), 0);
}

void TestMonitorFocus::tracker_beginEditingIsIdempotent()
{
    ActiveMonitorTracker t(2);
    t.pointerAt(0);
    t.beginEditing();
    t.beginEditing(1); // must not re-latch onto a different monitor
    QCOMPARE(t.activeMonitor(), 0);
    QVERIFY(t.isLatched());
}

void TestMonitorFocus::tracker_shouldSwitchTo()
{
    ActiveMonitorTracker t(2);
    t.pointerAt(0);
    QVERIFY(t.shouldSwitchTo(1));
    QVERIFY(!t.shouldSwitchTo(0));  // already active
    QVERIFY(!t.shouldSwitchTo(-1)); // unknown
    QVERIFY(!t.shouldSwitchTo(2));  // out of range

    ActiveMonitorTracker single(1, 0);
    QVERIFY(!single.shouldSwitchTo(0));
    QVERIFY(!single.shouldSwitchTo(1));
}

QTEST_APPLESS_MAIN(TestMonitorFocus)
#include "tst_monitorfocus.moc"
