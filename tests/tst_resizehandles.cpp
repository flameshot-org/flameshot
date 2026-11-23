// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/resizehandles.h"

#include <QTest>

using ResizeHandles::Handle;

Q_DECLARE_METATYPE(ResizeHandles::Handle)
Q_DECLARE_METATYPE(std::optional<Qt::CursorShape>)

namespace {
// left 100, top 100, right 300, bottom 200
const QRect BOX(QPoint(100, 100), QPoint(300, 200));
const int TOLERANCE = 8;
}

class TestResizeHandles : public QObject
{
    Q_OBJECT

private slots:
    void boxOf_data();
    void boxOf();

    void handleAt_data();
    void handleAt();
    void handleAt_tinyBoxPrefersCorner();
    void handleAt_emptyBoxHasNoHandles();

    void resized_data();
    void resized();

    void endpointAt_data();
    void endpointAt();

    void movedEndpoint_data();
    void movedEndpoint();

    void stretched_data();
    void stretched();
    void stretched_degenerateAxisIsLeftAlone();

    void objectCursor_data();
    void objectCursor();
};

void TestResizeHandles::boxOf_data()
{
    QTest::addColumn<QPoint>("first");
    QTest::addColumn<QPoint>("second");
    QTest::addColumn<QRect>("expected");

    const QRect box(QPoint(0, 0), QPoint(10, 10));
    QTest::newRow("drawn down-right") << QPoint(0, 0) << QPoint(10, 10) << box;
    // QRect(first, second).normalized() loses a pixel on each reversed side.
    QTest::newRow("drawn up-left") << QPoint(10, 10) << QPoint(0, 0) << box;
    QTest::newRow("drawn up-right") << QPoint(0, 10) << QPoint(10, 0) << box;
    QTest::newRow("drawn down-left") << QPoint(10, 0) << QPoint(0, 10) << box;
    QTest::newRow("reversed, one pixel apart")
      << QPoint(1, 0) << QPoint(0, 10) << QRect(QPoint(0, 0), QPoint(1, 10));
    QTest::newRow("single point")
      << QPoint(5, 5) << QPoint(5, 5) << QRect(QPoint(5, 5), QPoint(5, 5));
}

void TestResizeHandles::boxOf()
{
    QFETCH(QPoint, first);
    QFETCH(QPoint, second);
    QFETCH(QRect, expected);

    QCOMPARE(ResizeHandles::boxOf(first, second), expected);
}

void TestResizeHandles::handleAt_data()
{
    QTest::addColumn<QPoint>("pos");
    QTest::addColumn<Handle>("expected");

    QTest::newRow("top-left") << QPoint(100, 100) << ResizeHandles::TopLeft;
    QTest::newRow("top") << QPoint(200, 100) << ResizeHandles::Top;
    QTest::newRow("top-right") << QPoint(300, 100) << ResizeHandles::TopRight;
    QTest::newRow("right") << QPoint(300, 150) << ResizeHandles::Right;
    QTest::newRow("bottom-right")
      << QPoint(300, 200) << ResizeHandles::BottomRight;
    QTest::newRow("bottom") << QPoint(200, 200) << ResizeHandles::Bottom;
    QTest::newRow("bottom-left")
      << QPoint(100, 200) << ResizeHandles::BottomLeft;
    QTest::newRow("left") << QPoint(100, 150) << ResizeHandles::Left;

    QTest::newRow("within tolerance")
      << QPoint(105, 94) << ResizeHandles::TopLeft;
    QTest::newRow("just outside tolerance")
      << QPoint(100, 91) << ResizeHandles::None;
    QTest::newRow("interior") << QPoint(150, 130) << ResizeHandles::None;
    QTest::newRow("on an edge between handles")
      << QPoint(150, 100) << ResizeHandles::None;
}

void TestResizeHandles::handleAt()
{
    QFETCH(QPoint, pos);
    QFETCH(Handle, expected);

    QCOMPARE(ResizeHandles::handleAt(BOX, pos, TOLERANCE), expected);
}

