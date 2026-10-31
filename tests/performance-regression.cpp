// SPDX-License-Identifier: GPL-3.0-or-later
#include "tools/pixelate/pixelatetool.h"
#include "tools/rectangle/rectangletool.h"
#include "widgets/capture/capturewidget.h"
#include "widgets/capture/overlaymessage.h"
#include "widgets/panel/utilitypanel.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QPaintEvent>
#include <QTest>
#include <algorithm>
#include <iostream>
#include <stdexcept>

static void require(bool condition, const char* message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

static QPixmap pattern(QSize size, qreal scale = 1)
{
    QImage image(size, QImage::Format_RGB32);
    for (int y = 0; y < size.height(); ++y) {
        auto* row = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < size.width(); ++x) {
            row[x] = qRgb(x % 256, y % 256, (x + y) % 256);
        }
    }
    QPixmap result = QPixmap::fromImage(image);
    result.setDevicePixelRatio(scale);
    return result;
}

static void drain()
{
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents();
}

class PaintRecorder : public QObject
{
public:
    QRegion dirty;
    bool eventFilter(QObject*, QEvent* event) override
    {
        if (event->type() == QEvent::Paint) {
            dirty |= static_cast<QPaintEvent*>(event)->region();
        }
        return false;
    }
};

static qint64 area(const QRegion& region)
{
    qint64 result = 0;
    for (const auto& rect : region) {
        result += qint64(rect.width()) * rect.height();
    }
    return result;
}

static void testBackground()
{
    for (qreal scale : { 1.0, 1.25, 2.0 }) {
        auto screenshot = pattern(QSize(400, 300) * scale, scale);
        CaptureBackground cache;
        for (int opacity : { 0, 128, 188, 255 }) {
            for (QRect selection : { QRect(),
                                     QRect(90, 70, 180, 160),
                                     QRect(0, 0, 400, 300),
                                     QRect(-20, 10, 65, 24) }) {
                const QRect label(selection.topLeft(), QSize(140, 35));
                QImage expected(screenshot.size(), QImage::Format_RGB32);
                expected.setDevicePixelRatio(scale);
                QImage actual = expected.copy();
                const auto draw = [&](QImage& target, bool cached) {
                    QPainter p(&target);
                    if (cached) {
                        cache.paint(p,
                                    screenshot,
                                    QRegion(selection) | QRegion(label),
                                    opacity);
                    } else {
                        p.drawPixmap(0, 0, screenshot);
                    }
                    p.fillRect(label, QColor(180, 30, 90, 200));
                    QRegion dim =
                      QRegion(QRect(0, 0, 400, 300)).subtracted(selection);
                    if (cached) {
                        dim &= label;
                    }
                    p.setClipRegion(dim);
                    p.fillRect(QRect(0, 0, 400, 300), QColor(0, 0, 0, opacity));
                };
                draw(expected, false);
                draw(actual, true);
                require(actual == expected,
                        "cached dimming differs from full composition");
            }
        }
        // The cache must refresh when the screenshot changes at the same size.
        screenshot.fill(Qt::green);
        QImage target(screenshot.size(), QImage::Format_RGB32);
        target.setDevicePixelRatio(scale);
        QPainter painter(&target);
        cache.paint(painter, screenshot, {}, 0);
        painter.end();
        require(target.pixelColor(20, 20) == QColor(Qt::green),
                "stale dimmed screenshot");
    }
    std::cout << "PASS cached background: exact pixels at DPR 1, 1.25 and 2, "
                 "opacity/edge/invalidation cases\n";
}

static void mouse(QWidget& widget,
                  QEvent::Type type,
                  QPoint pos,
                  Qt::MouseButton button,
                  Qt::MouseButtons buttons)
{
    QMouseEvent event(type,
                      QPointF(pos),
                      QPointF(widget.mapToGlobal(pos)),
                      button,
                      buttons,
                      Qt::NoModifier);
    QApplication::sendEvent(&widget, &event);
}

