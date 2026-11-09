// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#include "gifrecordingoverlay.h"

#include "utils/confighandler.h"

#include <QFont>
#include <QGuiApplication>
#include <QHideEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QShortcut>
#include <QShowEvent>

#include <cmath>

#if defined(FLAMESHOT_HAVE_X11)
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#endif

namespace {
constexpr double kPi = 3.14159265358979323846;

#if defined(FLAMESHOT_HAVE_X11)
// Sets the X11 input shape for a window. An empty input shape makes the
// window ignore all mouse input (clicks pass through to the windows below),
// while a full shape makes it receive input again. Only ShapeInput is
// touched, so rendering and the compositor's bounding shape stay intact.
void setX11InputShape(WId window, bool acceptInput)
{
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        return;
    }

    if (acceptInput) {
        XWindowAttributes attributes;
        if (!XGetWindowAttributes(
              display, static_cast<Window>(window), &attributes)) {
            XCloseDisplay(display);
            return;
        }
        XRectangle rect;
        rect.x = 0;
        rect.y = 0;
        rect.width = static_cast<unsigned short>(qMax(1, attributes.width));
        rect.height = static_cast<unsigned short>(qMax(1, attributes.height));
        XShapeCombineRectangles(display,
                                static_cast<Window>(window),
                                ShapeInput,
                                0,
                                0,
                                &rect,
                                1,
                                ShapeSet,
                                Unsorted);
    } else {
        XShapeCombineRectangles(display,
                                static_cast<Window>(window),
                                ShapeInput,
                                0,
                                0,
                                nullptr,
                                0,
                                ShapeSet,
                                Unsorted);
    }
    XFlush(display);
    XCloseDisplay(display);
}
#endif
} // namespace

GifRecordingOverlay::GifRecordingOverlay(const QRect& region, QWidget* parent)
  : QWidget(parent)
  , m_region(region)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setMouseTracking(true);
    setGeometry(region);
    setCursor(Qt::ArrowCursor);

    ConfigHandler config;
    m_color = config.drawColor();
    m_width = qMax(1, config.drawThickness());

    m_border = new GifRecordingBorder(region);

    auto* undoShortcut = new QShortcut(QKeySequence::Undo, this);
    connect(undoShortcut,
            &QShortcut::activated,
            this,
            &GifRecordingOverlay::undo);
}

GifRecordingOverlay::~GifRecordingOverlay()
{
    delete m_border;
    m_border = nullptr;
}

void GifRecordingOverlay::setDrawingColor(const QColor& color)
{
    if (color.isValid()) {
        m_color = color;
    }
}

void GifRecordingOverlay::setDrawingWidth(int width)
{
    m_width = qBound(1, width, 30);
}

void GifRecordingOverlay::setStrokes(const QVector<Stroke>& strokes)
{
    m_strokes = strokes;
    update();
}

QImage GifRecordingOverlay::renderToImage() const
{
    QImage image(size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    for (const Stroke& stroke : m_strokes) {
        drawStroke(painter, stroke);
    }
    if (m_drawing) {
        drawStroke(painter, m_current);
    }
    painter.end();
    return image;
}

bool GifRecordingOverlay::canBeHiddenForCapture() const
{
    return m_tool == GifDrawingTool::NoTool && !m_drawing;
}

void GifRecordingOverlay::setCaptureHidden(bool hidden)
{
    if (m_captureHidden == hidden) {
        return;
    }
    m_captureHidden = hidden;
    if (hidden) {
        QWidget::hide();
    } else {
        QWidget::show();
    }
}

void GifRecordingOverlay::setTool(GifDrawingTool tool)
{
    if (m_drawing) {
        commitStroke();
    }

    m_tool = tool;
    if (tool == GifDrawingTool::NoTool) {
        setCursor(Qt::ArrowCursor);
        setInputTransparent(true);
    } else {
        setCursor(Qt::CrossCursor);
        setInputTransparent(false);
        raise();
        // The overlay deliberately never accepts keyboard focus
        // (Qt::WindowDoesNotAcceptFocus), so the panel keeps focus for Ctrl+Z
        // and for the text field. Mouse drawing does not need focus.
    }
}

void GifRecordingOverlay::finishTool()
{
    m_drawing = false;
    m_current = Stroke();
    m_tool = GifDrawingTool::NoTool;
    setCursor(Qt::ArrowCursor);
    setInputTransparent(true);
    update();
    emit toolFinished();
}

void GifRecordingOverlay::undo()
{
    if (!m_strokes.isEmpty()) {
        m_strokes.removeLast();
        emit recreateRequested();
    }
}

void GifRecordingOverlay::clear()
{
    m_strokes.clear();
    m_current = Stroke();
    m_drawing = false;
    m_numberCounter = 0;
    emit recreateRequested();
}

void GifRecordingOverlay::addText(const QPoint& pos, const QString& text)
{
    if (text.isEmpty()) {
        return;
    }

    Stroke stroke;
    stroke.tool = GifDrawingTool::Text;
    stroke.points.append(pos);
    stroke.color = m_color;
    stroke.width = m_width;
    stroke.text = text;
    m_strokes.append(stroke);
    update();
}

void GifRecordingOverlay::commitStroke()
{
    if (m_current.tool != GifDrawingTool::NoTool &&
        !m_current.points.isEmpty()) {
        m_strokes.append(m_current);
    }
    m_current = Stroke();
    m_drawing = false;
    update();
}

void GifRecordingOverlay::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton ||
        m_tool == GifDrawingTool::NoTool) {
        return;
    }

    if (m_tool == GifDrawingTool::Text) {
        emit textInputRequested(event->pos());
        return;
    }

    if (m_tool == GifDrawingTool::Number) {
        Stroke stroke;
        stroke.tool = GifDrawingTool::Number;
        stroke.points.append(event->pos());
        stroke.color = m_color;
        stroke.width = m_width;
        stroke.number = ++m_numberCounter;
        m_strokes.append(stroke);
        update();
        finishTool();
        return;
    }

    m_current = Stroke();
    m_current.tool = m_tool;
    m_current.color = m_color;
    m_current.width = m_width;
    m_current.points.append(event->pos());
    m_drawing = true;
    update();
}