void TestResizeHandles::handleAt_tinyBoxPrefersCorner()
{
    // Every handle is within reach; a corner resizes both axes, so it wins.
    const QRect tiny(QPoint(100, 100), QPoint(102, 102));

    QCOMPARE(ResizeHandles::handleAt(tiny, QPoint(101, 101), TOLERANCE),
             ResizeHandles::TopLeft);
}

void TestResizeHandles::handleAt_emptyBoxHasNoHandles()
{
    QCOMPARE(ResizeHandles::handleAt(QRect(), QPoint(0, 0), TOLERANCE),
             ResizeHandles::None);
}

void TestResizeHandles::resized_data()
{
    QTest::addColumn<Handle>("handle");
    QTest::addColumn<QPoint>("pos");
    QTest::addColumn<QRect>("expected");

    QTest::newRow("top-left moves its corner")
      << ResizeHandles::TopLeft << QPoint(50, 40)
      << QRect(QPoint(50, 40), QPoint(300, 200));
    QTest::newRow("top-right moves its corner")
      << ResizeHandles::TopRight << QPoint(320, 90)
      << QRect(QPoint(100, 90), QPoint(320, 200));
    QTest::newRow("bottom-right moves its corner")
      << ResizeHandles::BottomRight << QPoint(350, 260)
      << QRect(QPoint(100, 100), QPoint(350, 260));
    QTest::newRow("bottom-left moves its corner")
      << ResizeHandles::BottomLeft << QPoint(90, 210)
      << QRect(QPoint(90, 100), QPoint(300, 210));

    QTest::newRow("top ignores horizontal motion")
      << ResizeHandles::Top << QPoint(999, 80)
      << QRect(QPoint(100, 80), QPoint(300, 200));
    QTest::newRow("right ignores vertical motion")
      << ResizeHandles::Right << QPoint(350, 999)
      << QRect(QPoint(100, 100), QPoint(350, 200));
    QTest::newRow("bottom ignores horizontal motion")
      << ResizeHandles::Bottom << QPoint(0, 250)
      << QRect(QPoint(100, 100), QPoint(300, 250));
    QTest::newRow("left ignores vertical motion")
      << ResizeHandles::Left << QPoint(120, 0)
      << QRect(QPoint(120, 100), QPoint(300, 200));

    QTest::newRow("edge dragged past the opposite side flips")
      << ResizeHandles::Right << QPoint(40, 150)
      << QRect(QPoint(40, 100), QPoint(100, 200));
    QTest::newRow("corner dragged past the opposite corner flips")
      << ResizeHandles::BottomRight << QPoint(50, 50)
      << QRect(QPoint(50, 50), QPoint(100, 100));

    // Coinciding edges would leave the object's two points equal on that
    // axis; both equal makes it invalid and impossible to select again.
    QTest::newRow("edge onto the opposite side keeps one pixel")
      << ResizeHandles::Right << QPoint(100, 150)
      << QRect(QPoint(100, 100), QPoint(101, 200));
    QTest::newRow("corner onto the opposite corner keeps one pixel")
      << ResizeHandles::BottomRight << QPoint(100, 100)
      << QRect(QPoint(100, 100), QPoint(101, 101));
    QTest::newRow("top onto the bottom keeps one pixel")
      << ResizeHandles::Top << QPoint(0, 200)
      << QRect(QPoint(100, 199), QPoint(300, 200));

    QTest::newRow("no handle leaves the box alone")
      << ResizeHandles::None << QPoint(0, 0) << BOX;
}

void TestResizeHandles::resized()
{
    QFETCH(Handle, handle);
    QFETCH(QPoint, pos);
    QFETCH(QRect, expected);

    QCOMPARE(ResizeHandles::resized(BOX, handle, pos), expected);
}

