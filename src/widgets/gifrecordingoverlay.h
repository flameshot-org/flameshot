// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#pragma once

#include "gifdrawingtool.h"

#include <QColor>
#include <QImage>
#include <QPointF>
#include <QRect>
#include <QVector>
#include <QWidget>

class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QShowEvent;
class QHideEvent;

class GifRecordingBorder;

/**
 * Full-screen transparent canvas shown on top of the recorded region.
 *
 * The canvas is click-through by default, so the user can keep working with
 * other applications while a GIF is being recorded. Selecting a drawing tool
 * temporarily makes it receive mouse input; after the stroke is finished it
 * becomes click-through again. Drawings stay on the canvas, remain visible and
 * are therefore captured by GifRecorder.
 */
class GifRecordingOverlay : public QWidget
{
    Q_OBJECT

public:
    struct Stroke
    {
        GifDrawingTool tool = GifDrawingTool::NoTool;
        QVector<QPointF> points;
        QColor color;
        int width = 1;
        QString text;
        int number = 0;
    };

    explicit GifRecordingOverlay(const QRect& region,
                                 QWidget* parent = nullptr);
    ~GifRecordingOverlay() override;

    void setDrawingColor(const QColor& color);
    void setDrawingWidth(int width);
    QVector<Stroke> strokes() const { return m_strokes; }
    void setStrokes(const QVector<Stroke>& strokes);

    // Renders the current strokes to a transparent image the size of the
    // recorded region. Used by GifRecorder to composite drawings into frames
    // independently of the compositor.
    QImage renderToImage() const;
    // The overlay may be hidden for the duration of a screen grab only when
    // no tool is active and no stroke is in progress.
    bool canBeHiddenForCapture() const;
    void setCaptureHidden(bool hidden);

    QColor drawingColor() const { return m_color; }
    int drawingWidth() const { return m_width; }

public slots:
    void setTool(GifDrawingTool tool);
    void finishTool();
    void undo();
    void clear();
    void addText(const QPoint& pos, const QString& text);
    void requestRecreate();

signals:
    // Emitted when the current stroke/tool is finished and the overlay is
    // click-through again.
    void toolFinished();
    // Emitted when the overlay window must be recreated to force the
    // compositor to repaint it (undo, clear, text placement).
    void recreateRequested();
    // Emitted when the user clicks with the text tool. The text itself is
    // requested by the owner, so a modal dialog can be shown while the
    // recorder is paused.
    void textInputRequested(const QPoint& pos);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void setInputTransparent(bool transparent);
    void applyInputShape();
    void commitStroke();
    void drawStroke(QPainter& painter, const Stroke& stroke) const;
    void drawArrow(QPainter& painter,
                   const QPointF& from,
                   const QPointF& to,
                   int width) const;

    QRect m_region;
    QVector<Stroke> m_strokes;
    Stroke m_current;
    bool m_drawing = false;
    GifDrawingTool m_tool = GifDrawingTool::NoTool;
    QColor m_color = Qt::red;
    int m_width = 3;
    int m_numberCounter = 0;
    bool m_inputTransparent = true;
    bool m_applyingInputFlag = false;
    bool m_captureHidden = false;
    GifRecordingBorder* m_border = nullptr;
};

/**
 * Thin frame drawn just outside the recorded region. It is a separate
 * click-through window, so it is visible to the user but never appears in the
 * captured frames.
 */
class GifRecordingBorder : public QWidget
{
public:
    explicit GifRecordingBorder(const QRect& region,
                                QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
};