void GifRecordingOverlay::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_drawing) {
        return;
    }

    if (m_current.tool == GifDrawingTool::Pencil) {
        m_current.points.append(event->pos());
    } else {
        if (m_current.points.size() < 2) {
            m_current.points.append(event->pos());
        } else {
            m_current.points[1] = event->pos();
        }
    }
    update();
}

void GifRecordingOverlay::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_drawing || event->button() != Qt::LeftButton) {
        return;
    }

    if (m_current.tool == GifDrawingTool::Pencil) {
        m_current.points.append(event->pos());
    } else if (m_current.points.size() < 2) {
        m_current.points.append(event->pos());
    } else {
        m_current.points[1] = event->pos();
    }

    commitStroke();
    finishTool();
}

void GifRecordingOverlay::keyPressEvent(QKeyEvent* event)
{
    if (event->matches(QKeySequence::Undo)) {
        undo();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        if (m_drawing) {
            m_current = Stroke();
            m_drawing = false;
        }
        finishTool();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void GifRecordingOverlay::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (m_captureHidden) {
        // Hidden only for the duration of a screen grab; keep the recording
        // border visible and do not touch the input state.
        if (m_border) {
            m_border->show();
        }
        return;
    }

    if (m_border) {
        m_border->show();
        m_border->raise();
    }

#if defined(FLAMESHOT_HAVE_X11)
    if (QGuiApplication::platformName() == QLatin1String("xcb")) {
        if (Display* display = XOpenDisplay(nullptr)) {
            // Qt::Tool windows get a transient parent from the active window.
            // In this case that window is being destroyed, so drop the hint.
            XDeleteProperty(display,
                            static_cast<Window>(winId()),
                            XA_WM_TRANSIENT_FOR);
            XFlush(display);
            XCloseDisplay(display);
        }
    }
#endif

    // winId() is valid now; make the canvas click-through by default.
    if (!m_applyingInputFlag) {
        applyInputShape();
    }
}

void GifRecordingOverlay::hideEvent(QHideEvent* event)
{
    if (!m_captureHidden && m_border) {
        m_border->hide();
    }
    QWidget::hideEvent(event);
}

void GifRecordingOverlay::setInputTransparent(bool transparent)
{
    if (m_inputTransparent == transparent) {
        return;
    }
    m_inputTransparent = transparent;
    applyInputShape();
}

void GifRecordingOverlay::applyInputShape()
{
#if defined(FLAMESHOT_HAVE_X11)
    if (QGuiApplication::platformName() == QLatin1String("xcb")) {
        // Make sure the native window exists, then change only its input
        // shape. The bounding shape is left untouched and the window is not
        // recreated: both caused flicker or stale pixels on Mutter.
        winId();
        setX11InputShape(winId(), !m_inputTransparent);
        return;
    }
#endif

    const bool wasVisible = isVisible();
    m_applyingInputFlag = true;
    setWindowFlag(Qt::WindowTransparentForInput, m_inputTransparent);
    setAttribute(Qt::WA_TransparentForMouseEvents, m_inputTransparent);
    if (wasVisible) {
        show();
    }
    m_applyingInputFlag = false;
}

void GifRecordingOverlay::requestRecreate()
{
    emit recreateRequested();
}

void GifRecordingOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)


    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Clear the previous frame explicitly. With a translucent window the old
    // pixels can otherwise stay visible after an undo/clear.
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    for (const Stroke& stroke : m_strokes) {
        drawStroke(painter, stroke);
    }
    if (m_drawing) {
        drawStroke(painter, m_current);
    }
}