void TestResizeHandles::endpointAt_data()
{
    QTest::addColumn<QPoint>("first");
    QTest::addColumn<QPoint>("second");
    QTest::addColumn<QPoint>("pos");
    QTest::addColumn<Handle>("expected");

    const QPoint a(100, 100);
    const QPoint b(300, 200);
    QTest::newRow("start") << a << b << QPoint(100, 100)
                           << ResizeHandles::Start;
    QTest::newRow("end within tolerance")
      << a << b << QPoint(305, 195) << ResizeHandles::End;
    QTest::newRow("middle of the line")
      << a << b << QPoint(200, 150) << ResizeHandles::None;
    QTest::newRow("just outside tolerance")
      << a << b << QPoint(100, 109) << ResizeHandles::None;

    // Both ends within reach on a short line: the nearer one wins.
    const QPoint c(104, 100);
    QTest::newRow("short line, nearer the start")
      << a << c << QPoint(101, 100) << ResizeHandles::Start;
    QTest::newRow("short line, nearer the end")
      << a << c << QPoint(103, 100) << ResizeHandles::End;
    QTest::newRow("short line, halfway goes to the end")
      << a << c << QPoint(102, 100) << ResizeHandles::End;
}

void TestResizeHandles::endpointAt()
{
    QFETCH(QPoint, first);
    QFETCH(QPoint, second);
    QFETCH(QPoint, pos);
    QFETCH(Handle, expected);

    QCOMPARE(ResizeHandles::endpointAt(first, second, pos, TOLERANCE),
             expected);
}

void TestResizeHandles::movedEndpoint_data()
{
    QTest::addColumn<QPoint>("first");
    QTest::addColumn<QPoint>("second");
    QTest::addColumn<Handle>("handle");
    QTest::addColumn<QPoint>("pos");
    QTest::addColumn<QPoint>("expectedFirst");
    QTest::addColumn<QPoint>("expectedSecond");

    const QPoint a(100, 100);
    const QPoint b(300, 200);
    QTest::newRow("start moves, end stays")
      << a << b << ResizeHandles::Start << QPoint(50, 60) << QPoint(50, 60)
      << b;
    QTest::newRow("end moves, start stays")
      << a << b << ResizeHandles::End << QPoint(400, 10) << a
      << QPoint(400, 10);

    // Coinciding ends make the object invalid and impossible to select again.
    QTest::newRow("end onto the start keeps one pixel")
      << a << b << ResizeHandles::End << a << a << QPoint(101, 100);
    QTest::newRow("start onto the end keeps one pixel")
      << a << b << ResizeHandles::Start << b << QPoint(299, 200) << b;
    QTest::newRow("vertical line, end onto the start keeps one pixel")
      << a << QPoint(100, 200) << ResizeHandles::End << a << a
      << QPoint(101, 100);

    QTest::newRow("no handle leaves the line alone")
      << a << b << ResizeHandles::None << QPoint(0, 0) << a << b;
}

void TestResizeHandles::movedEndpoint()
{
    QFETCH(QPoint, first);
    QFETCH(QPoint, second);
    QFETCH(Handle, handle);
    QFETCH(QPoint, pos);
    QFETCH(QPoint, expectedFirst);
    QFETCH(QPoint, expectedSecond);

    const auto moved = ResizeHandles::movedEndpoint(first, second, handle, pos);
    QCOMPARE(moved.first, expectedFirst);
    QCOMPARE(moved.second, expectedSecond);
}

void TestResizeHandles::stretched_data()
{
    QTest::addColumn<Handle>("handle");
    QTest::addColumn<QPoint>("pos");
    QTest::addColumn<QVector<QPoint>>("expected");

    // Stroke across BOX: its corners and its centre.
    QTest::newRow("corner scales both axes")
      << ResizeHandles::BottomRight << QPoint(500, 300)
      << QVector<QPoint>{ { 100, 100 }, { 300, 200 }, { 500, 300 } };
    QTest::newRow("edge scales one axis")
      << ResizeHandles::Right << QPoint(200, 999)
      << QVector<QPoint>{ { 100, 100 }, { 150, 150 }, { 200, 200 } };
    QTest::newRow("top-left keeps the bottom-right fixed")
      << ResizeHandles::TopLeft << QPoint(200, 150)
      << QVector<QPoint>{ { 200, 150 }, { 250, 175 }, { 300, 200 } };
    QTest::newRow("edge dragged past the opposite side mirrors")
      << ResizeHandles::Left << QPoint(500, 0)
      << QVector<QPoint>{ { 500, 100 }, { 400, 150 }, { 300, 200 } };
    QTest::newRow("no handle leaves the stroke alone")
      << ResizeHandles::None << QPoint(0, 0)
      << QVector<QPoint>{ { 100, 100 }, { 200, 150 }, { 300, 200 } };
}

