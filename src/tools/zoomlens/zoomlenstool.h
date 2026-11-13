// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "tools/abstracttwopointtool.h"

class ZoomLensTool : public AbstractTwoPointTool
{
    Q_OBJECT
public:
    explicit ZoomLensTool(QObject* parent = nullptr);

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;
    QRect boundingRect() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;
    void paintMousePreview(QPainter& painter,
                           const CaptureContext& context) override;
    void drawObjectSelection(QPainter& painter) override;
    void move(const QPoint& pos) override;
    const QPoint* pos() override;
    void prepareMove(const QPoint& clickPos) override;

protected:
    CaptureTool::Type type() const override;

public slots:
    void drawStart(const CaptureContext& context) override;
    void drawMove(const QPoint& p) override;
    void drawEnd(const QPoint& p) override;
    void pressed(CaptureContext& context) override;

private:
    enum class DragTarget {
        SOURCE_MOVE,
        SOURCE_RESIZE,
        SOURCE_EDGE_RESIZE,
        POPUP_MOVE,
        POPUP_RESIZE,
        POPUP_EDGE_RESIZE
    };
    enum class Corner { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT };
    enum class Edge { TOP, BOTTOM, LEFT, RIGHT };

    QRect sourceRect() const;
    QRect calcPopupRect(const QRect& source, const QSize& imageBounds) const;
    QPoint connectorStart(const QRect& source, const QRect& popup,
                          int margin) const;
    QPoint connectorEnd(const QRect& source, const QRect& popup,
                        int margin) const;
    Corner nearestCorner(const QPoint& pos, const QRect& rect) const;
    bool detectEdge(const QPoint& clickPos, const QRect& rect, Edge& edge,
                    QPoint& anchor, int& fixedVal) const;
    void drawHandles(QPainter& painter, const QRect& rect) const;

    QRect m_popupRect;
    QSize m_imageBounds;
    bool m_popupInitialized;

    DragTarget m_dragTarget;
    Corner m_resizeCorner;
    Edge m_resizeEdge;
    QPoint m_moveAnchor;
    QPoint m_fixedCorner;
    int m_fixedEdgeVal;
    qreal m_aspectRatio;
};