static void testSelection()
{
    QWidget parent;
    parent.resize(1000, 700);
    SelectionWidget selection(Qt::red, &parent);
    selection.setGeometry(QRect(100, 100, 300, 200));
    parent.show();
    drain();
    int changed = 0;
    QObject::connect(
      &selection, &SelectionWidget::geometryChanged, [&]() { ++changed; });
    selection.setGeometry(QRect(101, 102, 301, 202));
    require(changed == 1,
            "selection move/resize emitted duplicate notifications");
    selection.setGeometry(selection.geometry());
    require(changed == 1, "unchanged selection emitted a notification");
    require(selection.getMouseSide(selection.geometry().topLeft()) ==
              SelectionWidget::TOPLEFT_SIDE,
            "handle hit testing changed");
    const QRect before = selection.geometry();
    mouse(parent,
          QEvent::MouseButtonPress,
          before.center(),
          Qt::LeftButton,
          Qt::LeftButton);
    mouse(parent,
          QEvent::MouseMove,
          before.center() + QPoint(20, 15),
          Qt::NoButton,
          Qt::LeftButton);
    mouse(parent,
          QEvent::MouseButtonRelease,
          before.center() + QPoint(20, 15),
          Qt::LeftButton,
          Qt::NoButton);
    require(selection.geometry() == before.translated(20, 15),
            "moving selection changed its size or offset");
    const QRect beforeResize = selection.geometry();
    mouse(parent,
          QEvent::MouseButtonPress,
          beforeResize.bottomRight(),
          Qt::LeftButton,
          Qt::LeftButton);
    mouse(parent,
          QEvent::MouseMove,
          beforeResize.bottomRight() + QPoint(12, 9),
          Qt::NoButton,
          Qt::LeftButton);
    mouse(parent,
          QEvent::MouseButtonRelease,
          beforeResize.bottomRight() + QPoint(12, 9),
          Qt::LeftButton,
          Qt::NoButton);
    require(selection.geometry() == beforeResize.adjusted(0, 0, 12, 9),
            "resizing selection failed");
    int settled = 0;
    QObject::connect(
      &selection, &SelectionWidget::geometrySettled, [&]() { ++settled; });
    selection.moveRight();
    QTest::qWait(850);
    require(settled == 1, "keyboard settle timer kept firing");
    const QRect animated(80, 75, 240, 180);
    selection.setGeometryAnimated(animated);
    QTest::qWait(250);
    require(selection.geometry() == animated,
            "animation did not update logical selection geometry");
    require(static_cast<QWidget*>(&selection)->geometry() == parent.rect(),
            "selection overlay moved/resized during drag");
    std::cout << "PASS selection: one notification, no-op suppression, "
                 "dragging, resizing, animation, single-shot keyboard timer\n";
}

static void testMagnifier()
{
    QWidget parent;
    parent.resize(500, 400);
    parent.show();
    for (qreal scale : { 1.0, 2.0 }) {
        const QPixmap source = pattern(QSize(500, 400) * scale, scale);
        for (bool square : { false, true }) {
            MagnifierWidget magnifier(source, Qt::red, square, &parent);
            magnifier.show();
            for (QPoint pos :
                 { QPoint(250, 200), QPoint(0, 0), QPoint(499, 399) }) {
                magnifier.setCursorPosition(pos);
                drain();
                require(parent.rect().contains(magnifier.geometry()),
                        "magnifier escaped capture bounds");
                require(magnifier.width() < 200 && magnifier.height() < 200,
                        "magnifier invalidates a full screen");
                if (pos == QPoint(250, 200)) {
                    auto lens = magnifier.grab().toImage();
                    auto expected = source.toImage().pixelColor(pos * scale);
                    require(lens.pixelColor(lens.width() / 2,
                                            lens.height() / 2) == expected,
                            "magnifier sampled wrong physical pixel");
                }
            }
        }
    }
    std::cout << "PASS magnifier: bounded lens, both shapes, screen edges and "
                 "correct HiDPI pixel sampling\n";
}