void TestResizeHandles::stretched()
{
    QFETCH(Handle, handle);
    QFETCH(QPoint, pos);
    QFETCH(QVector<QPoint>, expected);

    const QVector<QPoint> stroke{ { 100, 100 }, { 200, 150 }, { 300, 200 } };
    QCOMPARE(ResizeHandles::stretched(stroke, BOX, handle, pos), expected);
}

void TestResizeHandles::stretched_degenerateAxisIsLeftAlone()
{
    // A vertical stroke has no width to scale; it must not divide by zero.
    const QVector<QPoint> stroke{ { 100, 100 }, { 100, 200 } };
    const QRect box(QPoint(100, 100), QPoint(100, 200));

    QCOMPARE(ResizeHandles::stretched(
               stroke, box, ResizeHandles::BottomRight, QPoint(150, 300)),
             (QVector<QPoint>{ { 100, 100 }, { 100, 300 } }));
}

void TestResizeHandles::objectCursor_data()
{
    QTest::addColumn<Handle>("handle");
    QTest::addColumn<bool>("movingObject");
    QTest::addColumn<bool>("overObject");
    QTest::addColumn<std::optional<Qt::CursorShape>>("expected");

    using Cursor = std::optional<Qt::CursorShape>;
    QTest::newRow("top-left") << ResizeHandles::TopLeft << false << false
                              << Cursor(Qt::SizeFDiagCursor);
    QTest::newRow("bottom-right") << ResizeHandles::BottomRight << false
                                  << false << Cursor(Qt::SizeFDiagCursor);
    QTest::newRow("top-right") << ResizeHandles::TopRight << false << false
                               << Cursor(Qt::SizeBDiagCursor);
    QTest::newRow("bottom-left") << ResizeHandles::BottomLeft << false << false
                                 << Cursor(Qt::SizeBDiagCursor);
    QTest::newRow("left") << ResizeHandles::Left << false << false
                          << Cursor(Qt::SizeHorCursor);
    QTest::newRow("right") << ResizeHandles::Right << false << false
                           << Cursor(Qt::SizeHorCursor);
    QTest::newRow("top") << ResizeHandles::Top << false << false
                         << Cursor(Qt::SizeVerCursor);
    QTest::newRow("bottom")
      << ResizeHandles::Bottom << false << false << Cursor(Qt::SizeVerCursor);
    QTest::newRow("start") << ResizeHandles::Start << false << false
                           << Cursor(Qt::SizeAllCursor);
    QTest::newRow("end") << ResizeHandles::End << false << false
                         << Cursor(Qt::SizeAllCursor);

    // Handles sit on the object's outline, so the object is under them too.
    QTest::newRow("handle over the object")
      << ResizeHandles::Right << false << true << Cursor(Qt::SizeHorCursor);
    QTest::newRow("over the object")
      << ResizeHandles::None << false << true << Cursor(Qt::OpenHandCursor);
    QTest::newRow("moving the object")
      << ResizeHandles::None << true << true << Cursor(Qt::ClosedHandCursor);
    QTest::newRow("moving, pointer ahead of the object")
      << ResizeHandles::None << true << false << Cursor(Qt::ClosedHandCursor);
    QTest::newRow("away from the object")
      << ResizeHandles::None << false << false << Cursor();
}

void TestResizeHandles::objectCursor()
{
    QFETCH(Handle, handle);
    QFETCH(bool, movingObject);
    QFETCH(bool, overObject);
    QFETCH(std::optional<Qt::CursorShape>, expected);

    QCOMPARE(ResizeHandles::objectCursor(handle, movingObject, overObject),
             expected);
}

QTEST_APPLESS_MAIN(TestResizeHandles)
#include "tst_resizehandles.moc"