void GifRecordingOverlay::drawStroke(QPainter& painter,
                                     const Stroke& stroke) const
{
    if (stroke.points.isEmpty()) {
        return;
    }

    painter.save();
    QPen pen(stroke.color,
             qMax(1, stroke.width),
             Qt::SolidLine,
             Qt::RoundCap,
             Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    switch (stroke.tool) {
        case GifDrawingTool::Pencil: {
            if (stroke.points.size() == 1) {
                painter.drawPoint(stroke.points.first());
            } else {
                QPainterPath path;
                path.moveTo(stroke.points.first());
                for (int i = 1; i < stroke.points.size(); ++i) {
                    path.lineTo(stroke.points.at(i));
                }
                painter.drawPath(path);
            }
            break;
        }
        case GifDrawingTool::Line:
            if (stroke.points.size() >= 2) {
                painter.drawLine(stroke.points.at(0), stroke.points.at(1));
            }
            break;
        case GifDrawingTool::Arrow:
            if (stroke.points.size() >= 2) {
                drawArrow(painter,
                          stroke.points.at(0),
                          stroke.points.at(1),
                          stroke.width);
            }
            break;
        case GifDrawingTool::Rectangle:
            if (stroke.points.size() >= 2) {
                painter.drawRect(
                  QRectF(stroke.points.at(0), stroke.points.at(1)).normalized());
            }
            break;
        case GifDrawingTool::Ellipse:
            if (stroke.points.size() >= 2) {
                painter.drawEllipse(
                  QRectF(stroke.points.at(0), stroke.points.at(1)).normalized());
            }
            break;
        case GifDrawingTool::Text: {
            QFont font = painter.font();
            font.setPixelSize(qMax(14, stroke.width * 5));
            font.setBold(true);
            painter.setFont(font);
            const QPointF pos = stroke.points.first();
            painter.setPen(QPen(Qt::black, qMax(1, stroke.width / 2 + 1)));
            painter.drawText(pos + QPointF(1, 1), stroke.text);
            painter.setPen(QPen(stroke.color, 1));
            painter.drawText(pos, stroke.text);
            break;
        }
        case GifDrawingTool::Number: {
            const qreal radius =
              qMax<qreal>(12, stroke.width * 3 + 6);
            const QPointF center = stroke.points.first();
            const QRectF circle(center.x() - radius,
                                center.y() - radius,
                                radius * 2,
                                radius * 2);
            painter.setBrush(stroke.color);
            painter.setPen(QPen(Qt::white, 2));
            painter.drawEllipse(circle);

            QFont font = painter.font();
            font.setPixelSize(qMax(12, qRound(radius)));
            font.setBold(true);
            painter.setFont(font);
            painter.setPen(Qt::white);
            painter.drawText(circle,
                             Qt::AlignCenter,
                             QString::number(stroke.number));
            break;
        }
        case GifDrawingTool::NoTool:
        default:
            break;
    }

    painter.restore();
}

void GifRecordingOverlay::drawArrow(QPainter& painter,
                                    const QPointF& from,
                                    const QPointF& to,
                                    int width) const
{
    painter.drawLine(from, to);

    const double angle = std::atan2(-(to.y() - from.y()),
                                    to.x() - from.x());
    const double arrowSize = qMax(8.0, width * 4.0);
    const QPointF p1(
      to.x() - std::sin(angle + kPi / 3.0) * arrowSize,
      to.y() - std::cos(angle + kPi / 3.0) * arrowSize);
    const QPointF p2(
      to.x() - std::sin(angle + 2.0 * kPi / 3.0) * arrowSize,
      to.y() - std::cos(angle + 2.0 * kPi / 3.0) * arrowSize);

    painter.drawLine(to, p1);
    painter.drawLine(to, p2);
}

GifRecordingBorder::GifRecordingBorder(const QRect& region, QWidget* parent)
  : QWidget(parent)
{
    constexpr int margin = 4;
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setGeometry(region.adjusted(-margin, -margin, margin, margin));
}

void GifRecordingBorder::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

#if defined(FLAMESHOT_HAVE_X11)
    if (QGuiApplication::platformName() == QLatin1String("xcb")) {
        // The border is purely visual: always give it an empty input shape so
        // clicks near the recorded region pass through to the desktop.
        winId();
        setX11InputShape(winId(), false);
    }
#endif
}

void GifRecordingBorder::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(QColor(255, 59, 48), 2, Qt::DashLine);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    // The window is a few pixels larger than the recorded region, so this
    // border stays outside the region and never appears in the GIF.
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}