class CaptureWidgetTest
{
public:
    static void run()
    {
        CaptureRequest request(CaptureRequest::GRAPHICAL_MODE);
        request.setInitialSelection(QRect(40, 40, 180, 120));
        CaptureWidget widget(request, false);
        widget.setAttribute(Qt::WA_DeleteOnClose, false);
        const qreal scale = widget.devicePixelRatioF();
        widget.m_context.origScreenshot =
          pattern(QSize(1000, 700) * scale, scale);
        widget.m_context.screenshot = widget.m_context.origScreenshot;
        widget.resize(1000, 700);
        widget.show();
        widget.activateWindow();
        drain();
        require(widget.m_buttonHandler->isVisible(),
                "initial selection lost its toolbar on show");
        widget.m_selection->setGeometry(QRect(150, 120, 400, 300));
        widget.m_selection->show();
        drain();
        PaintRecorder recorder;
        widget.installEventFilter(&recorder);
        widget.m_selection->setGeometry(QRect(151, 120, 401, 300));
        drain();
        require(!recorder.dirty.isEmpty(), "selection did not repaint");
        const auto dirtyArea = area(recorder.dirty);
        require(dirtyArea < widget.width() * widget.height() / 2,
                "selection still repaints the entire window");
        std::cout
          << "PASS actual capture repaint: " << dirtyArea << "/"
          << widget.width() * widget.height()
          << " logical pixels invalidated for a small selection change\n";
        require(widget.m_context.selection ==
                  widget.extendedRect(widget.m_selection->geometry()),
                "export coordinates lag behind selection");
        recorder.dirty = {};
        const QRect oldLabel = widget.m_xywhRect;
        widget.xywhTick();
        drain();
        require(area(recorder.dirty) <= oldLabel.width() * oldLabel.height(),
                "label expiry repaints unrelated pixels");

        CaptureContext context = widget.m_context;
        context.toolSize = 12;
        context.color = Qt::red;
        context.mousePos = QPoint(50, 50);
        RectangleTool first(&widget);
        first.drawStart(context);
        first.drawMove(QPoint(320, 240));
        widget.m_captureToolObjects.append(&first);
        context.mousePos = QPoint(100, 100);
        context.color = Qt::blue;
        RectangleTool moving(&widget);
        moving.drawStart(context);
        moving.drawMove(QPoint(300, 300));
        widget.m_captureToolObjects.append(&moving);
        context.mousePos = QPoint(90, 90);
        PixelateTool pixelate(&widget);
        pixelate.drawStart(context);
        pixelate.drawMove(QPoint(340, 320));
        widget.m_captureToolObjects.append(&pixelate);
        widget.updateLayersPanel();
        widget.m_panel->setActiveLayer(1);
        for (bool insecure : { false, true }) {
            ConfigHandler().setInsecurePixelate(insecure);
            for (int thickness : { 1, 8 }) {
                widget.m_captureToolObjects.at(2)->onSizeChanged(thickness);
                widget.drawToolsData(
                  false); // invalidate cached prefix after edit
                qint64 prefixKey = 0;
                for (int i = 0; i < 3; ++i) {
                    widget.m_captureToolObjects.at(1)->move(
                      QPoint(130 + i * 15, 130));
                    widget.drawToolsData(false, true);
                    if (i) {
                        require(prefixKey == widget.m_dragBackground.cacheKey(),
                                "unchanged layers were rerendered during drag");
                    }
                    prefixKey = widget.m_dragBackground.cacheKey();
                    QPixmap reference = widget.m_context.origScreenshot;
                    for (auto tool :
                         widget.m_captureToolObjects.captureToolObjects()) {
                        widget.processPixmapWithTool(&reference, tool);
                    }
                    require(
                      reference.toImage() ==
                        widget.m_context.screenshot.toImage(),
                      "cached annotation replay differs from full replay");
                }
            }
        }
        widget.m_captureToolObjects.at(0)->onColorChanged(Qt::green);
        widget.drawToolsData(false);
        require(widget.m_dragBackground.isNull(),
                "editing did not invalidate annotation prefix");
        CaptureToolObjects saved;
        saved = widget.m_captureToolObjects;
        widget.m_captureToolObjects.removeAt(0);
        widget.drawToolsData(false);
        require(widget.m_dragBackground.isNull(),
                "deletion did not invalidate annotation prefix");
        widget.setCaptureToolObjects(saved);
        require(widget.m_dragBackground.isNull(),
                "undo did not invalidate annotation prefix");
        ConfigHandler().setInsecurePixelate(false);
        std::cout
          << "PASS annotation prefix: exact full-replay comparison with later "
             "secure pixelation, pixelation and blur; cache invalidation\n";
        widget.hide();
        widget.removeEventFilter(&recorder);
    }
};

static void benchmark()
{
    const QSize size(5760, 3240);
    const auto screenshot = pattern(size);
    QImage target(size, QImage::Format_RGB32);
    CaptureBackground cache;
    for (bool optimized : { false, true }) {
        std::vector<double> timings;
        QRect previous(1200, 700, 2000, 1500);
        for (int i = 0; i < 120; ++i) {
            QRect selection = previous.adjusted(0, 0, 2, 2);
            const QRegion border =
              QRegion(selection.adjusted(-20, -20, 20, 20))
                .subtracted(selection.adjusted(20, 20, -20, -20));
            QRegion dirty = QRegion(previous).xored(selection) | border;
            QElapsedTimer timer;
            timer.start();
            {
                QPainter p(&target);
                if (optimized) {
                    p.setClipRegion(dirty);
                    cache.paint(p, screenshot, selection, 188);
                } else {
                    p.drawPixmap(0, 0, screenshot);
                    p.setClipRegion(
                      QRegion(QRect(QPoint(), size)).subtracted(selection));
                    p.fillRect(QRect(QPoint(), size), QColor(0, 0, 0, 188));
                }
            }
            if (i >= 20)
                timings.push_back(timer.nsecsElapsed() / 1e6);
            previous = selection;
        }
        std::sort(timings.begin(), timings.end());
        std::cout << "BENCH synthetic 5760x3240 "
                  << (optimized ? "cached regional" : "original full")
                  << " median_ms=" << timings[50] << " p95_ms=" << timings[95]
                  << '\n';
    }
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("flameshot");
    app.setOrganizationName("flameshot");
    app.setQuitOnLastWindowClosed(false);
    ConfigHandler().setShowHelp(false);
    ConfigHandler().setShowSelectionGeometry(4);
    ConfigHandler().showSelectionGeometryHideTime(0);
    ConfigHandler().setShowMagnifier(false);
    try {
        testBackground();
        testSelection();
        testMagnifier();
        CaptureWidgetTest::run();
        if (app.arguments().contains("--benchmark"))
            benchmark();
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
